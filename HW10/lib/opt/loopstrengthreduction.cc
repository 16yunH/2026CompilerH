#include "loopstrengthreduction.hh"
#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <algorithm>
#include <cstdlib>

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

QuadTemp* makeQuadTemp(int tempNum, QuadType type = QuadType::INT) {
    return new QuadTemp(new Temp(tempNum), type);
}

QuadTerm* makeTempTerm(int tempNum, QuadType type = QuadType::INT) {
    return new QuadTerm(makeQuadTemp(tempNum, type));
}

QuadTerm* makeConstTerm(int value) {
    return new QuadTerm(value);
}

set<Temp*>* makeDefSet(int tempNum) {
    auto out = new set<Temp*>();
    out->insert(new Temp(tempNum));
    return out;
}

set<Temp*>* makeUseSet(initializer_list<int> temps) {
    auto out = new set<Temp*>();
    for (int tempNum : temps) {
        if (tempNum != -1) {
            out->insert(new Temp(tempNum));
        }
    }
    return out;
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

void collectTermTemp(QuadTerm* term, set<int>& temps) {
    int tempNum = termTempNum(term);
    if (tempNum != -1) {
        temps.insert(tempNum);
    }
}

set<int> collectAllTemps(QuadFuncDecl* func) {
    set<int> temps;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return temps;
    }

    if (func->params != nullptr) {
        for (Temp* param : *func->params) {
            if (param != nullptr) {
                temps.insert(param->num);
            }
        }
    }

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            int def = dstTempNum(stm);
            if (def != -1) {
                temps.insert(def);
            }

            switch (stm->kind) {
            case QuadKind::MOVE:
                collectTermTemp(static_cast<QuadMove*>(stm)->src, temps);
                break;
            case QuadKind::LOAD:
                collectTermTemp(static_cast<QuadLoad*>(stm)->src, temps);
                break;
            case QuadKind::STORE:
                collectTermTemp(static_cast<QuadStore*>(stm)->src, temps);
                collectTermTemp(static_cast<QuadStore*>(stm)->dst, temps);
                break;
            case QuadKind::MOVE_BINOP:
                collectTermTemp(static_cast<QuadMoveBinop*>(stm)->left, temps);
                collectTermTemp(static_cast<QuadMoveBinop*>(stm)->right, temps);
                break;
            case QuadKind::CALL: {
                QuadCall* call = static_cast<QuadCall*>(stm);
                collectTermTemp(call->obj_term, temps);
                if (call->args != nullptr) {
                    for (QuadTerm* arg : *call->args) {
                        collectTermTemp(arg, temps);
                    }
                }
                break;
            }
            case QuadKind::MOVE_CALL: {
                QuadCall* call = static_cast<QuadMoveCall*>(stm)->call;
                if (call != nullptr) {
                    collectTermTemp(call->obj_term, temps);
                    if (call->args != nullptr) {
                        for (QuadTerm* arg : *call->args) {
                            collectTermTemp(arg, temps);
                        }
                    }
                }
                break;
            }
            case QuadKind::EXTCALL:
                if (static_cast<QuadExtCall*>(stm)->args != nullptr) {
                    for (QuadTerm* arg : *static_cast<QuadExtCall*>(stm)->args) {
                        collectTermTemp(arg, temps);
                    }
                }
                break;
            case QuadKind::MOVE_EXTCALL:
                if (static_cast<QuadMoveExtCall*>(stm)->extcall != nullptr &&
                    static_cast<QuadMoveExtCall*>(stm)->extcall->args != nullptr) {
                    for (QuadTerm* arg : *static_cast<QuadMoveExtCall*>(stm)->extcall->args) {
                        collectTermTemp(arg, temps);
                    }
                }
                break;
            case QuadKind::CJUMP:
                collectTermTemp(static_cast<QuadCJump*>(stm)->left, temps);
                collectTermTemp(static_cast<QuadCJump*>(stm)->right, temps);
                break;
            case QuadKind::PHI:
                if (static_cast<QuadPhi*>(stm)->args != nullptr) {
                    for (const auto& arg : *static_cast<QuadPhi*>(stm)->args) {
                        if (arg.first != nullptr) {
                            temps.insert(arg.first->num);
                        }
                    }
                }
                break;
            case QuadKind::RETURN:
                collectTermTemp(static_cast<QuadReturn*>(stm)->exp, temps);
                break;
            case QuadKind::PTR_CALC:
                collectTermTemp(static_cast<QuadPtrCalc*>(stm)->dst, temps);
                collectTermTemp(static_cast<QuadPtrCalc*>(stm)->ptr, temps);
                collectTermTemp(static_cast<QuadPtrCalc*>(stm)->offset, temps);
                break;
            default:
                break;
            }
        }
    }
    return temps;
}

