#include "schedule.hh"

#include "advDFG.hh"
#include "instrSelection.hh"

#include <algorithm>
#include <functional>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace instr {

namespace {

using TempMove = std::pair<tree::Temp*, tree::Temp*>; // dst, src

static int labelNum(tree::Label *label) {
    return label == nullptr ? -1 : label->num;
}

static int tempNum(tree::Temp *temp) {
    return temp == nullptr ? -1 : temp->num;
}

static tree::Temp *newTemp(int &nextTempNum) {
    return new tree::Temp(nextTempNum++);
}

static tree::Label *newLabel(int &nextLabelNum) {
    return new tree::Label(nextLabelNum++);
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

static bool sameLabel(tree::Label *left, tree::Label *right) {
    return labelNum(left) >= 0 && labelNum(left) == labelNum(right);
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

static std::string branchMnemonic(const std::string &relop) {
    if (relop == "==") {
        return "beq";
    }
    if (relop == "!=") {
        return "bne";
    }
    if (relop == "<") {
        return "blt";
    }
    if (relop == "<=") {
        return "ble";
    }
    if (relop == ">") {
        return "bgt";
    }
    if (relop == ">=") {
        return "bge";
    }
    return "bne";
}

static void appendPrologue(ScheduleFunc *funcSchedule) {
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("push {r4-r10, fp, lr}", {}, {}, AssemTargets()));
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("sub sp, sp, #4", {}, {}, AssemTargets()));
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("add fp, sp, #36", {}, {}, AssemTargets()));
}

static void appendEpilogue(ScheduleFunc *funcSchedule) {
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("sub sp, fp, #36", {}, {}, AssemTargets()));
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("add sp, sp, #4", {}, {}, AssemTargets()));
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("pop {r4-r10, fp, lr}", {}, {}, AssemTargets()));
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("bx lr", {}, {}, AssemTargets()));
}

static void appendParamMoves(ScheduleFunc *funcSchedule, const quad::QuadFuncDecl *quadFunc) {
    if (quadFunc == nullptr || quadFunc->params == nullptr) {
        return;
    }

    for (size_t index = 0; index < quadFunc->params->size(); ++index) {
        auto *param = (*quadFunc->params)[index];
        if (param == nullptr) {
            continue;
        }
        if (index < 4) {
            funcSchedule->addLinearizedInstruction(
                AssemInstr::Move("mov `d0, r" + std::to_string(index), {param}, {}));
        } else {
            int offset = 4 + static_cast<int>((index - 4) * 4);
            funcSchedule->addLinearizedInstruction(
                AssemInstr::Oper("ldr `d0, [fp, #" + std::to_string(offset) + "]", {param}, {}, AssemTargets()));
        }
    }
}

static preScheduleBlock *findBlock(
    const std::unordered_map<int, preScheduleBlock*> &blocksByLabel,
    tree::Label *label
) {
    auto it = blocksByLabel.find(labelNum(label));
    return it == blocksByLabel.end() ? nullptr : it->second;
}

static std::vector<preScheduleBlock*> buildLayout(preScheduleFunc *funcSchedule) {
    std::vector<preScheduleBlock*> layout;
    if (funcSchedule == nullptr) {
        return layout;
    }

    std::unordered_map<int, preScheduleBlock*> blocksByLabel;
    for (auto *block : funcSchedule->blockSchedules) {
        if (block != nullptr && block->entryLabel != nullptr) {
            blocksByLabel[block->entryLabel->num] = block;
        }
    }

    std::unordered_set<int> placed;
    std::function<void(preScheduleBlock*)> place = [&](preScheduleBlock *block) {
        if (block == nullptr || block->entryLabel == nullptr) {
            return;
        }
        int label = block->entryLabel->num;
        if (placed.find(label) != placed.end()) {
            return;
        }

        placed.insert(label);
        layout.push_back(block);

        const auto *last = block->lastInstruction;
        if (last == nullptr) {
            return;
        }
        if (last->kind == quad::QuadKind::JUMP) {
            auto *jump = dynamic_cast<const quad::QuadJump*>(last);
            place(findBlock(blocksByLabel, jump == nullptr ? nullptr : jump->label));
            return;
        }
        if (last->kind == quad::QuadKind::CJUMP) {
            auto *cjump = dynamic_cast<const quad::QuadCJump*>(last);
            if (cjump != nullptr) {
                place(findBlock(blocksByLabel, cjump->f));
                place(findBlock(blocksByLabel, cjump->t));
            }
        }
    };

    if (!funcSchedule->blockSchedules.empty()) {
        place(funcSchedule->blockSchedules.front());
    }
    for (auto *block : funcSchedule->blockSchedules) {
        place(block);
    }

    return layout;
}

