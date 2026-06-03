#include "instrSelection.hh"

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace instr {

namespace {

struct PtrCalcInfo {
    tree::Temp *base = nullptr;
    bool offsetIsTemp = false;
    tree::Temp *offsetTemp = nullptr;
    int offsetConst = 0;
};

struct AddressMode {
    tree::Temp *base = nullptr;
    bool offsetIsTemp = false;
    tree::Temp *offsetTemp = nullptr;
    int offsetConst = 0;
};

static int tempNum(tree::Temp *temp) {
    return temp == nullptr ? -1 : temp->num;
}

static tree::Temp *newTemp(int &nextTempNum) {
    return new tree::Temp(nextTempNum++);
}

static tree::Temp *termTemp(const quad::QuadTerm *term) {
    if (term == nullptr || term->kind != quad::QuadTermKind::TEMP) {
        return nullptr;
    }
    auto *mutableTerm = const_cast<quad::QuadTerm*>(term);
    auto *quadTemp = mutableTerm->get_temp();
    return quadTemp == nullptr ? nullptr : quadTemp->temp;
}

static int termConst(const quad::QuadTerm *term) {
    auto *mutableTerm = const_cast<quad::QuadTerm*>(term);
    return mutableTerm->get_const();
}

static std::string termName(const quad::QuadTerm *term) {
    auto *mutableTerm = const_cast<quad::QuadTerm*>(term);
    return mutableTerm->get_name();
}

static tree::Temp *quadTempPtr(quad::QuadTemp *temp) {
    return temp == nullptr ? nullptr : temp->temp;
}

static void appendLoadConst(AssemInstrList &out, tree::Temp *dst, int value) {
    if (dst == nullptr) {
        return;
    }

    uint32_t bits = static_cast<uint32_t>(value);
    uint32_t low = bits & 0xffffu;
    uint32_t high = (bits >> 16u) & 0xffffu;
    out.append(AssemInstr::Oper("movw `d0, #" + std::to_string(low), {dst}, {}, AssemTargets()));
    if (high != 0) {
        out.append(AssemInstr::Oper("movt `d0, #" + std::to_string(high), {dst}, {dst}, AssemTargets()));
    }
}

static bool smallMoveImmediate(int value) {
    return value >= 0 && value <= 255;
}

static bool smallDataImmediate(int value) {
    return value >= 0 && value <= 4095;
}

static tree::Temp *materializeTerm(
    const quad::QuadTerm *term,
    AssemInstrList &out,
    int &nextTempNum
) {
    if (term == nullptr) {
        return nullptr;
    }

    if (term->kind == quad::QuadTermKind::TEMP) {
        return termTemp(term);
    }

    auto *tmp = newTemp(nextTempNum);
    if (term->kind == quad::QuadTermKind::CONST) {
        appendLoadConst(out, tmp, termConst(term));
    } else if (term->kind == quad::QuadTermKind::NAME) {
        out.append(AssemInstr::Oper("ldr `d0, =" + termName(term), {tmp}, {}, AssemTargets()));
    }
    return tmp;
}

static bool isExitCallStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    if (stm->kind == quad::QuadKind::EXTCALL) {
        auto *ext = dynamic_cast<const quad::QuadExtCall*>(stm);
        return ext != nullptr && ext->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
        return moveExt != nullptr && moveExt->extcall != nullptr && moveExt->extcall->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::CALL) {
        auto *call = dynamic_cast<const quad::QuadCall*>(stm);
        return call != nullptr && call->name == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall*>(stm);
        return moveCall != nullptr && moveCall->call != nullptr && moveCall->call->name == "exit";
    }
    return false;
}

static bool isControlOnlyStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return true;
    }
    return stm->kind == quad::QuadKind::LABEL ||
           stm->kind == quad::QuadKind::PHI ||
           stm->kind == quad::QuadKind::JUMP ||
           stm->kind == quad::QuadKind::CJUMP ||
           stm->kind == quad::QuadKind::RETURN;
}

static bool isSelectableStmt(const quad::QuadStm *stm) {
    return stm != nullptr && !isControlOnlyStmt(stm) && !isExitCallStmt(stm);
}

static bool isMallocMoveExtCall(const quad::QuadStm *stm) {
    auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
    return moveExt != nullptr && moveExt->extcall != nullptr && moveExt->extcall->extfun == "malloc";
}

static int definedTempNum(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->def == nullptr || stm->def->empty()) {
        return -1;
    }
    int out = -1;
    for (auto *temp : *stm->def) {
        int num = tempNum(temp);
        if (num >= 0 && (out < 0 || num < out)) {
            out = num;
        }
    }
    return out;
}

static bool usesTempNum(const quad::QuadStm *stm, int temp) {
    if (stm == nullptr || stm->use == nullptr || temp < 0) {
        return false;
    }
    for (auto *used : *stm->use) {
        if (tempNum(used) == temp) {
            return true;
        }
    }
    return false;
}

static bool isMemoryOrCallLike(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == quad::QuadKind::LOAD ||
           stm->kind == quad::QuadKind::STORE ||
           stm->kind == quad::QuadKind::CALL ||
           stm->kind == quad::QuadKind::MOVE_CALL ||
           stm->kind == quad::QuadKind::EXTCALL ||
           stm->kind == quad::QuadKind::MOVE_EXTCALL;
}

static bool isInternalCallLike(const quad::QuadStm *stm) {
    return stm != nullptr &&
           (stm->kind == quad::QuadKind::CALL || stm->kind == quad::QuadKind::MOVE_CALL);
}