int allocateTemp(set<int>& usedTemps, int start) {
    int tempNum = start;
    while (usedTemps.count(tempNum)) {
        ++tempNum;
    }
    usedTemps.insert(tempNum);
    return tempNum;
}

const BasicInductionVar* findBasicIV(const vector<BasicInductionVar>& basics, int phiTempNum) {
    for (const BasicInductionVar& biv : basics) {
        if (biv.phiTempNum == phiTempNum) {
            return &biv;
        }
    }
    return nullptr;
}

int phiArgLabel(QuadStm* phiStm, int tempNum, bool wantInside, const set<int>& loopBody) {
    if (phiStm == nullptr || phiStm->kind != QuadKind::PHI) {
        return -1;
    }
    QuadPhi* phi = static_cast<QuadPhi*>(phiStm);
    if (phi->args == nullptr) {
        return -1;
    }
    for (const auto& arg : *phi->args) {
        if (arg.first == nullptr || arg.second == nullptr || arg.first->num != tempNum) {
            continue;
        }
        bool inside = loopBody.count(arg.second->num) != 0;
        if (inside == wantInside) {
            return arg.second->num;
        }
    }
    return -1;
}

LoopHeader* findLoopHeader(LoopHeaderMap* loopHeaderMap, QuadFuncDecl* func, int headerLabel) {
    if (loopHeaderMap == nullptr || !loopHeaderMap->funcLoopHeaders.count(func)) {
        return nullptr;
    }
    for (LoopHeader* loopHeader : loopHeaderMap->funcLoopHeaders[func]) {
        if (loopHeader != nullptr && loopHeader->headerLabel == headerLabel) {
            return loopHeader;
        }
    }
    return nullptr;
}

bool isTerminator(QuadStm* stm) {
    return stm != nullptr &&
           (stm->kind == QuadKind::JUMP ||
            stm->kind == QuadKind::CJUMP ||
            stm->kind == QuadKind::RETURN);
}

QuadMoveBinop* makeConstBinop(int dst, int lhs, const string& op, int rhsConst) {
    return new QuadMoveBinop(makeQuadTemp(dst),
                             makeTempTerm(lhs),
                             op,
                             makeConstTerm(rhsConst),
                             makeDefSet(dst),
                             makeUseSet({lhs}));
}

QuadMoveBinop* makeTempBinop(int dst, int lhs, const string& op, int rhs) {
    return new QuadMoveBinop(makeQuadTemp(dst),
                             makeTempTerm(lhs),
                             op,
                             makeTempTerm(rhs),
                             makeDefSet(dst),
                             makeUseSet({lhs, rhs}));
}

QuadMove* makeMove(int dst, int src) {
    return new QuadMove(makeQuadTemp(dst),
                        makeTempTerm(src),
                        makeDefSet(dst),
                        makeUseSet({src}));
}