static std::vector<TempMove> collectPhiMoves(
    preScheduleBlock *successor,
    tree::Label *predecessorLabel
) {
    std::vector<TempMove> moves;
    if (successor == nullptr || predecessorLabel == nullptr) {
        return moves;
    }

    for (auto *phi : successor->phiFunctions) {
        if (phi == nullptr || phi->args == nullptr || phi->temp_exp == nullptr) {
            continue;
        }

        auto *dst = quadTempPtr(phi->temp_exp);
        for (const auto &arg : *phi->args) {
            if (arg.first != nullptr && sameLabel(arg.second, predecessorLabel) &&
                tempNum(dst) != tempNum(arg.first)) {
                moves.emplace_back(dst, arg.first);
                break;
            }
        }
    }
    return moves;
}

static void appendParallelMoves(
    ScheduleFunc *funcSchedule,
    std::vector<TempMove> moves,
    int &nextTempNum
) {
    moves.erase(
        std::remove_if(
            moves.begin(),
            moves.end(),
            [](const TempMove &move) {
                return move.first == nullptr || move.second == nullptr ||
                       tempNum(move.first) == tempNum(move.second);
            }),
        moves.end());

    while (!moves.empty()) {
        bool emitted = false;
        for (auto it = moves.begin(); it != moves.end(); ++it) {
            bool dstUsedAsSource = false;
            for (const auto &other : moves) {
                if (&other != &(*it) && tempNum(other.second) == tempNum(it->first)) {
                    dstUsedAsSource = true;
                    break;
                }
            }

            if (!dstUsedAsSource) {
                funcSchedule->addLinearizedInstruction(
                    AssemInstr::Move("mov `d0, `s0", {it->first}, {it->second}));
                moves.erase(it);
                emitted = true;
                break;
            }
        }

        if (emitted) {
            continue;
        }

        auto *tmp = newTemp(nextTempNum);
        auto source = moves.front().second;
        funcSchedule->addLinearizedInstruction(AssemInstr::Move("mov `d0, `s0", {tmp}, {source}));
        for (auto &move : moves) {
            if (tempNum(move.second) == tempNum(source)) {
                move.second = tmp;
                break;
            }
        }
    }
}

static void appendPhiMovesForEdge(
    ScheduleFunc *funcSchedule,
    preScheduleBlock *successor,
    tree::Label *predecessorLabel,
    int &nextTempNum
) {
    appendParallelMoves(funcSchedule, collectPhiMoves(successor, predecessorLabel), nextTempNum);
}

static void appendBranchTo(ScheduleFunc *funcSchedule, tree::Label *target) {
    if (target == nullptr) {
        return;
    }
    funcSchedule->addLinearizedInstruction(
        AssemInstr::Oper("b `j0", {}, {}, AssemTargets({target})));
}

static AssemInstr makeLabelInstr(tree::Label *label) {
    return AssemInstr::Label(label == nullptr ? "" : label->str() + ":", label);
}

static void appendReturn(
    ScheduleFunc *funcSchedule,
    const quad::QuadReturn *ret,
    int &nextTempNum
) {
    if (ret != nullptr && ret->exp != nullptr) {
        auto *value = materializeTerm(ret->exp, funcSchedule->linearizedInstructions, nextTempNum);
        if (value != nullptr) {
            funcSchedule->addLinearizedInstruction(AssemInstr::Move("mov r0, `s0", {}, {value}));
        }
    }
    appendEpilogue(funcSchedule);
}