static int callObjectTempNum(const quad::QuadStm *stm) {
    const quad::QuadCall *call = nullptr;
    if (stm != nullptr && stm->kind == quad::QuadKind::CALL) {
        call = dynamic_cast<const quad::QuadCall*>(stm);
    } else if (stm != nullptr && stm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall*>(stm);
        call = moveCall == nullptr ? nullptr : moveCall->call;
    }

    if (call == nullptr) {
        return -1;
    }
    return tempNum(termTemp(call->obj_term));
}

static bool isNonMallocCallLike(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    if (isInternalCallLike(stm) || stm->kind == quad::QuadKind::EXTCALL) {
        return true;
    }
    auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
    return moveExt != nullptr && moveExt->extcall != nullptr && moveExt->extcall->extfun != "malloc";
}

static bool buildPtrCalcInfo(const quad::QuadPtrCalc *ptrCalc, PtrCalcInfo &info) {
    if (ptrCalc == nullptr) {
        return false;
    }

    info.base = termTemp(ptrCalc->ptr);
    if (info.base == nullptr || ptrCalc->offset == nullptr) {
        return false;
    }

    if (ptrCalc->offset->kind == quad::QuadTermKind::TEMP) {
        info.offsetIsTemp = true;
        info.offsetTemp = termTemp(ptrCalc->offset);
    } else if (ptrCalc->offset->kind == quad::QuadTermKind::CONST) {
        info.offsetIsTemp = false;
        info.offsetConst = termConst(ptrCalc->offset);
    } else {
        return false;
    }
    return true;
}

static bool infoHasNonZeroOffset(const PtrCalcInfo &info) {
    return info.offsetIsTemp || info.offsetConst != 0;
}

static int addressBaseTempNum(
    const quad::QuadTerm *term,
    const std::unordered_map<int, PtrCalcInfo> &ptrCalcInfos
) {
    auto *temp = termTemp(term);
    int num = tempNum(temp);
    auto it = ptrCalcInfos.find(num);
    if (it != ptrCalcInfos.end()) {
        return tempNum(it->second.base);
    }
    return num;
}

static bool addressHasDynamicOffset(
    const quad::QuadTerm *term,
    const std::unordered_map<int, PtrCalcInfo> &ptrCalcInfos
) {
    auto *temp = termTemp(term);
    int num = tempNum(temp);
    auto it = ptrCalcInfos.find(num);
    return it != ptrCalcInfos.end() && it->second.offsetIsTemp;
}

static bool loadResultFeedsCallAcrossCall(
    const quad::QuadLoad *load,
    const std::vector<const quad::QuadStm*> &statements,
    size_t loadIndex
) {
    if (load == nullptr) {
        return false;
    }

    std::unordered_set<int> aliases;
    aliases.insert(tempNum(quadTempPtr(load->dst)));
    bool sawInterveningCall = false;

    for (size_t index = loadIndex + 1; index < statements.size(); ++index) {
        auto *stm = statements[index];
        int objectTemp = callObjectTempNum(stm);
        if (objectTemp >= 0 && aliases.find(objectTemp) != aliases.end()) {
            return sawInterveningCall;
        }

        auto *move = dynamic_cast<const quad::QuadMove*>(stm);
        if (move != nullptr && move->src != nullptr && move->src->kind == quad::QuadTermKind::TEMP) {
            int src = tempNum(termTemp(move->src));
            if (aliases.find(src) != aliases.end()) {
                aliases.insert(tempNum(quadTempPtr(move->dst)));
            }
        }

        if (isNonMallocCallLike(stm)) {
            sawInterveningCall = true;
        }
    }

    return false;
}

static AddressMode makeAddress(
    const quad::QuadTerm *term,
    const std::unordered_map<int, PtrCalcInfo> &foldedPtrCalcs,
    bool allowFold
) {
    AddressMode address;
    if (term == nullptr) {
        return address;
    }

    if (term->kind == quad::QuadTermKind::TEMP) {
        auto *temp = termTemp(term);
        int num = tempNum(temp);
        if (allowFold) {
            auto it = foldedPtrCalcs.find(num);
            if (it != foldedPtrCalcs.end()) {
                address.base = it->second.base;
                address.offsetIsTemp = it->second.offsetIsTemp;
                address.offsetTemp = it->second.offsetTemp;
                address.offsetConst = it->second.offsetConst;
                return address;
            }
        }
        address.base = temp;
    }

    return address;
}

static void appendLoad(AssemInstrList &out, tree::Temp *dst, const AddressMode &address) {
    if (dst == nullptr || address.base == nullptr) {
        return;
    }

    if (address.offsetIsTemp && address.offsetTemp != nullptr) {
        out.append(AssemInstr::Oper(
            "ldr `d0, [`s0, `s1]",
            {dst},
            {address.base, address.offsetTemp},
            AssemTargets()));
    } else if (address.offsetConst != 0) {
        out.append(AssemInstr::Oper(
            "ldr `d0, [`s0, #" + std::to_string(address.offsetConst) + "]",
            {dst},
            {address.base},
            AssemTargets()));
    } else {
        out.append(AssemInstr::Oper("ldr `d0, [`s0]", {dst}, {address.base}, AssemTargets()));
    }
}

static void appendStore(AssemInstrList &out, tree::Temp *src, const AddressMode &address) {
    if (src == nullptr || address.base == nullptr) {
        return;
    }

    if (address.offsetIsTemp && address.offsetTemp != nullptr) {
        out.append(AssemInstr::Oper(
            "str `s0, [`s1, `s2]",
            {},
            {src, address.base, address.offsetTemp},
            AssemTargets()));
    } else if (address.offsetConst != 0) {
        out.append(AssemInstr::Oper(
            "str `s0, [`s1, #" + std::to_string(address.offsetConst) + "]",
            {},
            {src, address.base},
            AssemTargets()));
    } else {
        out.append(AssemInstr::Oper("str `s0, [`s1]", {}, {src, address.base}, AssemTargets()));
    }
}