vector<QuadStm*> buildInitStatements(const StrengthReductionPlan::ReplacementIV& repl) {
    vector<QuadStm*> stms;

    int sourceTemp = repl.initExpr.initTempNum;
    if (repl.initExpr.sourceAfterBasicUpdate) {
        int adjusted = repl.initTemps.newInitAdjustedSourceTemp;
        if (repl.initExpr.basicStepTempNum != -1) {
            int stepTemp = repl.initExpr.basicStepTempNum < 0
                ? -repl.initExpr.basicStepTempNum
                : repl.initExpr.basicStepTempNum;
            string op = repl.initExpr.basicStepTempNum < 0 ? "-" : "+";
            stms.push_back(makeTempBinop(adjusted, sourceTemp, op, stepTemp));
        } else if (repl.initExpr.basicStepValue < 0) {
            stms.push_back(makeConstBinop(adjusted, sourceTemp, "-", -repl.initExpr.basicStepValue));
        } else if (repl.initExpr.basicStepValue > 0) {
            stms.push_back(makeConstBinop(adjusted, sourceTemp, "+", repl.initExpr.basicStepValue));
        } else {
            stms.push_back(makeMove(adjusted, sourceTemp));
        }
        sourceTemp = adjusted;
    }

    int scaledTemp = sourceTemp;
    int coeff = repl.initExpr.basicCoeff;
    if (coeff != 1) {
        scaledTemp = repl.initTemps.newInitIntermediateTemp;
        stms.push_back(makeConstBinop(scaledTemp, sourceTemp, "*", coeff));
    }

    if (repl.initExpr.constant > 0) {
        stms.push_back(makeConstBinop(repl.initTemps.newInitTemp, scaledTemp, "+",
                                      repl.initExpr.constant));
    } else if (repl.initExpr.constant < 0) {
        stms.push_back(makeConstBinop(repl.initTemps.newInitTemp, scaledTemp, "-",
                                      -repl.initExpr.constant));
    } else if (scaledTemp != repl.initTemps.newInitTemp) {
        stms.push_back(makeMove(repl.initTemps.newInitTemp, scaledTemp));
    }

    return stms;
}

vector<QuadStm*> buildStepPrepStatements(const StrengthReductionPlan::ReplacementIV& repl) {
    vector<QuadStm*> stms;
    if (repl.stepExpr.stepIncrementTempNum == -1 ||
        repl.stepExpr.newStepTemp == -1 ||
        repl.stepExpr.stepSourceTempNum == -1) {
        return stms;
    }

    int scale = repl.stepExpr.stepTempScaleFactor;
    if (scale == 1) {
        return stms;
    }
    stms.push_back(makeConstBinop(repl.stepExpr.newStepTemp,
                                  repl.stepExpr.stepSourceTempNum,
                                  "*",
                                  scale));
    return stms;
}

QuadStm* buildUpdateStatement(const StrengthReductionPlan::ReplacementIV& repl) {
    int dst = repl.map.newBackedgeTemp;
    int src = repl.map.newPhiTemp;

    if (repl.stepExpr.stepIncrementTempNum != -1) {
        string op = repl.stepExpr.stepIncrementNegative ? "-" : "+";
        return makeTempBinop(dst, src, op, repl.stepExpr.stepIncrementTempNum);
    }

    int step = repl.stepExpr.stepIncrementValue;
    if (step < 0) {
        return makeConstBinop(dst, src, "-", -step);
    }
    if (step > 0) {
        return makeConstBinop(dst, src, "+", step);
    }
    return makeMove(dst, src);
}

QuadPhi* buildPhiStatement(const StrengthReductionPlan::ReplacementIV& repl) {
    auto args = new vector<pair<Temp*, Label*>>();
    args->push_back({new Temp(repl.map.newBackedgeTemp), new Label(repl.placement.backedgeLabel)});
    args->push_back({new Temp(repl.initTemps.newInitTemp), new Label(repl.placement.initLabel)});
    return new QuadPhi(makeQuadTemp(repl.map.newPhiTemp),
                       args,
                       makeDefSet(repl.map.newPhiTemp),
                       makeUseSet({repl.map.newBackedgeTemp, repl.initTemps.newInitTemp}));
}

void insertBeforeTerminator(QuadBlock* block, const vector<QuadStm*>& stms) {
    if (block == nullptr || block->quadlist == nullptr || stms.empty()) {
        return;
    }
    auto pos = block->quadlist->end();
    if (!block->quadlist->empty() && isTerminator(block->quadlist->back())) {
        pos = block->quadlist->end() - 1;
    }
    block->quadlist->insert(pos, stms.begin(), stms.end());
}

