#include <iostream>
#include <map>
#include <set>
#include "quad.hh"
#include "quadssa.hh"
#include "quadssa_diag.hh"

using namespace std;

static void recordActualPhiBlocks(quad::QuadFuncDecl* func, SsaDiagState& diag) {
    diag.actualPhiBlocksByVar.clear();

    if (!func || !func->quadblocklist) return;

    for (auto block : *func->quadblocklist) {
        if (!block || !block->quadlist || !block->entry_label) continue;

        for (auto stmt : *block->quadlist) {
            if (stmt->kind != quad::QuadKind::PHI) continue;

            auto* phi = static_cast<quad::QuadPhi*>(stmt);
            if (!phi->temp_exp || !phi->temp_exp->temp) continue;

            int encodedTempNum = phi->temp_exp->temp->num;
            int origVar = VersionedTemp::origTempNum(encodedTempNum);
            diag.actualPhiBlocksByVar[origVar].insert(block->entry_label->num);
        }
    }
}

static void printBlockSet(const set<int>& blocks) {
    cout << "[";
    bool first = true;
    for (auto block : blocks) {
        if (!first) cout << ",";
        cout << "L" << block;
        first = false;
    }
    cout << "]";
}

static void printVersionBlocks(const map<int, set<int>>& versionBlocks) {
    cout << "[";
    bool firstVersion = true;
    for (auto& versionPair : versionBlocks) {
        if (versionPair.first == 0) continue;
        if (!firstVersion) cout << ",";
        cout << "v" << versionPair.first << "@{";
        bool firstBlock = true;
        for (auto block : versionPair.second) {
            if (!firstBlock) cout << ",";
            cout << "L" << block;
            firstBlock = false;
        }
        cout << "}";
        firstVersion = false;
    }
    cout << "]";
}

static bool hasNonRootVersion(const map<int, set<int>>& versionBlocks) {
    for (auto& versionPair : versionBlocks) {
        if (versionPair.first != 0) {
            return true;
        }
    }
    return false;
}

void printSsaDiagSummary(quad::QuadFuncDecl* func, SsaDiagState& diag) {
    recordActualPhiBlocks(func, diag);

    cout << "SSA_DIAG_BEGIN func=" << diag.funcName << endl;

    set<int> allVars;
    for (auto& it : diag.candidatePhiBlocksByVar) allVars.insert(it.first);
    for (auto& it : diag.actualPhiBlocksByVar) allVars.insert(it.first);
    for (auto& it : diag.createdVersionBlocksByVar) allVars.insert(it.first);
    for (auto& it : diag.eliminatedVersionBlocksByVar) allVars.insert(it.first);

    bool hasAnyVersionInfo = false;
    for (auto var : allVars) {
        auto candidateIt = diag.candidatePhiBlocksByVar.find(var);
        auto actualIt = diag.actualPhiBlocksByVar.find(var);
        auto createdIt = diag.createdVersionBlocksByVar.find(var);
        auto eliminatedIt = diag.eliminatedVersionBlocksByVar.find(var);
        bool hasCandidatePhi = (candidateIt != diag.candidatePhiBlocksByVar.end() && !candidateIt->second.empty());
        bool hasActualPhi = (actualIt != diag.actualPhiBlocksByVar.end() && !actualIt->second.empty());
        bool hasCreated = (createdIt != diag.createdVersionBlocksByVar.end() && hasNonRootVersion(createdIt->second));
        bool hasEliminated = (eliminatedIt != diag.eliminatedVersionBlocksByVar.end() && hasNonRootVersion(eliminatedIt->second));
        if (hasCandidatePhi || hasActualPhi || hasCreated || hasEliminated) {
            hasAnyVersionInfo = true;
            break;
        }
    }

    if (hasAnyVersionInfo) {
        for (auto var : allVars) {
            auto candidateIt = diag.candidatePhiBlocksByVar.find(var);
            auto actualIt = diag.actualPhiBlocksByVar.find(var);
            auto createdIt = diag.createdVersionBlocksByVar.find(var);
            auto eliminatedIt = diag.eliminatedVersionBlocksByVar.find(var);
            bool hasCandidatePhi = (candidateIt != diag.candidatePhiBlocksByVar.end() && !candidateIt->second.empty());
            bool hasActualPhi = (actualIt != diag.actualPhiBlocksByVar.end() && !actualIt->second.empty());
            bool hasCreated = (createdIt != diag.createdVersionBlocksByVar.end() && hasNonRootVersion(createdIt->second));
            bool hasEliminated = (eliminatedIt != diag.eliminatedVersionBlocksByVar.end() && hasNonRootVersion(eliminatedIt->second));

            if (!hasCandidatePhi && !hasActualPhi && !hasCreated && !hasEliminated) {
                continue;
            }

            cout << "SSA_TEMP_DIAG temp=t" << var;

            if (hasCandidatePhi) {
                cout << " candidate_phi=";
                printBlockSet(candidateIt->second);
            }

            if (hasActualPhi) {
                cout << " actual_phi=";
                printBlockSet(actualIt->second);
            }

            if (hasCreated) {
                cout << " created=";
                printVersionBlocks(createdIt->second);
            }

            if (hasEliminated) {
                cout << " eliminated=";
                printVersionBlocks(eliminatedIt->second);
            }
            cout << endl;
        }
    }

    cout << "SSA_DIAG_END func=" << diag.funcName << endl;
}
