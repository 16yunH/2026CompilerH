#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <sstream>
#include <algorithm>
#include "asmprogpass.hh"
#include "temp.hh"

using namespace std;
using namespace tree;

namespace instr {

namespace {

static Temp *regTemp(int reg) {
    return new Temp(reg);
}

static bool isMachineReg(int temp) {
    return temp < 100;
}

static int tempNum(Temp *temp) {
    return temp == nullptr ? -1 : temp->num;
}

static bool isSpilled(int temp, const Coloring *coloring) {
    return coloring != nullptr && coloring->spilled.find(temp) != coloring->spilled.end();
}

static int colorFor(int temp, const Coloring *coloring) {
    if (isMachineReg(temp)) {
        return temp;
    }
    if (coloring == nullptr) {
        return temp;
    }
    auto it = coloring->colors.find(temp);
    return it == coloring->colors.end() ? temp : it->second;
}

static int stackOffsetForSlot(int slot) {
    return 36 + (slot + 1) * 4;
}

static int alignLocalBytes(int localBytes) {
    // The fixed prologue pushes 9 registers (36 bytes), so localBytes must be
    // 4 mod 8 to keep sp 8-byte aligned at calls.
    return localBytes % 8 == 4 ? localBytes : localBytes + 4;
}

static AssemInstr replaceFrameSize(const AssemInstr &instr, int localBytes, int frameBytes) {
    AssemInstr out = instr;
    if (regex_match(out.assem, regex(R"(sub sp, sp, #\d+)"))) {
        out.assem = "sub sp, sp, #" + to_string(localBytes);
    } else if (regex_match(out.assem, regex(R"(add fp, sp, #\d+)"))) {
        out.assem = "add fp, sp, #" + to_string(frameBytes);
    } else if (regex_match(out.assem, regex(R"(sub sp, fp, #\d+)"))) {
        out.assem = "sub sp, fp, #" + to_string(frameBytes);
    } else if (regex_match(out.assem, regex(R"(add sp, sp, #\d+)"))) {
        out.assem = "add sp, sp, #" + to_string(localBytes);
    }
    return out;
}

static bool isRedundantMove(const AssemInstr &instr) {
    if (instr.kind != AssemInstr::I_MOVE || instr.dst.size() != 1 || instr.src.size() != 1) {
        return false;
    }
    return tempNum(instr.dst.front()) >= 0 && tempNum(instr.dst.front()) == tempNum(instr.src.front());
}

static void appendLoadSpill(vector<AssemInstr> &out, int scratchReg, int offset) {
    out.push_back(
        AssemInstr::Oper(
            "ldr `d0, [fp, #-" + to_string(offset) + "]",
            {regTemp(scratchReg)},
            {},
            AssemTargets()));
}

static void appendStoreSpill(vector<AssemInstr> &out, int scratchReg, int offset) {
    out.push_back(
        AssemInstr::Oper(
            "str `s0, [fp, #-" + to_string(offset) + "]",
            {},
            {regTemp(scratchReg)},
            AssemTargets()));
}

class InstrColorer {
public:
    InstrColorer(const Coloring *coloring, const map<int, int> &spillSlots)
        : coloring(coloring), spillSlots(spillSlots) {}

    void colorInstruction(const AssemInstr &instr, AsmFunction &outFunc) {
        before.clear();
        after.clear();
        scratchForTemp.clear();
        nextScratch = 0;

        AssemInstr colored = instr;
        colored.dst.clear();
        colored.src.clear();

        for (auto *dst : instr.dst) {
            colored.dst.push_back(mapDst(dst));
        }
        for (auto *src : instr.src) {
            colored.src.push_back(mapSrc(src));
        }

        for (const auto &load : before) {
            outFunc.instructions.push_back(load);
        }
        if (!isRedundantMove(colored) || !after.empty()) {
            outFunc.instructions.push_back(colored);
        }
        for (const auto &store : after) {
            outFunc.instructions.push_back(store);
        }
    }

private:
    const Coloring *coloring;
    const map<int, int> &spillSlots;
    vector<AssemInstr> before;
    vector<AssemInstr> after;
    map<int, int> scratchForTemp;
    int nextScratch = 0;
    const vector<int> scratchRegs = {9, 10, 12};

    int scratchFor(int temp) {
        auto it = scratchForTemp.find(temp);
        if (it != scratchForTemp.end()) {
            return it->second;
        }
        int index = min<int>(nextScratch, static_cast<int>(scratchRegs.size()) - 1);
        int scratch = scratchRegs[index];
        ++nextScratch;
        scratchForTemp[temp] = scratch;
        return scratch;
    }

    Temp *mapSrc(Temp *temp) {
        int num = tempNum(temp);
        if (num < 0) {
            return nullptr;
        }
        if (!isSpilled(num, coloring)) {
            return regTemp(colorFor(num, coloring));
        }

        int scratch = scratchFor(num);
        auto slotIt = spillSlots.find(num);
        if (slotIt != spillSlots.end()) {
            appendLoadSpill(before, scratch, stackOffsetForSlot(slotIt->second));
        }
        return regTemp(scratch);
    }

    Temp *mapDst(Temp *temp) {
        int num = tempNum(temp);
        if (num < 0) {
            return nullptr;
        }
        if (!isSpilled(num, coloring)) {
            return regTemp(colorFor(num, coloring));
        }

        int scratch = scratchFor(num);
        auto slotIt = spillSlots.find(num);
        if (slotIt != spillSlots.end()) {
            appendStoreSpill(after, scratch, stackOffsetForSlot(slotIt->second));
        }
        return regTemp(scratch);
    }
};

static map<int, int> buildSpillSlots(const Coloring *coloring) {
    map<int, int> slots;
    if (coloring == nullptr) {
        return slots;
    }

    int nextSlot = 0;
    for (int temp : coloring->spilled) {
        if (!isMachineReg(temp)) {
            slots[temp] = nextSlot++;
        }
    }
    return slots;
}

} // namespace

string getRegName(int colorNum) {
    if (colorNum == 11) return "fp";
    if (colorNum == 13) return "sp";
    if (colorNum == 14) return "lr";
    return "r" + to_string(colorNum);
}

string getTempRegName(int tempNum, const Coloring* coloring) {
    if (isMachineReg(tempNum)) {
        return getRegName(tempNum);
    }
    if (coloring == nullptr) {
        return "t" + to_string(tempNum);
    }
    auto it = coloring->colors.find(tempNum);
    if (it == coloring->colors.end()) {
        return "spill" + to_string(tempNum);
    }
    return getRegName(it->second);
}

AsmProg* asmprog2colored(AsmProg* program, const vector<Coloring*>& colorings) {
    AsmProg* colored = new AsmProg();
    if (program == nullptr) {
        return colored;
    }

    for (size_t funcIndex = 0; funcIndex < program->functions.size(); ++funcIndex) {
        const Coloring *coloring = funcIndex < colorings.size() ? colorings[funcIndex] : nullptr;
        map<int, int> spillSlots = buildSpillSlots(coloring);
        int spillBytes = static_cast<int>(spillSlots.size()) * 4;
        int localBytes = alignLocalBytes(4 + spillBytes);
        int frameBytes = 32 + localBytes;

        AsmFunction outFunc(program->functions[funcIndex].name);
        InstrColorer instrColorer(coloring, spillSlots);

        for (const auto &instr : program->functions[funcIndex].instructions) {
            AssemInstr adjusted = replaceFrameSize(instr, localBytes, frameBytes);
            instrColorer.colorInstruction(adjusted, outFunc);
        }

        colored->functions.push_back(outFunc);
    }

    return colored;
}

} // namespace instr
