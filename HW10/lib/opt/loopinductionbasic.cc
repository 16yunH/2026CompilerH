#include "loopinductionbasic.hh"
#include "defusechain.hh"
#include "loopinductionopt.hh"
#include <algorithm>

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
    case QuadKind::LOAD:
        return static_cast<QuadLoad*>(stm)->dst != nullptr &&
               static_cast<QuadLoad*>(stm)->dst->temp != nullptr
            ? static_cast<QuadLoad*>(stm)->dst->temp->num
            : -1;
    case QuadKind::MOVE_BINOP:
        return static_cast<QuadMoveBinop*>(stm)->dst != nullptr &&
               static_cast<QuadMoveBinop*>(stm)->dst->temp != nullptr
            ? static_cast<QuadMoveBinop*>(stm)->dst->temp->num
            : -1;
    case QuadKind::MOVE_CALL:
        return static_cast<QuadMoveCall*>(stm)->dst != nullptr &&
               static_cast<QuadMoveCall*>(stm)->dst->temp != nullptr
            ? static_cast<QuadMoveCall*>(stm)->dst->temp->num
            : -1;
    case QuadKind::MOVE_EXTCALL:
        return static_cast<QuadMoveExtCall*>(stm)->dst != nullptr &&
               static_cast<QuadMoveExtCall*>(stm)->dst->temp != nullptr
            ? static_cast<QuadMoveExtCall*>(stm)->dst->temp->num
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

map<QuadStm*, int> buildStatementOrder(QuadFuncDecl* func) {
    map<QuadStm*, int> order;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return order;
    }

    int next = 0;
    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            order[stm] = next++;
        }
    }
    return order;
}

map<int, int> buildDefBlocks(QuadFuncDecl* func) {
    map<int, int> defBlocks;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return defBlocks;
    }

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr) {
            continue;
        }
        int blockLabel = block->entry_label->num;
        for (QuadStm* stm : *block->quadlist) {
            int temp = dstTempNum(stm);
            if (temp != -1) {
                defBlocks[temp] = blockLabel;
            }
        }
    }
    return defBlocks;
}

bool tempIsLoopInvariant(int tempNum, const set<int>& loopBody, const map<int, int>& defBlocks) {
    auto it = defBlocks.find(tempNum);
    if (it == defBlocks.end()) {
        return true;
    }
    return !loopBody.count(it->second);
}

struct StepInfo {
    bool valid = false;
    int constStep = 0;
    int stepTempNum = -1;
};

bool decodeStepTerm(QuadTerm* term,
                    int sign,
                    const set<int>& loopBody,
                    const map<int, int>& defBlocks,
                    StepInfo& out) {
    if (term == nullptr) {
        return false;
    }

    if (term->kind == QuadTermKind::CONST) {
        out.valid = true;
        out.constStep = sign * term->get_const();
        out.stepTempNum = -1;
        return true;
    }

    int tempNum = termTempNum(term);
    if (tempNum == -1 || !tempIsLoopInvariant(tempNum, loopBody, defBlocks)) {
        return false;
    }

    out.valid = true;
    out.constStep = 0;
    out.stepTempNum = sign >= 0 ? tempNum : -tempNum;
    return true;
}

bool parseBasicUpdate(QuadStm* stm,
                      int phiTempNum,
                      const set<int>& loopBody,
                      const map<int, int>& defBlocks,
                      StepInfo& out) {
    if (stm == nullptr || stm->kind != QuadKind::MOVE_BINOP) {
        return false;
    }

    QuadMoveBinop* binop = static_cast<QuadMoveBinop*>(stm);
    int leftTemp = termTempNum(binop->left);
    int rightTemp = termTempNum(binop->right);

    if (binop->binop == "+") {
        if (leftTemp == phiTempNum) {
            return decodeStepTerm(binop->right, +1, loopBody, defBlocks, out);
        }
        if (rightTemp == phiTempNum) {
            return decodeStepTerm(binop->left, +1, loopBody, defBlocks, out);
        }
    } else if (binop->binop == "-") {
        if (leftTemp == phiTempNum) {
            return decodeStepTerm(binop->right, -1, loopBody, defBlocks, out);
        }
    }

    return false;
}

bool phiArgLabels(QuadPhi* phi,
                  const set<int>& loopBody,
                  int& initTemp,
                  int& backedgeTemp) {
    if (phi == nullptr || phi->args == nullptr) {
        return false;
    }

    initTemp = -1;
    backedgeTemp = -1;
    for (const auto& arg : *phi->args) {
        if (arg.first == nullptr || arg.second == nullptr) {
            continue;
        }
        if (loopBody.count(arg.second->num)) {
            backedgeTemp = arg.first->num;
        } else {
            initTemp = arg.first->num;
        }
    }

    return initTemp != -1 && backedgeTemp != -1;
}

} // namespace

