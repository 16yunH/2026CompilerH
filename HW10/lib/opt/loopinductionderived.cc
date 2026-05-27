#include "loopinductionderived.hh"
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

map<QuadStm*, size_t> buildStatementOrder(QuadFuncDecl* func) {
    map<QuadStm*, size_t> order;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return order;
    }

    size_t next = 0;
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

struct AffineState {
    bool valid = false;
    int basicTempNum = -1;
    int coeff = 0;
    int constant = 0;
};

AffineState fromTerm(QuadTerm* term, const map<int, AffineState>& affineTemps) {
    AffineState out;
    if (term == nullptr) {
        return out;
    }
    if (term->kind == QuadTermKind::CONST) {
        out.valid = true;
        out.basicTempNum = -1;
        out.coeff = 0;
        out.constant = term->get_const();
        return out;
    }

    int tempNum = termTempNum(term);
    auto it = affineTemps.find(tempNum);
    if (it != affineTemps.end()) {
        return it->second;
    }
    return out;
}

AffineState addAffine(const AffineState& lhs, const AffineState& rhs, int rhsSign) {
    AffineState out;
    if (!lhs.valid || !rhs.valid) {
        return out;
    }

    if (lhs.basicTempNum != -1 && rhs.basicTempNum != -1 &&
        lhs.basicTempNum != rhs.basicTempNum) {
        return out;
    }

    out.valid = true;
    out.basicTempNum = lhs.basicTempNum != -1 ? lhs.basicTempNum : rhs.basicTempNum;
    out.coeff = lhs.coeff + rhsSign * rhs.coeff;
    out.constant = lhs.constant + rhsSign * rhs.constant;
    return out;
}

AffineState mulAffine(const AffineState& lhs, const AffineState& rhs) {
    AffineState out;
    if (!lhs.valid || !rhs.valid) {
        return out;
    }

    bool lhsConst = lhs.basicTempNum == -1 && lhs.coeff == 0;
    bool rhsConst = rhs.basicTempNum == -1 && rhs.coeff == 0;
    if (lhsConst == rhsConst) {
        return out;
    }

    const AffineState& affine = lhsConst ? rhs : lhs;
    int scale = lhsConst ? lhs.constant : rhs.constant;
    out.valid = true;
    out.basicTempNum = affine.basicTempNum;
    out.coeff = affine.coeff * scale;
    out.constant = affine.constant * scale;
    return out;
}

AffineState computeAffine(QuadStm* stm, const map<int, AffineState>& affineTemps) {
    AffineState out;
    if (stm == nullptr) {
        return out;
    }

    if (stm->kind == QuadKind::MOVE) {
        QuadMove* move = static_cast<QuadMove*>(stm);
        return fromTerm(move->src, affineTemps);
    }

    if (stm->kind != QuadKind::MOVE_BINOP) {
        return out;
    }

    QuadMoveBinop* binop = static_cast<QuadMoveBinop*>(stm);
    AffineState left = fromTerm(binop->left, affineTemps);
    AffineState right = fromTerm(binop->right, affineTemps);

    if (binop->binop == "+") {
        return addAffine(left, right, +1);
    }
    if (binop->binop == "-") {
        return addAffine(left, right, -1);
    }
    if (binop->binop == "*") {
        return mulAffine(left, right);
    }

    return out;
}

int firstSourceTemp(QuadStm* stm) {
    if (stm == nullptr) {
        return -1;
    }

    if (stm->kind == QuadKind::MOVE) {
        return termTempNum(static_cast<QuadMove*>(stm)->src);
    }
    if (stm->kind == QuadKind::MOVE_BINOP) {
        QuadMoveBinop* binop = static_cast<QuadMoveBinop*>(stm);
        int left = termTempNum(binop->left);
        if (left != -1) {
            return left;
        }
        return termTempNum(binop->right);
    }
    return -1;
}

bool isOnlyUsedByAffineDefs(int tempNum,
                            const DefUseChain& du,
                            const set<QuadStm*>& affineDefStms) {
    VarDefInfo* defInfo = du.getDef(tempNum);
    if (defInfo == nullptr) {
        return false;
    }

    if (defInfo->useSet.empty()) {
        return false;
    }

    for (const auto& use : defInfo->useSet) {
        if (!affineDefStms.count(use.second)) {
            return false;
        }
    }
    return true;
}

} // namespace