void insertPhi(QuadBlock* header, QuadStm* phi) {
    if (header == nullptr || header->quadlist == nullptr || phi == nullptr) {
        return;
    }
    auto pos = header->quadlist->begin();
    while (pos != header->quadlist->end() &&
           ((*pos) == nullptr || (*pos)->kind == QuadKind::LABEL || (*pos)->kind == QuadKind::PHI)) {
        ++pos;
    }
    header->quadlist->insert(pos, phi);
}

void replaceTermTemp(QuadTerm*& term, const map<int, int>& replacements) {
    int tempNum = termTempNum(term);
    if (tempNum == -1 || !replacements.count(tempNum)) {
        return;
    }
    term = makeTempTerm(replacements.at(tempNum));
}

void replaceUsesInCall(QuadCall* call, const map<int, int>& replacements) {
    if (call == nullptr) {
        return;
    }
    replaceTermTemp(call->obj_term, replacements);
    if (call->args != nullptr) {
        for (QuadTerm*& arg : *call->args) {
            replaceTermTemp(arg, replacements);
        }
    }
}

void replaceUsesInStm(QuadStm* stm, const map<int, int>& replacements) {
    if (stm == nullptr || replacements.empty()) {
        return;
    }

    switch (stm->kind) {
    case QuadKind::MOVE:
        replaceTermTemp(static_cast<QuadMove*>(stm)->src, replacements);
        break;
    case QuadKind::LOAD:
        replaceTermTemp(static_cast<QuadLoad*>(stm)->src, replacements);
        break;
    case QuadKind::STORE:
        replaceTermTemp(static_cast<QuadStore*>(stm)->src, replacements);
        replaceTermTemp(static_cast<QuadStore*>(stm)->dst, replacements);
        break;
    case QuadKind::MOVE_BINOP:
        replaceTermTemp(static_cast<QuadMoveBinop*>(stm)->left, replacements);
        replaceTermTemp(static_cast<QuadMoveBinop*>(stm)->right, replacements);
        break;
    case QuadKind::CALL:
        replaceUsesInCall(static_cast<QuadCall*>(stm), replacements);
        break;
    case QuadKind::MOVE_CALL:
        replaceUsesInCall(static_cast<QuadMoveCall*>(stm)->call, replacements);
        break;
    case QuadKind::EXTCALL:
        if (static_cast<QuadExtCall*>(stm)->args != nullptr) {
            for (QuadTerm*& arg : *static_cast<QuadExtCall*>(stm)->args) {
                replaceTermTemp(arg, replacements);
            }
        }
        break;
    case QuadKind::MOVE_EXTCALL:
        if (static_cast<QuadMoveExtCall*>(stm)->extcall != nullptr &&
            static_cast<QuadMoveExtCall*>(stm)->extcall->args != nullptr) {
            for (QuadTerm*& arg : *static_cast<QuadMoveExtCall*>(stm)->extcall->args) {
                replaceTermTemp(arg, replacements);
            }
        }
        break;
    case QuadKind::CJUMP:
        replaceTermTemp(static_cast<QuadCJump*>(stm)->left, replacements);
        replaceTermTemp(static_cast<QuadCJump*>(stm)->right, replacements);
        break;
    case QuadKind::PHI:
        if (static_cast<QuadPhi*>(stm)->args != nullptr) {
            for (auto& arg : *static_cast<QuadPhi*>(stm)->args) {
                if (arg.first != nullptr && replacements.count(arg.first->num)) {
                    arg.first = new Temp(replacements.at(arg.first->num));
                }
            }
        }
        break;
    case QuadKind::RETURN:
        replaceTermTemp(static_cast<QuadReturn*>(stm)->exp, replacements);
        break;
    case QuadKind::PTR_CALC:
        replaceTermTemp(static_cast<QuadPtrCalc*>(stm)->ptr, replacements);
        replaceTermTemp(static_cast<QuadPtrCalc*>(stm)->offset, replacements);
        break;
    default:
        break;
    }
}