static void appendMoveConst(AssemInstrList &out, tree::Temp *dst, int value) {
    if (dst == nullptr) {
        return;
    }
    if (smallMoveImmediate(value)) {
        out.append(AssemInstr::Move("mov `d0, #" + std::to_string(value), {dst}, {}));
    } else {
        appendLoadConst(out, dst, value);
    }
}

static void appendMoveFromTerm(
    AssemInstrList &out,
    tree::Temp *dst,
    const quad::QuadTerm *src,
    int &nextTempNum
) {
    if (dst == nullptr || src == nullptr) {
        return;
    }

    if (src->kind == quad::QuadTermKind::CONST) {
        appendMoveConst(out, dst, termConst(src));
    } else if (src->kind == quad::QuadTermKind::NAME) {
        out.append(AssemInstr::Oper("ldr `d0, =" + termName(src), {dst}, {}, AssemTargets()));
    } else {
        auto *srcTemp = materializeTerm(src, out, nextTempNum);
        out.append(AssemInstr::Move("mov `d0, `s0", {dst}, {srcTemp}));
    }
}

static void appendPtrCalc(
    AssemInstrList &out,
    const quad::QuadPtrCalc *ptrCalc,
    int &nextTempNum
) {
    if (ptrCalc == nullptr) {
        return;
    }

    auto *dst = termTemp(ptrCalc->dst);
    auto *base = materializeTerm(ptrCalc->ptr, out, nextTempNum);
    if (dst == nullptr || base == nullptr || ptrCalc->offset == nullptr) {
        return;
    }

    if (ptrCalc->offset->kind == quad::QuadTermKind::CONST) {
        int offset = termConst(ptrCalc->offset);
        if (offset == 0) {
            out.append(AssemInstr::Move("mov `d0, `s0", {dst}, {base}));
        } else {
            out.append(AssemInstr::Oper(
                "add `d0, `s0, #" + std::to_string(offset),
                {dst},
                {base},
                AssemTargets()));
        }
    } else {
        auto *offset = materializeTerm(ptrCalc->offset, out, nextTempNum);
        out.append(AssemInstr::Oper("add `d0, `s0, `s1", {dst}, {base, offset}, AssemTargets()));
    }
}

static void appendBinop(
    AssemInstrList &out,
    const quad::QuadMoveBinop *binop,
    int &nextTempNum
) {
    if (binop == nullptr || binop->dst == nullptr) {
        return;
    }

    auto *dst = quadTempPtr(binop->dst);
    const auto *left = binop->left;
    const auto *right = binop->right;
    const std::string &op = binop->binop;
    if (dst == nullptr || left == nullptr || right == nullptr) {
        return;
    }

    if (op == "+") {
        if (left->kind == quad::QuadTermKind::TEMP && right->kind == quad::QuadTermKind::CONST &&
            smallDataImmediate(termConst(right))) {
            auto *src = termTemp(left);
            out.append(AssemInstr::Oper(
                "add `d0, `s0, #" + std::to_string(termConst(right)),
                {dst},
                {src},
                AssemTargets()));
            return;
        }
        if (right->kind == quad::QuadTermKind::TEMP && left->kind == quad::QuadTermKind::CONST &&
            smallDataImmediate(termConst(left))) {
            auto *src = termTemp(right);
            out.append(AssemInstr::Oper(
                "add `d0, `s0, #" + std::to_string(termConst(left)),
                {dst},
                {src},
                AssemTargets()));
            return;
        }
        auto *lhs = materializeTerm(left, out, nextTempNum);
        auto *rhs = materializeTerm(right, out, nextTempNum);
        out.append(AssemInstr::Oper("add `d0, `s0, `s1", {dst}, {lhs, rhs}, AssemTargets()));
        return;
    }

    if (op == "-") {
        if (left->kind == quad::QuadTermKind::TEMP && right->kind == quad::QuadTermKind::CONST &&
            smallDataImmediate(termConst(right))) {
            auto *src = termTemp(left);
            out.append(AssemInstr::Oper(
                "sub `d0, `s0, #" + std::to_string(termConst(right)),
                {dst},
                {src},
                AssemTargets()));
            return;
        }
        auto *lhs = materializeTerm(left, out, nextTempNum);
        auto *rhs = materializeTerm(right, out, nextTempNum);
        out.append(AssemInstr::Oper("sub `d0, `s0, `s1", {dst}, {lhs, rhs}, AssemTargets()));
        return;
    }

    if (op == "*") {
        auto *lhs = materializeTerm(left, out, nextTempNum);
        auto *rhs = materializeTerm(right, out, nextTempNum);
        out.append(AssemInstr::Oper("mul `d0, `s0, `s1", {dst}, {lhs, rhs}, AssemTargets()));
        return;
    }

    if (op == "/") {
        auto *lhs = materializeTerm(left, out, nextTempNum);
        auto *rhs = materializeTerm(right, out, nextTempNum);
        out.append(AssemInstr::Oper("sdiv `d0, `s0, `s1", {dst}, {lhs, rhs}, AssemTargets()));
    }
}

static void appendArgumentMoves(
    AssemInstrList &out,
    const std::vector<quad::QuadTerm*> *args,
    int &nextTempNum
) {
    if (args == nullptr) {
        return;
    }

    for (size_t index = args->size(); index > 4; --index) {
        auto *arg = materializeTerm((*args)[index - 1], out, nextTempNum);
        out.append(AssemInstr::Oper("str `s0, [sp, #-4]!", {}, {arg}, AssemTargets()));
    }

    std::vector<tree::Temp*> regArgs;
    size_t regCount = std::min<size_t>(args->size(), 4);
    regArgs.reserve(regCount);
    for (size_t index = 0; index < regCount; ++index) {
        regArgs.push_back(materializeTerm((*args)[index], out, nextTempNum));
    }
    for (size_t index = 0; index < regArgs.size(); ++index) {
        out.append(AssemInstr::Move("mov r" + std::to_string(index) + ", `s0", {}, {regArgs[index]}));
    }
}

