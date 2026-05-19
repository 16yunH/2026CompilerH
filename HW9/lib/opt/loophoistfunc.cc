#define DEBUG
#undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "quad.hh"
#include "flowinfo.hh"
#include "loopopt.hh"

using namespace std;
using namespace quad;

// Main entry point for loop optimization
// Complete the function!!

static int termTempNum(QuadTerm *term) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) {
        return -1;
    }
    QuadTemp *temp = term->get_temp();
    if (temp == nullptr || temp->temp == nullptr) {
        return -1;
    }
    return temp->temp->num;
}

static int defTempNum(QuadStm *stm) {
    if (stm == nullptr) {
        return -1;
    }

    switch (stm->kind) {
    case QuadKind::MOVE: {
        QuadMove *move = static_cast<QuadMove *>(stm);
        return move->dst != nullptr && move->dst->temp != nullptr ? move->dst->temp->num : -1;
    }
    case QuadKind::MOVE_BINOP: {
        QuadMoveBinop *move = static_cast<QuadMoveBinop *>(stm);
        return move->dst != nullptr && move->dst->temp != nullptr ? move->dst->temp->num : -1;
    }
    case QuadKind::PTR_CALC: {
        QuadPtrCalc *ptrCalc = static_cast<QuadPtrCalc *>(stm);
        return termTempNum(ptrCalc->dst);
    }
    default:
        break;
    }

    return -1;
}

static bool isHoistablePureStm(QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }

    return stm->kind == QuadKind::MOVE ||
           stm->kind == QuadKind::MOVE_BINOP ||
           stm->kind == QuadKind::PTR_CALC;
}

static vector<QuadTerm *> usedTerms(QuadStm *stm) {
    vector<QuadTerm *> terms;
    if (stm == nullptr) {
        return terms;
    }

    switch (stm->kind) {
    case QuadKind::MOVE: {
        QuadMove *move = static_cast<QuadMove *>(stm);
        terms.push_back(move->src);
        break;
    }
    case QuadKind::MOVE_BINOP: {
        QuadMoveBinop *move = static_cast<QuadMoveBinop *>(stm);
        terms.push_back(move->left);
        terms.push_back(move->right);
        break;
    }
    case QuadKind::PTR_CALC: {
        QuadPtrCalc *ptrCalc = static_cast<QuadPtrCalc *>(stm);
        terms.push_back(ptrCalc->ptr);
        terms.push_back(ptrCalc->offset);
        break;
    }
    default:
        break;
    }

    return terms;
}

static map<int, QuadBlock *> collectBlocks(QuadFuncDecl *func) {
    map<int, QuadBlock *> blocks;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return blocks;
    }

    for (QuadBlock *block : *func->quadblocklist) {
        if (block != nullptr && block->entry_label != nullptr) {
            blocks[block->entry_label->num] = block;
        }
    }
    return blocks;
}

static map<int, set<int>> buildPredecessors(QuadFuncDecl *func) {
    map<int, set<int>> predecessors;
    map<int, QuadBlock *> blocks = collectBlocks(func);
    for (const auto &entry : blocks) {
        predecessors[entry.first] = {};
    }

    for (const auto &entry : blocks) {
        int pred = entry.first;
        QuadBlock *block = entry.second;
        if (block == nullptr || block->exit_labels == nullptr) {
            continue;
        }

        for (Label *label : *block->exit_labels) {
            if (label != nullptr && blocks.count(label->num)) {
                predecessors[label->num].insert(pred);
            }
        }
    }

    return predecessors;
}

static map<int, int> collectDefBlocks(QuadFuncDecl *func) {
    map<int, int> defBlocks;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return defBlocks;
    }

    for (QuadBlock *block : *func->quadblocklist) {
        if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr) {
            continue;
        }

        int blockLabel = block->entry_label->num;
        for (QuadStm *stm : *block->quadlist) {
            if (stm == nullptr || stm->def == nullptr) {
                continue;
            }
            for (Temp *temp : *stm->def) {
                if (temp != nullptr) {
                    defBlocks[temp->num] = blockLabel;
                }
            }
        }
    }

    return defBlocks;
}

static bool termIsLoopInvariant(QuadTerm *term,
                                const set<int> &loopBody,
                                const map<int, int> &defBlocks,
                                const set<int> &invariantDefs) {
    if (term == nullptr) {
        return false;
    }

    if (term->kind == QuadTermKind::CONST || term->kind == QuadTermKind::NAME) {
        return true;
    }

    int tempNum = termTempNum(term);
    if (tempNum < 0) {
        return false;
    }

    auto defIt = defBlocks.find(tempNum);
    if (defIt == defBlocks.end()) {
        return true;
    }
    if (!loopBody.count(defIt->second)) {
        return true;
    }

    return invariantDefs.count(tempNum) != 0;
}