static void appendCallArgsForTermList(
    ScheduleFunc *funcSchedule,
    const std::vector<quad::QuadTerm*> *args,
    int &nextTempNum
) {
    if (args == nullptr) {
        return;
    }

    std::vector<tree::Temp*> regArgs;
    size_t regCount = std::min<size_t>(args->size(), 4);
    regArgs.reserve(regCount);
    for (size_t index = 0; index < regCount; ++index) {
        regArgs.push_back(materializeTerm(args->at(index), funcSchedule->linearizedInstructions, nextTempNum));
    }
    for (size_t index = args->size(); index > 4; --index) {
        auto *arg = materializeTerm(args->at(index - 1), funcSchedule->linearizedInstructions, nextTempNum);
        funcSchedule->addLinearizedInstruction(
            AssemInstr::Oper("str `s0, [sp, #-4]!", {}, {arg}, AssemTargets()));
    }
    for (size_t index = 0; index < regArgs.size(); ++index) {
        funcSchedule->addLinearizedInstruction(
            AssemInstr::Move("mov r" + std::to_string(index) + ", `s0", {}, {regArgs[index]}));
    }
}

static void appendStackArgCleanupForTermList(
    ScheduleFunc *funcSchedule,
    const std::vector<quad::QuadTerm*> *args
) {
    if (args != nullptr && args->size() > 4) {
        size_t bytes = (args->size() - 4) * 4;
        funcSchedule->addLinearizedInstruction(
            AssemInstr::Oper("add sp, sp, #" + std::to_string(bytes), {}, {}, AssemTargets()));
    }
}

static void appendExitCall(
    ScheduleFunc *funcSchedule,
    const quad::QuadStm *stm,
    int &nextTempNum
) {
    if (stm == nullptr) {
        return;
    }

    if (stm->kind == quad::QuadKind::EXTCALL) {
        auto *ext = dynamic_cast<const quad::QuadExtCall*>(stm);
        if (ext != nullptr) {
            appendCallArgsForTermList(funcSchedule, ext->args, nextTempNum);
            funcSchedule->addLinearizedInstruction(AssemInstr::ExtCall("bl " + ext->extfun, {}, {}));
            appendStackArgCleanupForTermList(funcSchedule, ext->args);
        }
    } else if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
        if (moveExt != nullptr && moveExt->extcall != nullptr) {
            appendCallArgsForTermList(funcSchedule, moveExt->extcall->args, nextTempNum);
            funcSchedule->addLinearizedInstruction(AssemInstr::ExtCall("bl " + moveExt->extcall->extfun, {}, {}));
            appendStackArgCleanupForTermList(funcSchedule, moveExt->extcall->args);
        }
    }
}

static void appendJump(
    ScheduleFunc *funcSchedule,
    const quad::QuadJump *jump,
    preScheduleBlock *current,
    preScheduleBlock *nextBlock,
    const std::unordered_map<int, preScheduleBlock*> &blocksByLabel,
    int &nextTempNum
) {
    if (jump == nullptr) {
        return;
    }

    auto *targetBlock = findBlock(blocksByLabel, jump->label);
    appendPhiMovesForEdge(funcSchedule, targetBlock, current == nullptr ? nullptr : current->entryLabel, nextTempNum);
    if (nextBlock == nullptr || !sameLabel(nextBlock->entryLabel, jump->label)) {
        appendBranchTo(funcSchedule, jump->label);
    }
}