map<int, vector<BasicInductionVar>> discoverBasicInductionVars(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, 
        const DefUseChain& du, const ControlFlowInfo& cfi) {
    map<int, vector<BasicInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }

    map<int, QuadBlock*> labelToBlock = buildLabelToBlock(func);
    map<QuadStm*, int> statementOrder = buildStatementOrder(func);
    map<int, int> defBlocks = buildDefBlocks(func);

    vector<LoopHeader*> loops(loopHeaderMap->funcLoopHeaders[func].begin(),
                              loopHeaderMap->funcLoopHeaders[func].end());
    sort(loops.begin(), loops.end(), [](LoopHeader* lhs, LoopHeader* rhs) {
        return lhs != nullptr && (rhs == nullptr || lhs->headerLabel < rhs->headerLabel);
    });

    for (LoopHeader* loopHeader : loops) {
        if (loopHeader == nullptr || !labelToBlock.count(loopHeader->headerLabel)) {
            continue;
        }

        QuadBlock* headerBlock = labelToBlock[loopHeader->headerLabel];
        if (headerBlock == nullptr || headerBlock->quadlist == nullptr) {
            continue;
        }

        for (QuadStm* stm : *headerBlock->quadlist) {
            if (stm == nullptr || stm->kind != QuadKind::PHI) {
                continue;
            }

            QuadPhi* phi = static_cast<QuadPhi*>(stm);
            int phiTempNum = dstTempNum(stm);
            int initTempNum = -1;
            int backedgeTempNum = -1;
            if (phiTempNum == -1 ||
                !phiArgLabels(phi, loopHeader->bodyBlocks, initTempNum, backedgeTempNum)) {
                continue;
            }

            VarDefInfo* backedgeDef = du.getDef(backedgeTempNum);
            if (backedgeDef == nullptr || backedgeDef->defStm == nullptr ||
                backedgeDef->defBlock == nullptr || backedgeDef->defBlock->entry_label == nullptr ||
                !loopHeader->bodyBlocks.count(backedgeDef->defBlock->entry_label->num)) {
                continue;
            }

            StepInfo stepInfo;
            if (!parseBasicUpdate(backedgeDef->defStm, phiTempNum, loopHeader->bodyBlocks,
                                  defBlocks, stepInfo)) {
                continue;
            }

            BasicInductionVar biv(loopHeader->headerLabel,
                                  phiTempNum,
                                  initTempNum,
                                  backedgeTempNum,
                                  stepInfo.constStep,
                                  stm,
                                  backedgeDef->defStm);
            biv.stepTempNum = stepInfo.stepTempNum;
            auto orderIt = statementOrder.find(backedgeDef->defStm);
            if (orderIt != statementOrder.end()) {
                biv.updateOrder = orderIt->second;
            }
            if (stepInfo.stepTempNum != -1) {
                int absStepTemp = stepInfo.stepTempNum < 0
                    ? -stepInfo.stepTempNum
                    : stepInfo.stepTempNum;
                biv.addRelatedTemp(absStepTemp);
                biv.markUseful(absStepTemp);
            }

            result[loopHeader->headerLabel].push_back(biv);
        }
    }

    return result;
}

// Classify related temps as useless (only in backedges) or useful (in computations)
void classifyRelatedTemps(
    QuadFuncDecl* func,
    map<int, vector<BasicInductionVar>>& basicByHeader,
    const DefUseChain& du
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }

    for (auto& entry : basicByHeader) {
        for (BasicInductionVar& biv : entry.second) {
            int absStepTemp = biv.stepTempNum == -1
                ? -1
                : (biv.stepTempNum < 0 ? -biv.stepTempNum : biv.stepTempNum);

            for (int tempNum : biv.relatedTemps) {
                if (tempNum == biv.phiTempNum || tempNum == absStepTemp) {
                    biv.markUseful(tempNum);
                    continue;
                }

                bool useful = false;
                VarDefInfo* defInfo = du.getDef(tempNum);
                if (defInfo != nullptr) {
                    for (const auto& use : defInfo->useSet) {
                        QuadStm* useStm = use.second;
                        if (useStm == biv.phiStm) {
                            continue;
                        }
                        if (useStm == biv.updateStm && tempNum != absStepTemp) {
                            continue;
                        }
                        useful = true;
                        break;
                    }
                }

                if (useful) {
                    biv.markUseful(tempNum);
                } else {
                    biv.markUseless(tempNum);
                }
            }
        }
    }
    return ;
}