static void appendStackArgCleanup(
    AssemInstrList &out,
    const std::vector<quad::QuadTerm*> *args
) {
    if (args != nullptr && args->size() > 4) {
        size_t bytes = (args->size() - 4) * 4;
        out.append(AssemInstr::Oper("add sp, sp, #" + std::to_string(bytes), {}, {}, AssemTargets()));
    }
}

static void appendExtCall(
    AssemInstrList &out,
    const quad::QuadExtCall *extcall,
    tree::Temp *dst,
    int &nextTempNum
) {
    if (extcall == nullptr) {
        return;
    }

    appendArgumentMoves(out, extcall->args, nextTempNum);
    out.append(AssemInstr::ExtCall("bl " + extcall->extfun, {}, {}));
    appendStackArgCleanup(out, extcall->args);

    if (dst != nullptr) {
        out.append(AssemInstr::Move("mov `d0, r0", {dst}, {}));
    }
}

static void appendCall(
    AssemInstrList &out,
    const quad::QuadCall *call,
    tree::Temp *dst,
    int &nextTempNum
) {
    if (call == nullptr) {
        return;
    }

    appendArgumentMoves(out, call->args, nextTempNum);
    auto *callee = materializeTerm(call->obj_term, out, nextTempNum);
    if (callee != nullptr) {
        out.append(AssemInstr::Call("blx `s0", {}, {callee}));
    } else {
        out.append(AssemInstr::Call("bl " + call->name, {}, {}));
    }
    appendStackArgCleanup(out, call->args);

    if (dst != nullptr) {
        out.append(AssemInstr::Move("mov `d0, r0", {dst}, {}));
    }
}

static std::vector<const quad::QuadStm*> orderSelectableStatements(
    const std::vector<advDFGNode*> &nodes,
    const std::unordered_set<int> &foldedPtrCalcTemps,
    const std::unordered_map<int, PtrCalcInfo> &foldedPtrCalcs
) {
    std::vector<const quad::QuadStm*> ordered;
    std::vector<advDFGNode*> candidates;
    std::unordered_set<const advDFGNode*> candidateSet;
    std::unordered_map<const advDFGNode*, size_t> originalIndex;

    for (size_t index = 0; index < nodes.size(); ++index) {
        auto *node = nodes[index];
        if (node == nullptr || !isSelectableStmt(node->quadStatement)) {
            continue;
        }
        if (node->quadStatement->kind == quad::QuadKind::PTR_CALC) {
            auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(node->quadStatement);
            auto *dst = ptrCalc == nullptr ? nullptr : termTemp(ptrCalc->dst);
            if (dst != nullptr && foldedPtrCalcTemps.find(dst->num) != foldedPtrCalcTemps.end()) {
                continue;
            }
        }
        candidates.push_back(node);
        candidateSet.insert(node);
        originalIndex[node] = index;
    }

    std::unordered_map<const advDFGNode*, int> indegree;
    std::unordered_map<int, const advDFGNode*> tempDefs;
    for (auto *node : candidates) {
        if (node != nullptr && node->tempDefined >= 0) {
            tempDefs[node->tempDefined] = node;
        }
    }

    std::unordered_map<const advDFGNode*, std::unordered_set<const advDFGNode*>> extraPreds;
    auto addFoldedAddressUses = [&](const quad::QuadTerm *addressTerm, std::set<int> &uses) {
        auto *addressTemp = termTemp(addressTerm);
        int addressNum = tempNum(addressTemp);
        auto infoIt = foldedPtrCalcs.find(addressNum);
        if (infoIt == foldedPtrCalcs.end()) {
            return;
        }

        uses.erase(addressNum);
        int baseNum = tempNum(infoIt->second.base);
        if (baseNum >= 0) {
            uses.insert(baseNum);
        }
        if (infoIt->second.offsetIsTemp) {
            int offsetNum = tempNum(infoIt->second.offsetTemp);
            if (offsetNum >= 0) {
                uses.insert(offsetNum);
            }
        }
    };

    for (auto *node : candidates) {
        if (node == nullptr) {
            continue;
        }
        std::set<int> effectiveUses = node->tempsUsed;
        if (node->quadStatement != nullptr && node->quadStatement->kind == quad::QuadKind::LOAD) {
            auto *load = dynamic_cast<const quad::QuadLoad*>(node->quadStatement);
            if (load != nullptr) {
                addFoldedAddressUses(load->src, effectiveUses);
            }
        } else if (node->quadStatement != nullptr && node->quadStatement->kind == quad::QuadKind::STORE) {
            auto *store = dynamic_cast<const quad::QuadStore*>(node->quadStatement);
            if (store != nullptr) {
                addFoldedAddressUses(store->dst, effectiveUses);
            }
        }

        for (int usedTemp : effectiveUses) {
            auto defIt = tempDefs.find(usedTemp);
            if (defIt != tempDefs.end() && defIt->second != nullptr && defIt->second != node) {
                extraPreds[node].insert(defIt->second);
            }
        }
    }

    for (auto *node : candidates) {
        std::unordered_set<const advDFGNode*> preds;
        for (auto *pred : node->predecessors) {
            if (candidateSet.find(pred) != candidateSet.end()) {
                preds.insert(pred);
            }
        }
        auto extraIt = extraPreds.find(node);
        if (extraIt != extraPreds.end()) {
            preds.insert(extraIt->second.begin(), extraIt->second.end());
        }
        indegree[node] = static_cast<int>(preds.size());
    }

    std::vector<advDFGNode*> ready;
    for (auto *node : candidates) {
        if (indegree[node] == 0) {
            ready.push_back(node);
        }
    }
    std::stable_sort(
        ready.begin(),
        ready.end(),
        [&](advDFGNode *left, advDFGNode *right) {
            return originalIndex[left] < originalIndex[right];
        });
    auto firstMalloc = std::find_if(
        ready.begin(),
        ready.end(),
        [](advDFGNode *node) {
            return isMallocMoveExtCall(node == nullptr ? nullptr : node->quadStatement);
        });
    if (firstMalloc != ready.end() && firstMalloc != ready.begin()) {
        std::rotate(ready.begin(), firstMalloc, firstMalloc + 1);
    }

    std::unordered_set<const advDFGNode*> emitted;
    size_t cursor = 0;
    while (cursor < ready.size()) {
        auto *node = ready[cursor++];
        if (node == nullptr || emitted.find(node) != emitted.end()) {
            continue;
        }
        emitted.insert(node);
        ordered.push_back(node->quadStatement);

        for (auto *candidate : candidates) {
            if (emitted.find(candidate) != emitted.end()) {
                continue;
            }
            bool isPred = candidate->predecessors.find(node) != candidate->predecessors.end();
            auto extraIt = extraPreds.find(candidate);
            if (extraIt != extraPreds.end() && extraIt->second.find(node) != extraIt->second.end()) {
                isPred = true;
            }
            if (!isPred) {
                continue;
            }
            --indegree[candidate];
            if (indegree[candidate] == 0) {
                ready.push_back(candidate);
            }
        }
    }

    for (auto *node : candidates) {
        if (emitted.find(node) == emitted.end()) {
            ordered.push_back(node->quadStatement);
        }
    }
    return ordered;
}

