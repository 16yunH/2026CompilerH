#include "preSchedule.hh"

#include <iostream>

namespace instr {

static const char *assemKindToString(AssemInstr::Kind kind) {
    switch (kind) {
        case AssemInstr::I_OPER:
            return "I_OPER";
        case AssemInstr::I_LABEL:
            return "I_LABEL";
        case AssemInstr::I_MOVE:
            return "I_MOVE";
        case AssemInstr::I_CALL:
            return "I_CALL";
        case AssemInstr::I_EXTCALL:
            return "I_EXTCALL";
    }
    return "UNKNOWN";
}

static void printTempList(const std::vector<tree::Temp*> &temps) {
    std::cout << "<";
    bool first = true;
    for (auto *t : temps) {
        if (t == nullptr) {
            continue;
        }
        if (!first) {
            std::cout << ",";
        }
        std::cout << "t" << t->num;
        first = false;
    }
    std::cout << ">";
}

void preScheduleProg::printToStdout() const {
    std::cout << "preSchedule program dump" << std::endl;
    std::cout << "  function_count=" << funcSchedules.size() << std::endl;

    int funcIndex = 0;
    for (auto *funcSchedule : funcSchedules) {
        std::cout << "  func[" << funcIndex++ << "]";
        if (funcSchedule != nullptr && funcSchedule->quadFunc != nullptr) {
            std::cout << " name=" << funcSchedule->quadFunc->funcname;
        }
        std::cout << std::endl;

        if (funcSchedule == nullptr) {
            continue;
        }

        std::cout << "    block_count=" << funcSchedule->blockSchedules.size() << std::endl;
        int blockIndex = 0;
        for (auto *blockSchedule : funcSchedule->blockSchedules) {
            std::cout << "    block[" << blockIndex++ << "]";
            if (blockSchedule != nullptr && blockSchedule->entryLabel != nullptr) {
                std::cout << " entry=L" << blockSchedule->entryLabel->num;
            }
            std::cout << std::endl;

            if (blockSchedule == nullptr) {
                continue;
            }

            std::cout << "      phi_count=" << blockSchedule->phiFunctions.size() << std::endl;
            std::cout << "      selected_instruction_count="
                      << blockSchedule->selectedInstructions.instrs.size() << std::endl;
            for (const auto &instr : blockSchedule->selectedInstructions.instrs) {
                std::cout << "        instr kind=" << assemKindToString(instr.kind)
                          << " assem=\"" << instr.assem << "\""
                          << " dst=";
                printTempList(instr.dst);
                std::cout << " src=";
                printTempList(instr.src);
                std::cout << std::endl;
            }
            if (blockSchedule->lastInstruction != nullptr) {
                std::cout << "      last_instruction_kind="
                          << quad::quadKindToString(blockSchedule->lastInstruction->kind)
                          << std::endl;
            } else {
                std::cout << "      last_instruction_kind=none" << std::endl;
            }
        }

    }
}

} // namespace instr
