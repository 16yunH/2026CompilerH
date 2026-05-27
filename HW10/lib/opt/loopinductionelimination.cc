#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <queue>

using namespace std;
using namespace quad;

namespace {

int termTempNum(QuadTerm* term) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) {
        return -1;
    }
    QuadTemp* temp = term->get_temp();
    return (temp != nullptr && temp->temp != nullptr) ? temp->temp->num : -1;
}

int dstTempNum(QuadStm* stm) {
    if (stm == nullptr) {
        return -1;
    }

    switch (stm->kind) {
    case QuadKind::MOVE:
        return static_cast<QuadMove*>(stm)->dst != nullptr &&
               static_cast<QuadMove*>(stm)->dst->temp != nullptr
            ? static_cast<QuadMove*>(stm)->dst->temp->num
            : -1;
    case QuadKind::MOVE_BINOP:
        return static_cast<QuadMoveBinop*>(stm)->dst != nullptr &&
               static_cast<QuadMoveBinop*>(stm)->dst->temp != nullptr
            ? static_cast<QuadMoveBinop*>(stm)->dst->temp->num
            : -1;
    case QuadKind::PHI:
        return static_cast<QuadPhi*>(stm)->temp_exp != nullptr &&
               static_cast<QuadPhi*>(stm)->temp_exp->temp != nullptr
            ? static_cast<QuadPhi*>(stm)->temp_exp->temp->num
            : -1;
    case QuadKind::PTR_CALC:
        return termTempNum(static_cast<QuadPtrCalc*>(stm)->dst);
    default:
        return -1;
    }
}

bool isPureDef(QuadStm* stm) {
    if (stm == nullptr || dstTempNum(stm) == -1) {
        return false;
    }
    return stm->kind == QuadKind::MOVE ||
           stm->kind == QuadKind::MOVE_BINOP ||
           stm->kind == QuadKind::PHI ||
           stm->kind == QuadKind::PTR_CALC;
}

bool isRootStatement(QuadStm* stm) {
    if (stm == nullptr) {
        return false;
    }
    if (!isPureDef(stm)) {
        return true;
    }
    return false;
}

void enqueueUses(QuadStm* stm, const DefUseChain& du, queue<int>& worklist, set<int>& seenTemps) {
    for (int tempNum : du.getUsesBy(stm)) {
        if (!seenTemps.count(tempNum)) {
            seenTemps.insert(tempNum);
            worklist.push(tempNum);
        }
    }
}

} // namespace

QuadFuncDecl* eliminateUnusedInductionVars(QuadFuncDecl* func) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }

    DefUseChain du(func);
    set<QuadStm*> liveStatements;
    set<int> seenTemps;
    queue<int> worklist;

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            if (isRootStatement(stm)) {
                liveStatements.insert(stm);
                enqueueUses(stm, du, worklist, seenTemps);
            }
        }
    }

    while (!worklist.empty()) {
        int tempNum = worklist.front();
        worklist.pop();

        VarDefInfo* defInfo = du.getDef(tempNum);
        if (defInfo == nullptr || defInfo->defStm == nullptr ||
            liveStatements.count(defInfo->defStm)) {
            continue;
        }

        liveStatements.insert(defInfo->defStm);
        enqueueUses(defInfo->defStm, du, worklist, seenTemps);
    }

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }

        auto filtered = new vector<QuadStm*>();
        for (QuadStm* stm : *block->quadlist) {
            if (!isPureDef(stm) || liveStatements.count(stm)) {
                filtered->push_back(stm);
            }
        }
        block->quadlist = filtered;
    }

    return func;
}