static bool isDirectStoreToTemp(const quad::QuadStm *stm, int temp) {
    auto *store = dynamic_cast<const quad::QuadStore*>(stm);
    return store != nullptr && tempNum(termTemp(store->dst)) == temp;
}

static bool storeSourceReadyBeforeRange(
    const quad::QuadStore *store,
    const std::vector<const quad::QuadStm*> &statements,
    size_t begin,
    size_t end
) {
    if (store == nullptr || store->src == nullptr || store->src->kind != quad::QuadTermKind::TEMP) {
        return true;
    }

    int src = tempNum(termTemp(store->src));
    for (size_t index = begin; index < end; ++index) {
        if (definedTempNum(statements[index]) == src) {
            return false;
        }
    }
    return true;
}

static void prioritizeDirectMallocStores(std::vector<const quad::QuadStm*> &statements) {
    for (size_t index = 0; index < statements.size(); ++index) {
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(statements[index]);
        if (moveExt == nullptr || moveExt->extcall == nullptr || moveExt->extcall->extfun != "malloc") {
            continue;
        }

        int mallocTemp = tempNum(quadTempPtr(moveExt->dst));
        if (mallocTemp < 0) {
            continue;
        }

        for (size_t storeIndex = index + 1; storeIndex < statements.size(); ++storeIndex) {
            if (isMemoryOrCallLike(statements[storeIndex]) && !isDirectStoreToTemp(statements[storeIndex], mallocTemp)) {
                break;
            }
            if (!isDirectStoreToTemp(statements[storeIndex], mallocTemp)) {
                continue;
            }

            bool skippedUsefulWork = false;
            bool movable = true;
            for (size_t mid = index + 1; mid < storeIndex; ++mid) {
                if (isMemoryOrCallLike(statements[mid]) || usesTempNum(statements[mid], mallocTemp)) {
                    movable = false;
                    break;
                }
                if (statements[mid] != nullptr && statements[mid]->use != nullptr && !statements[mid]->use->empty()) {
                    skippedUsefulWork = true;
                }
            }

            auto *store = dynamic_cast<const quad::QuadStore*>(statements[storeIndex]);
            if (!movable || !skippedUsefulWork ||
                !storeSourceReadyBeforeRange(store, statements, index + 1, storeIndex)) {
                break;
            }

            auto *directStore = statements[storeIndex];
            statements.erase(statements.begin() + storeIndex);
            statements.insert(statements.begin() + index + 1, directStore);
            break;
        }
    }
}

static bool moveTempToTemp(const quad::QuadStm *stm, int &dst, int &src) {
    auto *move = dynamic_cast<const quad::QuadMove*>(stm);
    if (move == nullptr || move->src == nullptr || move->src->kind != quad::QuadTermKind::TEMP) {
        return false;
    }
    dst = tempNum(quadTempPtr(move->dst));
    src = tempNum(termTemp(move->src));
    return dst >= 0 && src >= 0;
}

static bool tempUsedByStatement(const quad::QuadStm *stm, int temp) {
    return usesTempNum(stm, temp) || callObjectTempNum(stm) == temp;
}

static bool tempDefinedByLoad(
    int temp,
    const std::unordered_map<int, const quad::QuadStm*> &defByTemp
) {
    auto it = defByTemp.find(temp);
    return it != defByTemp.end() && it->second != nullptr && it->second->kind == quad::QuadKind::LOAD;
}

static bool tempDefinedByLoadOrCall(
    int temp,
    const std::unordered_map<int, const quad::QuadStm*> &defByTemp
) {
    auto it = defByTemp.find(temp);
    if (it == defByTemp.end() || it->second == nullptr) {
        return false;
    }
    return it->second->kind == quad::QuadKind::LOAD ||
           it->second->kind == quad::QuadKind::CALL ||
           it->second->kind == quad::QuadKind::MOVE_CALL;
}