void rewriteHeaderCondition(QuadBlock* header,
                            const StrengthReductionPlan::ReplacementIV& repl) {
    if (header == nullptr || header->quadlist == nullptr || repl.initExpr.basicCoeff <= 0) {
        return;
    }

    int adjustment = 0;
    if (repl.initExpr.sourceAfterBasicUpdate && repl.initExpr.basicStepTempNum == -1) {
        adjustment = repl.initExpr.basicCoeff * repl.initExpr.basicStepValue;
    }

    for (QuadStm* stm : *header->quadlist) {
        if (stm == nullptr || stm->kind != QuadKind::CJUMP) {
            continue;
        }
        QuadCJump* cjump = static_cast<QuadCJump*>(stm);
        int leftTemp = termTempNum(cjump->left);
        int rightTemp = termTempNum(cjump->right);

        if (leftTemp == repl.map.basicTempNum && cjump->right != nullptr &&
            cjump->right->kind == QuadTermKind::CONST) {
            int bound = cjump->right->get_const();
            int newBound = repl.initExpr.basicCoeff * bound + repl.initExpr.constant + adjustment;
            cjump->left = makeTempTerm(repl.map.newPhiTemp);
            cjump->right = makeConstTerm(newBound);
        } else if (rightTemp == repl.map.basicTempNum && cjump->left != nullptr &&
                   cjump->left->kind == QuadTermKind::CONST) {
            int bound = cjump->left->get_const();
            int newBound = repl.initExpr.basicCoeff * bound + repl.initExpr.constant + adjustment;
            cjump->right = makeTempTerm(repl.map.newPhiTemp);
            cjump->left = makeConstTerm(newBound);
        }
    }
}

} // namespace

StrengthReductionPlan generateStrengthReductionPlan(
    QuadFuncDecl* func,
    const map<int, vector<DerivedInductionVar>>& derivedIVsByHeader,
    const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
    LoopHeaderMap* loopHeaderMap
) {
    StrengthReductionPlan plan;
    
    if (func == nullptr || loopHeaderMap == nullptr || derivedIVsByHeader.empty()) {
        return plan;
    }

    set<int> usedTemps = collectAllTemps(func);

    for (const auto& entry : derivedIVsByHeader) {
        int headerLabel = entry.first;
        auto basicIt = basicIVsByHeader.find(headerLabel);
        if (basicIt == basicIVsByHeader.end()) {
            continue;
        }

        LoopHeader* loopHeader = findLoopHeader(loopHeaderMap, func, headerLabel);
        if (loopHeader == nullptr) {
            continue;
        }

        for (const DerivedInductionVar& div : entry.second) {
            const BasicInductionVar* basic = findBasicIV(basicIt->second, div.expr.basicTempNum);
            if (basic == nullptr) {
                continue;
            }

            StrengthReductionPlan::ReplacementIV repl;
            repl.sourceOrder = div.sourceOrder;
            repl.map.headerLabel = headerLabel;
            repl.map.oldTempNum = div.tempNum;
            repl.map.basicTempNum = basic->phiTempNum;

            int nextTemp = div.tempNum + 1;
            repl.map.newPhiTemp = allocateTemp(usedTemps, nextTemp);
            repl.initTemps.newInitTemp = allocateTemp(usedTemps, repl.map.newPhiTemp + 1);
            repl.map.newBackedgeTemp = allocateTemp(usedTemps, repl.initTemps.newInitTemp + 1);

            bool sourceAfterBasicUpdate = basic->updateOrder >= 0 &&
                div.sourceOrder != static_cast<size_t>(-1) &&
                div.sourceOrder > static_cast<size_t>(basic->updateOrder);

            repl.initExpr.initTempNum = basic->initTempNum;
            repl.initExpr.basicCoeff = div.expr.basicCoeff;
            repl.initExpr.constant = div.expr.constant;
            repl.initExpr.sourceAfterBasicUpdate = sourceAfterBasicUpdate;
            repl.initExpr.basicStepTempNum = basic->stepTempNum;
            repl.initExpr.basicStepValue = basic->step;

            if (sourceAfterBasicUpdate) {
                repl.initTemps.newInitAdjustedSourceTemp =
                    allocateTemp(usedTemps, repl.map.newBackedgeTemp + 1);
            }
            if (div.expr.basicCoeff != 1) {
                int start = sourceAfterBasicUpdate
                    ? repl.initTemps.newInitAdjustedSourceTemp + 1
                    : repl.map.newBackedgeTemp + 1;
                repl.initTemps.newInitIntermediateTemp = allocateTemp(usedTemps, start);
            }

            if (basic->stepTempNum != -1) {
                int absStepTemp = basic->stepTempNum < 0 ? -basic->stepTempNum : basic->stepTempNum;
                int absCoeff = div.expr.basicCoeff < 0 ? -div.expr.basicCoeff : div.expr.basicCoeff;
                repl.stepExpr.stepSourceTempNum = absStepTemp;
                repl.stepExpr.stepTempScaleFactor = absCoeff == 0 ? 1 : absCoeff;
                repl.stepExpr.stepIncrementNegative =
                    (basic->stepTempNum < 0) != (div.expr.basicCoeff < 0);

                if (absCoeff != 1) {
                    int start = repl.initTemps.newInitIntermediateTemp != -1
                        ? repl.initTemps.newInitIntermediateTemp + 1
                        : repl.map.newBackedgeTemp + 1;
                    repl.stepExpr.newStepTemp = allocateTemp(usedTemps, start);
                    repl.stepExpr.stepIncrementTempNum = repl.stepExpr.newStepTemp;
                } else {
                    repl.stepExpr.stepIncrementTempNum = absStepTemp;
                }
            } else {
                repl.stepExpr.stepIncrementValue = div.expr.basicCoeff * basic->step;
            }

            repl.placement.initLabel = phiArgLabel(basic->phiStm, basic->initTempNum,
                                                   false, loopHeader->bodyBlocks);
            repl.placement.backedgeLabel = phiArgLabel(basic->phiStm, basic->backedgeTempNum,
                                                       true, loopHeader->bodyBlocks);
            if (repl.placement.initLabel == -1 || repl.placement.backedgeLabel == -1) {
                continue;
            }

            plan.tempReplacement[div.tempNum] = repl.map.newPhiTemp;
            plan.stmtsToRemove.insert(div.defStm);
            plan.phiStmtsToAdd[repl.map.newPhiTemp] =
                {repl.initTemps.newInitTemp, repl.map.newBackedgeTemp};
            plan.updateStmtsToAdd[repl.map.newBackedgeTemp] =
                {div.expr.basicCoeff, div.expr.constant};
            plan.replacements.push_back(repl);
        }
    }

    return plan;
}