static bool stmIsLoopInvariant(QuadStm *stm,
                               const set<int> &loopBody,
                               const map<int, int> &defBlocks,
                               const set<int> &invariantDefs) {
    if (!isHoistablePureStm(stm) || defTempNum(stm) < 0) {
        return false;
    }

    for (QuadTerm *term : usedTerms(stm)) {
        if (!termIsLoopInvariant(term, loopBody, defBlocks, invariantDefs)) {
            return false;
        }
    }

    return true;
}

static bool isTerminator(QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == QuadKind::JUMP ||
           stm->kind == QuadKind::CJUMP ||
           stm->kind == QuadKind::RETURN;
}

static QuadBlock *findPreheader(QuadFuncDecl *func, LoopHeader *loopHeader) {
    if (func == nullptr || loopHeader == nullptr) {
        return nullptr;
    }

    map<int, QuadBlock *> blocks = collectBlocks(func);
    map<int, set<int>> predecessors = buildPredecessors(func);
    auto predIt = predecessors.find(loopHeader->headerLabel);
    if (predIt == predecessors.end()) {
        return nullptr;
    }

    for (int pred : predIt->second) {
        if (!loopHeader->bodyBlocks.count(pred) && blocks.count(pred)) {
            return blocks[pred];
        }
    }

    return nullptr;
}

static vector<QuadStm *> findInvariantStms(QuadFuncDecl *func, LoopHeader *loopHeader) {
    vector<QuadStm *> invariantStms;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeader == nullptr) {
        return invariantStms;
    }

    map<int, int> defBlocks = collectDefBlocks(func);
    set<int> invariantDefs;
    set<QuadStm *> selected;

    bool changed = true;
    while (changed) {
        changed = false;
        for (QuadBlock *block : *func->quadblocklist) {
            if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr ||
                !loopHeader->bodyBlocks.count(block->entry_label->num)) {
                continue;
            }

            for (QuadStm *stm : *block->quadlist) {
                if (stm == nullptr || selected.count(stm)) {
                    continue;
                }

                if (stmIsLoopInvariant(stm, loopHeader->bodyBlocks, defBlocks, invariantDefs)) {
                    selected.insert(stm);
                    invariantStms.push_back(stm);
                    invariantDefs.insert(defTempNum(stm));
                    changed = true;
                }
            }
        }
    }

    return invariantStms;
}

static void removeStmsFromLoop(QuadFuncDecl *func,
                               const set<int> &loopBody,
                               const set<QuadStm *> &toMove) {
    if (func == nullptr || func->quadblocklist == nullptr || toMove.empty()) {
        return;
    }

    for (QuadBlock *block : *func->quadblocklist) {
        if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr ||
            !loopBody.count(block->entry_label->num)) {
            continue;
        }

        vector<QuadStm *> *newQuadList = new vector<QuadStm *>();
        for (QuadStm *stm : *block->quadlist) {
            if (!toMove.count(stm)) {
                newQuadList->push_back(stm);
            }
        }
        block->quadlist = newQuadList;
    }
}

static void insertBeforeTerminator(QuadBlock *block, const vector<QuadStm *> &stms) {
    if (block == nullptr || block->quadlist == nullptr || stms.empty()) {
        return;
    }

    auto insertPos = block->quadlist->end();
    if (!block->quadlist->empty() && isTerminator(block->quadlist->back())) {
        insertPos = block->quadlist->end() - 1;
    }
    block->quadlist->insert(insertPos, stms.begin(), stms.end());
}

QuadFuncDecl* loopHoistFunc(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap) {
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return func;
    }

    auto mapIt = loopHeaderMap->funcLoopHeaders.find(func);
    if (mapIt == loopHeaderMap->funcLoopHeaders.end()) {
        return func;
    }

    vector<LoopHeader *> loops(mapIt->second.begin(), mapIt->second.end());
    sort(loops.begin(), loops.end(), [](LoopHeader *lhs, LoopHeader *rhs) {
        if (lhs == nullptr || rhs == nullptr) {
            return lhs != nullptr;
        }
        if (lhs->bodyBlocks.size() != rhs->bodyBlocks.size()) {
            return lhs->bodyBlocks.size() > rhs->bodyBlocks.size();
        }
        return lhs->headerLabel < rhs->headerLabel;
    });

    for (LoopHeader *loopHeader : loops) {
        if (loopHeader == nullptr) {
            continue;
        }

        QuadBlock *preheader = findPreheader(func, loopHeader);
        if (preheader == nullptr) {
            continue;
        }

        vector<QuadStm *> invariantStms = findInvariantStms(func, loopHeader);
        if (invariantStms.empty()) {
            continue;
        }

        set<QuadStm *> toMove(invariantStms.begin(), invariantStms.end());
        removeStmsFromLoop(func, loopHeader->bodyBlocks, toMove);
        insertBeforeTerminator(preheader, invariantStms);
    }

    return func;
}