static bool tempDefinedByCall(
    int temp,
    const std::unordered_map<int, const quad::QuadStm*> &defByTemp
) {
    auto it = defByTemp.find(temp);
    if (it == defByTemp.end() || it->second == nullptr) {
        return false;
    }
    return it->second->kind == quad::QuadKind::CALL ||
           it->second->kind == quad::QuadKind::MOVE_CALL;
}

static size_t insertionAfterMallocInit(
    const std::vector<const quad::QuadStm*> &statements,
    size_t mallocIndex
) {
    auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(statements[mallocIndex]);
    int mallocTemp = moveExt == nullptr ? -1 : tempNum(quadTempPtr(moveExt->dst));
    if (mallocTemp >= 0 && mallocIndex + 1 < statements.size() &&
        isDirectStoreToTemp(statements[mallocIndex + 1], mallocTemp)) {
        return mallocIndex + 2;
    }
    return mallocIndex + 1;
}

static void delayCallOnlyMoves(
    std::vector<const quad::QuadStm*> &statements,
    const std::unordered_map<int, int> &useCounts,
    const std::unordered_map<int, const quad::QuadStm*> &defByTemp
) {
    for (size_t index = 0; index < statements.size(); ++index) {
        int dst = -1;
        int src = -1;
        if (!moveTempToTemp(statements[index], dst, src)) {
            continue;
        }

        auto useIt = useCounts.find(dst);
        if (useIt == useCounts.end() || useIt->second != 1) {
            continue;
        }

        size_t callIndex = statements.size();
        for (size_t scan = index + 1; scan < statements.size(); ++scan) {
            if (tempUsedByStatement(statements[scan], dst) && isInternalCallLike(statements[scan])) {
                callIndex = scan;
                break;
            }
        }
        if (callIndex == statements.size()) {
            continue;
        }

        size_t target = index + 1;
        if (tempDefinedByLoadOrCall(src, defByTemp)) {
            for (size_t scan = index + 1; scan < callIndex; ++scan) {
                if (statements[scan] != nullptr && statements[scan]->kind == quad::QuadKind::LOAD) {
                    target = scan + 1;
                }
                if (isNonMallocCallLike(statements[scan])) {
                    target = scan + 1;
                }
            }
        } else {
            for (size_t scan = index + 1; scan < callIndex; ++scan) {
                if (isMallocMoveExtCall(statements[scan])) {
                    target = insertionAfterMallocInit(statements, scan);
                }
            }
        }

        if (target <= index + 1) {
            continue;
        }

        auto *move = statements[index];
        statements.erase(statements.begin() + index);
        if (target > index) {
            --target;
        }
        statements.insert(statements.begin() + target, move);
        if (target > 0) {
            index = target - 1;
        }
    }
}

static bool ptrCalcTemp(const quad::QuadStm *stm, int &dst) {
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(stm);
    if (ptrCalc == nullptr) {
        return false;
    }
    dst = tempNum(termTemp(ptrCalc->dst));
    return dst >= 0;
}

static void delayIndirectCallAddressCalcs(
    std::vector<const quad::QuadStm*> &statements,
    const std::unordered_map<int, int> &useCounts
) {
    for (size_t index = 0; index < statements.size(); ++index) {
        int ptrTemp = -1;
        if (!ptrCalcTemp(statements[index], ptrTemp)) {
            continue;
        }
        auto useIt = useCounts.find(ptrTemp);
        if (useIt == useCounts.end() || useIt->second != 1) {
            continue;
        }

        size_t loadIndex = statements.size();
        const quad::QuadLoad *load = nullptr;
        for (size_t scan = index + 1; scan < statements.size(); ++scan) {
            auto *candidate = dynamic_cast<const quad::QuadLoad*>(statements[scan]);
            if (candidate != nullptr && tempNum(termTemp(candidate->src)) == ptrTemp) {
                loadIndex = scan;
                load = candidate;
                break;
            }
        }
        if (load == nullptr || !loadResultFeedsCallAcrossCall(load, statements, loadIndex)) {
            continue;
        }

        size_t target = index + 1;
        for (size_t scan = index + 1; scan < loadIndex; ++scan) {
            if (isMallocMoveExtCall(statements[scan])) {
                target = insertionAfterMallocInit(statements, scan);
            }
        }
        if (target <= index + 1) {
            continue;
        }

        auto *ptrCalc = statements[index];
        statements.erase(statements.begin() + index);
        if (target > index) {
            --target;
        }
        statements.insert(statements.begin() + target, ptrCalc);
        if (target > 0) {
            index = target - 1;
        }
    }
}

static void delayCallResultCopiesPastLoads(
    std::vector<const quad::QuadStm*> &statements,
    const std::unordered_map<int, const quad::QuadStm*> &defByTemp
) {
    for (size_t index = 0; index < statements.size(); ++index) {
        int dst = -1;
        int src = -1;
        if (!moveTempToTemp(statements[index], dst, src) || !tempDefinedByCall(src, defByTemp)) {
            continue;
        }

        size_t target = index + 1;
        bool usedBeforeTarget = false;
        for (size_t scan = index + 1; scan < statements.size(); ++scan) {
            if (usesTempNum(statements[scan], dst)) {
                usedBeforeTarget = true;
                break;
            }
            if (statements[scan] != nullptr && statements[scan]->kind == quad::QuadKind::LOAD) {
                target = scan + 1;
                continue;
            }
            if (isMemoryOrCallLike(statements[scan])) {
                break;
            }
        }
        if (usedBeforeTarget || target <= index + 1) {
            continue;
        }

        auto *move = statements[index];
        statements.erase(statements.begin() + index);
        if (target > index) {
            --target;
        }
        statements.insert(statements.begin() + target, move);
        if (target > 0) {
            index = target - 1;
        }
    }
}

} // namespace

