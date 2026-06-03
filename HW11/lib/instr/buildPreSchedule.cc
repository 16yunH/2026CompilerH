#include "preSchedule.hh"

#include <vector>

namespace instr {

static bool isExitCallStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }

    if (stm->kind == quad::QuadKind::EXTCALL) {
        auto *ext = dynamic_cast<const quad::QuadExtCall *>(stm);
        return ext != nullptr && ext->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall *>(stm);
        return moveExt != nullptr && moveExt->extcall != nullptr && moveExt->extcall->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::CALL) {
        auto *call = dynamic_cast<const quad::QuadCall *>(stm);
        return call != nullptr && call->name == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall *>(stm);
        return moveCall != nullptr && moveCall->call != nullptr && moveCall->call->name == "exit";
    }

    return false;
}

static bool isBlockFinalStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == quad::QuadKind::RETURN ||
           stm->kind == quad::QuadKind::JUMP ||
           stm->kind == quad::QuadKind::CJUMP ||
           isExitCallStmt(stm);
}

preScheduleProg *buildPreScheduleProg(const quad::QuadProgram *program) {
    auto *out = new preScheduleProg(program);
    if (program == nullptr || program->quadFuncDeclList == nullptr) {
        return out;
    }

    size_t funcIndex = 0;
    for (auto *func : *program->quadFuncDeclList) {
        if (func == nullptr) {
            ++funcIndex;
            continue;
        }

        auto *funcSchedule = new preScheduleFunc(func);
        out->addFunc(funcSchedule);

        if (func->quadblocklist != nullptr) {
            for (auto *block : *func->quadblocklist) {
                auto *blockSchedule = new preScheduleBlock(block);
                funcSchedule->addBlock(blockSchedule);

                if (block == nullptr || block->quadlist == nullptr || block->quadlist->empty()) {
                    continue;
                }

                std::vector<const quad::QuadStm*> nonPhiStatements;
                nonPhiStatements.reserve(block->quadlist->size());

                for (auto *stm : *block->quadlist) {
                    if (stm == nullptr) {
                        continue;
                    }
                    if (stm->kind == quad::QuadKind::PHI) {
                        auto *phi = dynamic_cast<const quad::QuadPhi *>(stm);
                        if (phi != nullptr) {
                            blockSchedule->addPhi(phi);
                        }
                        continue;
                    }
                    nonPhiStatements.push_back(stm);
                }

                if (!nonPhiStatements.empty()) {
                    for (auto it = nonPhiStatements.rbegin(); it != nonPhiStatements.rend(); ++it) {
                        if (isBlockFinalStmt(*it)) {
                            blockSchedule->lastInstruction = *it;
                            break;
                        }
                    }
                    if (blockSchedule->lastInstruction == nullptr) {
                        blockSchedule->lastInstruction = nonPhiStatements.back();
                    }
                }

            }
        }

        ++funcIndex;
    }

    return out;
}

} // namespace instr