static void appendCJump(
    ScheduleFunc *funcSchedule,
    const quad::QuadCJump *cjump,
    preScheduleBlock *current,
    preScheduleBlock *nextBlock,
    const std::unordered_map<int, preScheduleBlock*> &blocksByLabel,
    int &nextTempNum,
    int &nextLabelNum
) {
    if (cjump == nullptr) {
        return;
    }

    auto *left = materializeTerm(cjump->left, funcSchedule->linearizedInstructions, nextTempNum);
    auto *right = materializeTerm(cjump->right, funcSchedule->linearizedInstructions, nextTempNum);
    funcSchedule->addLinearizedInstruction(AssemInstr::Oper("cmp `s0, `s1", {}, {left, right}, AssemTargets()));

    auto *trueBlock = findBlock(blocksByLabel, cjump->t);
    auto *falseBlock = findBlock(blocksByLabel, cjump->f);
    auto trueMoves = collectPhiMoves(trueBlock, current == nullptr ? nullptr : current->entryLabel);
    auto falseMoves = collectPhiMoves(falseBlock, current == nullptr ? nullptr : current->entryLabel);

    if (!trueMoves.empty()) {
        auto *edgeLabel = newLabel(nextLabelNum);
        funcSchedule->addLinearizedInstruction(
            AssemInstr::Oper(branchMnemonic(cjump->relop) + " `j0", {}, {}, AssemTargets({edgeLabel})));
        appendParallelMoves(funcSchedule, falseMoves, nextTempNum);
        appendBranchTo(funcSchedule, cjump->f);
        funcSchedule->addLinearizedInstruction(makeLabelInstr(edgeLabel));
        appendParallelMoves(funcSchedule, trueMoves, nextTempNum);
        appendBranchTo(funcSchedule, cjump->t);
        return;
    }

    funcSchedule->addLinearizedInstruction(
        AssemInstr::Oper(branchMnemonic(cjump->relop) + " `j0", {}, {}, AssemTargets({cjump->t})));
    appendParallelMoves(funcSchedule, falseMoves, nextTempNum);

    if (nextBlock == nullptr || !sameLabel(nextBlock->entryLabel, cjump->f)) {
        appendBranchTo(funcSchedule, cjump->f);
    }
}

} // namespace

ScheduleProg *scheduleProg(preScheduleProg *preScheduleProgram) {
    if (preScheduleProgram == nullptr) {
        return nullptr;
    }

    auto *out = new ScheduleProg(preScheduleProgram->quadProgram);

    for (auto *funcPreSchedule : preScheduleProgram->funcSchedules) {
        if (funcPreSchedule == nullptr || funcPreSchedule->quadFunc == nullptr) {
            continue;
        }

        auto *funcSchedule = new ScheduleFunc(funcPreSchedule->quadFunc);
        out->addFunc(funcSchedule);

        std::unordered_map<int, preScheduleBlock*> blocksByLabel;
        for (auto *block : funcPreSchedule->blockSchedules) {
            if (block != nullptr && block->entryLabel != nullptr) {
                blocksByLabel[block->entryLabel->num] = block;
            }
        }

        auto layout = buildLayout(funcPreSchedule);
        int nextTempNum = funcPreSchedule->quadFunc->last_temp_num + 1;
        int nextLabelNum = funcPreSchedule->quadFunc->last_label_num + 1;

        for (size_t index = 0; index < layout.size(); ++index) {
            auto *block = layout[index];
            if (block == nullptr) {
                continue;
            }
            auto *nextBlock = index + 1 < layout.size() ? layout[index + 1] : nullptr;

            if (block->entryLabel != nullptr) {
                funcSchedule->addLinearizedInstruction(makeLabelInstr(block->entryLabel));
            }

            if (index == 0) {
                appendPrologue(funcSchedule);
                appendParamMoves(funcSchedule, funcPreSchedule->quadFunc);
            }

            funcSchedule->linearizedInstructions.extend(block->selectedInstructions);

            const auto *last = block->lastInstruction;
            if (last == nullptr) {
                continue;
            }

            if (last->kind == quad::QuadKind::RETURN) {
                appendReturn(funcSchedule, dynamic_cast<const quad::QuadReturn*>(last), nextTempNum);
            } else if (last->kind == quad::QuadKind::JUMP) {
                appendJump(
                    funcSchedule,
                    dynamic_cast<const quad::QuadJump*>(last),
                    block,
                    nextBlock,
                    blocksByLabel,
                    nextTempNum);
            } else if (last->kind == quad::QuadKind::CJUMP) {
                appendCJump(
                    funcSchedule,
                    dynamic_cast<const quad::QuadCJump*>(last),
                    block,
                    nextBlock,
                    blocksByLabel,
                    nextTempNum,
                    nextLabelNum);
            } else if (isExitCallStmt(last)) {
                appendExitCall(funcSchedule, last, nextTempNum);
            }
        }

        auto *mutableFunc = const_cast<quad::QuadFuncDecl*>(funcPreSchedule->quadFunc);
        mutableFunc->last_temp_num = std::max(mutableFunc->last_temp_num, nextTempNum - 1);
        mutableFunc->last_label_num = std::max(mutableFunc->last_label_num, nextLabelNum - 1);
    }

    return out;
}

} // namespace instr