// Main instruction selection for a block
void selectInstructionsForBlock(
    const advDFGblock &blockGraph,
    preScheduleBlock &schedBlock,
    int &nextTempNum
) {
    const auto& graph = blockGraph.graph;
    const auto& nodes = graph.getNodes();
    if (nodes.empty()) {
        return;
    }

    std::vector<const quad::QuadStm*> statements;
    statements.reserve(nodes.size());
    for (auto *node : nodes) {
        if (node != nullptr && node->quadStatement != nullptr) {
            statements.push_back(node->quadStatement);
        }
    }

    std::unordered_map<int, int> useCounts;
    std::unordered_map<int, const quad::QuadStm*> singleUser;
    std::unordered_map<int, const quad::QuadStm*> defByTemp;
    std::unordered_map<int, PtrCalcInfo> allPtrCalcInfos;
    std::unordered_map<const quad::QuadStm*, size_t> statementIndex;
    for (size_t index = 0; index < statements.size(); ++index) {
        auto *stm = statements[index];
        statementIndex[stm] = index;
        int defTemp = definedTempNum(stm);
        if (defTemp >= 0) {
            defByTemp[defTemp] = stm;
        }
        if (stm == nullptr || stm->use == nullptr) {
            continue;
        }
        for (auto *temp : *stm->use) {
            int num = tempNum(temp);
            if (num < 0) {
                continue;
            }
            ++useCounts[num];
            if (singleUser.find(num) == singleUser.end()) {
                singleUser[num] = stm;
            } else {
                singleUser[num] = nullptr;
            }
        }
    }

    for (auto *stm : statements) {
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(stm);
        if (ptrCalc == nullptr) {
            continue;
        }

        auto *dst = termTemp(ptrCalc->dst);
        int dstNum = tempNum(dst);
        PtrCalcInfo info;
        if (dstNum >= 0 && buildPtrCalcInfo(ptrCalc, info)) {
            allPtrCalcInfos[dstNum] = info;
        }
    }

    std::unordered_map<int, int> storeBaseCounts;
    for (auto *stm : statements) {
        auto *store = dynamic_cast<const quad::QuadStore*>(stm);
        if (store == nullptr) {
            continue;
        }
        int base = addressBaseTempNum(store->dst, allPtrCalcInfos);
        if (base >= 0) {
            ++storeBaseCounts[base];
        }
    }

    std::unordered_set<int> foldedPtrCalcTemps;
    std::unordered_map<int, PtrCalcInfo> foldedPtrCalcs;
    for (auto *stm : statements) {
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(stm);
        if (ptrCalc == nullptr) {
            continue;
        }

        auto *dst = termTemp(ptrCalc->dst);
        int dstNum = tempNum(dst);
        PtrCalcInfo info;
        if (dstNum < 0 || !buildPtrCalcInfo(ptrCalc, info)) {
            continue;
        }

        auto userIt = singleUser.find(dstNum);
        if (useCounts[dstNum] != 1 || userIt == singleUser.end() || userIt->second == nullptr) {
            continue;
        }

        bool crossesInternalCall = false;
        auto ptrIndexIt = statementIndex.find(stm);
        size_t ptrIndex = ptrIndexIt == statementIndex.end() ? statements.size() : ptrIndexIt->second;
        for (size_t index = 0; index < ptrIndex; ++index) {
            if (isInternalCallLike(statements[index])) {
                crossesInternalCall = true;
                break;
            }
        }
        if (userIt->second->kind == quad::QuadKind::LOAD) {
            auto loadIndexIt = statementIndex.find(userIt->second);
            size_t loadIndex = loadIndexIt == statementIndex.end() ? statements.size() : loadIndexIt->second;
            crossesInternalCall = crossesInternalCall ||
                                  loadResultFeedsCallAcrossCall(
                                      dynamic_cast<const quad::QuadLoad*>(userIt->second),
                                      statements,
                                      loadIndex);
        }

        auto *storeUser = dynamic_cast<const quad::QuadStore*>(userIt->second);
        int baseStoreCount = storeBaseCounts[tempNum(info.base)];
        bool nameStoreWithOffset = storeUser != nullptr &&
                                   storeUser->src != nullptr &&
                                   storeUser->src->kind == quad::QuadTermKind::NAME &&
                                   infoHasNonZeroOffset(info) &&
                                   baseStoreCount > 1;
        bool canFold = userIt->second->kind == quad::QuadKind::LOAD && !crossesInternalCall;
        if (userIt->second->kind == quad::QuadKind::STORE) {
            auto *store = dynamic_cast<const quad::QuadStore*>(userIt->second);
            bool sourceIsConst = store != nullptr &&
                                 store->src != nullptr &&
                                 store->src->kind == quad::QuadTermKind::CONST;
            bool sourceIsZeroOffsetName = store != nullptr &&
                                          store->src != nullptr &&
                                          store->src->kind == quad::QuadTermKind::NAME &&
                                          !infoHasNonZeroOffset(info);
            bool sourceIsSimpleLoad = false;
            if (store != nullptr && store->src != nullptr && store->src->kind == quad::QuadTermKind::TEMP) {
                int srcNum = tempNum(termTemp(store->src));
                auto defIt = defByTemp.find(srcNum);
                auto *load = defIt == defByTemp.end() ? nullptr : dynamic_cast<const quad::QuadLoad*>(defIt->second);
                sourceIsSimpleLoad = load != nullptr && !addressHasDynamicOffset(load->src, allPtrCalcInfos);
            }
            bool sourceIsNameSingleStore = store != nullptr &&
                                           store->src != nullptr &&
                                           store->src->kind == quad::QuadTermKind::NAME &&
                                           baseStoreCount <= 1;
            canFold = sourceIsConst || sourceIsZeroOffsetName ||
                      sourceIsSimpleLoad || sourceIsNameSingleStore;
        }

        if (canFold && !nameStoreWithOffset) {
            foldedPtrCalcTemps.insert(dstNum);
            foldedPtrCalcs[dstNum] = info;
        }
    }

    auto orderedStatements = orderSelectableStatements(nodes, foldedPtrCalcTemps, foldedPtrCalcs);
    prioritizeDirectMallocStores(orderedStatements);
    delayCallOnlyMoves(orderedStatements, useCounts, defByTemp);
    delayIndirectCallAddressCalcs(orderedStatements, useCounts);
    delayCallResultCopiesPastLoads(orderedStatements, defByTemp);

    for (auto *stm : orderedStatements) {
        if (stm == nullptr) {
            continue;
        }

        if (stm->kind == quad::QuadKind::PTR_CALC) {
            auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(stm);
            appendPtrCalc(schedBlock.selectedInstructions, ptrCalc, nextTempNum);
            continue;
        }

        if (stm->kind == quad::QuadKind::MOVE) {
            auto *move = dynamic_cast<const quad::QuadMove*>(stm);
            if (move != nullptr) {
                appendMoveFromTerm(schedBlock.selectedInstructions, quadTempPtr(move->dst), move->src, nextTempNum);
            }
            continue;
        }

        if (stm->kind == quad::QuadKind::LOAD) {
            auto *load = dynamic_cast<const quad::QuadLoad*>(stm);
            if (load != nullptr) {
                AddressMode address = makeAddress(load->src, foldedPtrCalcs, true);
                appendLoad(schedBlock.selectedInstructions, quadTempPtr(load->dst), address);
            }
            continue;
        }

        if (stm->kind == quad::QuadKind::STORE) {
            auto *store = dynamic_cast<const quad::QuadStore*>(stm);
            if (store != nullptr) {
                bool allowFold = true;
                if (store->src != nullptr && store->src->kind == quad::QuadTermKind::NAME &&
                    store->dst != nullptr && store->dst->kind == quad::QuadTermKind::TEMP) {
                    int dstNum = tempNum(termTemp(store->dst));
                    auto it = foldedPtrCalcs.find(dstNum);
                    int base = it == foldedPtrCalcs.end() ? -1 : tempNum(it->second.base);
                    int baseStoreCount = base < 0 ? 0 : storeBaseCounts[base];
                    allowFold = it == foldedPtrCalcs.end() ||
                                !infoHasNonZeroOffset(it->second) ||
                                baseStoreCount <= 1;
                }
                auto *src = materializeTerm(store->src, schedBlock.selectedInstructions, nextTempNum);
                AddressMode address = makeAddress(store->dst, foldedPtrCalcs, allowFold);
                appendStore(schedBlock.selectedInstructions, src, address);
            }
            continue;
        }

        if (stm->kind == quad::QuadKind::MOVE_BINOP) {
            appendBinop(schedBlock.selectedInstructions, dynamic_cast<const quad::QuadMoveBinop*>(stm), nextTempNum);
            continue;
        }

        if (stm->kind == quad::QuadKind::EXTCALL) {
            auto *extcall = dynamic_cast<const quad::QuadExtCall*>(stm);
            appendExtCall(schedBlock.selectedInstructions, extcall, nullptr, nextTempNum);
            continue;
        }

        if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
            auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
            if (moveExt != nullptr) {
                appendExtCall(schedBlock.selectedInstructions, moveExt->extcall, quadTempPtr(moveExt->dst), nextTempNum);
            }
            continue;
        }

        if (stm->kind == quad::QuadKind::CALL) {
            auto *call = dynamic_cast<const quad::QuadCall*>(stm);
            appendCall(schedBlock.selectedInstructions, call, nullptr, nextTempNum);
            continue;
        }

        if (stm->kind == quad::QuadKind::MOVE_CALL) {
            auto *moveCall = dynamic_cast<const quad::QuadMoveCall*>(stm);
            if (moveCall != nullptr) {
                appendCall(schedBlock.selectedInstructions, moveCall->call, quadTempPtr(moveCall->dst), nextTempNum);
            }
            continue;
        }
    }

    return;
}

