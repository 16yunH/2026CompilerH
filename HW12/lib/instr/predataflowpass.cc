#include "asmprogpass.hh"
#include "temp.hh"

#include <algorithm>
#include <string>
#include <vector>

namespace instr {

namespace {

static tree::Temp *regTemp(int reg) {
    return new tree::Temp(reg);
}

static bool hasTemp(const std::vector<tree::Temp*> &temps, int num) {
    return std::any_of(
        temps.begin(),
        temps.end(),
        [num](tree::Temp *temp) {
            return temp != nullptr && temp->num == num;
        });
}

static void addTemp(std::vector<tree::Temp*> &temps, int num) {
    if (!hasTemp(temps, num)) {
        temps.push_back(regTemp(num));
    }
}

static bool startsWith(const std::string &text, const std::string &prefix) {
    return text.rfind(prefix, 0) == 0;
}

static bool isCallInstruction(const AssemInstr &instr) {
    return instr.kind == AssemInstr::I_CALL ||
           instr.kind == AssemInstr::I_EXTCALL ||
           startsWith(instr.assem, "bl ") ||
           startsWith(instr.assem, "blx ");
}

static bool isReturnInstruction(const AssemInstr &instr) {
    return instr.assem == "bx lr";
}

} // namespace

void preDataFlowPass(AsmProg* prog) {
    if (prog == nullptr) {
        return;
    }

    for (auto &func : prog->functions) {
        for (auto &instr : func.instructions) {
            if (isCallInstruction(instr)) {
                // ARM caller-saved registers. Adding them as defs prevents
                // temps live across calls from being colored to clobbered regs.
                for (int reg : {0, 1, 2, 3, 12, 14}) {
                    addTemp(instr.dst, reg);
                }
            }

            if (isReturnInstruction(instr)) {
                addTemp(instr.src, 14);
            }
        }
    }
}

} // namespace instr