map<int, vector<DerivedInductionVar>> discoverDerivedInductionVars(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, 
        const DefUseChain& du, const ControlFlowInfo& cfi) {
    map<int, vector<DerivedInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }

    map<int, vector<BasicInductionVar>> basicByHeader =
        discoverBasicInductionVars(func, loopHeaderMap, du, cfi);
    map<int, QuadBlock*> labelToBlock = buildLabelToBlock(func);
    map<QuadStm*, size_t> statementOrder = buildStatementOrder(func);

    vector<LoopHeader*> loops(loopHeaderMap->funcLoopHeaders[func].begin(),
                              loopHeaderMap->funcLoopHeaders[func].end());
    sort(loops.begin(), loops.end(), [](LoopHeader* lhs, LoopHeader* rhs) {
        return lhs != nullptr && (rhs == nullptr || lhs->headerLabel < rhs->headerLabel);
    });

    for (LoopHeader* loopHeader : loops) {
        if (loopHeader == nullptr || !basicByHeader.count(loopHeader->headerLabel)) {
            continue;
        }

        map<int, AffineState> affineTemps;
        set<QuadStm*> basicUpdateStms;
        set<int> basicInitTemps;
        set<int> stepTemps;
        for (const BasicInductionVar& biv : basicByHeader[loopHeader->headerLabel]) {
            AffineState state;
            state.valid = true;
            state.basicTempNum = biv.phiTempNum;
            state.coeff = 1;
            state.constant = 0;
            affineTemps[biv.phiTempNum] = state;
            affineTemps[biv.backedgeTempNum] = state;
            basicUpdateStms.insert(biv.updateStm);
            basicInitTemps.insert(biv.initTempNum);
            if (biv.stepTempNum != -1) {
                stepTemps.insert(biv.stepTempNum < 0 ? -biv.stepTempNum : biv.stepTempNum);
            }
        }

        map<int, AffineState> affineDefs;
        map<int, QuadStm*> affineDefStmByTemp;
        set<QuadStm*> affineDefStms;

        for (QuadBlock* block : *func->quadblocklist) {
            if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr ||
                !loopHeader->bodyBlocks.count(block->entry_label->num)) {
                continue;
            }

            for (QuadStm* stm : *block->quadlist) {
                if (stm == nullptr || basicUpdateStms.count(stm)) {
                    continue;
                }

                int defTemp = dstTempNum(stm);
                if (defTemp == -1 || basicInitTemps.count(defTemp) || stepTemps.count(defTemp)) {
                    continue;
                }

                AffineState affine = computeAffine(stm, affineTemps);
                if (!affine.valid || affine.basicTempNum == -1 || affine.coeff == 0) {
                    continue;
                }

                affineTemps[defTemp] = affine;
                affineDefs[defTemp] = affine;
                affineDefStmByTemp[defTemp] = stm;
                affineDefStms.insert(stm);
            }
        }

        vector<DerivedInductionVar> derived;
        for (const auto& entry : affineDefs) {
            int tempNum = entry.first;
            const AffineState& affine = entry.second;
            QuadStm* defStm = affineDefStmByTemp[tempNum];

            if (isOnlyUsedByAffineDefs(tempNum, du, affineDefStms)) {
                continue;
            }

            AffineIVExpr expr;
            expr.basicTempNum = affine.basicTempNum;
            expr.basicCoeff = affine.coeff;
            expr.constant = affine.constant;

            auto orderIt = statementOrder.find(defStm);
            size_t order = orderIt != statementOrder.end()
                ? orderIt->second
                : static_cast<size_t>(-1);

            derived.push_back(DerivedInductionVar(loopHeader->headerLabel,
                                                  tempNum,
                                                  firstSourceTemp(defStm),
                                                  order,
                                                  expr,
                                                  defStm));
        }

        sort(derived.begin(), derived.end(), [](const DerivedInductionVar& lhs,
                                                const DerivedInductionVar& rhs) {
            if (lhs.sourceOrder != rhs.sourceOrder) {
                return lhs.sourceOrder < rhs.sourceOrder;
            }
            return lhs.tempNum < rhs.tempNum;
        });

        if (!derived.empty()) {
            result[loopHeader->headerLabel] = derived;
        }
    }

    return result;
}