QuadFuncDecl* applyStrengthReduction(
    QuadFuncDecl* func,
    const StrengthReductionPlan& plan
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }

    if (plan.tempReplacement.empty()) {
        return func;
    }

    map<int, QuadBlock*> labelToBlock = buildLabelToBlock(func);

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            replaceUsesInStm(stm, plan.tempReplacement);
        }
    }

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        auto filtered = new vector<QuadStm*>();
        for (QuadStm* stm : *block->quadlist) {
            if (!plan.stmtsToRemove.count(stm)) {
                filtered->push_back(stm);
            }
        }
        block->quadlist = filtered;
    }

    for (const auto& repl : plan.replacements) {
        if (labelToBlock.count(repl.placement.initLabel)) {
            vector<QuadStm*> initStms = buildInitStatements(repl);
            vector<QuadStm*> stepPrepStms = buildStepPrepStatements(repl);
            initStms.insert(initStms.end(), stepPrepStms.begin(), stepPrepStms.end());
            insertBeforeTerminator(labelToBlock[repl.placement.initLabel], initStms);
        }
        if (labelToBlock.count(repl.map.headerLabel)) {
            insertPhi(labelToBlock[repl.map.headerLabel], buildPhiStatement(repl));
            rewriteHeaderCondition(labelToBlock[repl.map.headerLabel], repl);
        }
        if (labelToBlock.count(repl.placement.backedgeLabel)) {
            insertBeforeTerminator(labelToBlock[repl.placement.backedgeLabel],
                                   {buildUpdateStatement(repl)});
        }
    }

    return func;
}