void runInstructionSelectionPass(
    const advDFGprog &graphProgram,
    preScheduleProg &preScheduleProgram
) {
    size_t funcCount = std::min(graphProgram.fungraph.size(), preScheduleProgram.funcSchedules.size());
    for (size_t funcIndex = 0; funcIndex < funcCount; ++funcIndex) {
        auto *funcGraph = graphProgram.fungraph[funcIndex];
        auto *funcSchedule = preScheduleProgram.funcSchedules[funcIndex];
        if (funcGraph == nullptr || funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) {
            continue;
        }

        int nextTempNum = funcSchedule->quadFunc->last_temp_num + 1;
        size_t blockCount = std::min(funcGraph->blockgraph.size(), funcSchedule->blockSchedules.size());
        for (size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            auto *blockGraph = funcGraph->blockgraph[blockIndex];
            auto *blockSchedule = funcSchedule->blockSchedules[blockIndex];
            if (blockGraph == nullptr || blockSchedule == nullptr) {
                continue;
            }

            selectInstructionsForBlock(*blockGraph, *blockSchedule, nextTempNum);
        }

        auto *mutableFunc = const_cast<quad::QuadFuncDecl*>(funcSchedule->quadFunc);
        mutableFunc->last_temp_num = nextTempNum - 1;
    }
}

} // namespace instr
