#ifndef PRESCHEDULE_HH
#define PRESCHEDULE_HH

#include <string>
#include <vector>
#include "quad.hh"
#include "assem.hh"

namespace instr {

struct preScheduleBlock {
    const quad::QuadBlock *quadBlock;
    tree::Label *entryLabel;
    std::vector<const quad::QuadPhi*> phiFunctions;
    AssemInstrList selectedInstructions;
    const quad::QuadStm *lastInstruction;

    explicit preScheduleBlock(const quad::QuadBlock *block = nullptr)
        : quadBlock(block),
          entryLabel(block == nullptr ? nullptr : block->entry_label),
          lastInstruction(nullptr) {}

    void addPhi(const quad::QuadPhi *phi) {
        if (phi != nullptr) {
            phiFunctions.push_back(phi);
        }
    }

    void addSelectedInstruction(const AssemInstr& instr) {
        selectedInstructions.append(instr);
    }
};

struct preScheduleFunc {
    const quad::QuadFuncDecl *quadFunc;
    std::vector<preScheduleBlock*> blockSchedules;

    explicit preScheduleFunc(const quad::QuadFuncDecl *func = nullptr)
        : quadFunc(func) {}

    void addBlock(preScheduleBlock *block) {
        if (block != nullptr) {
            blockSchedules.push_back(block);
        }
    }
};

struct preScheduleProg {
    const quad::QuadProgram *quadProgram;
    std::vector<preScheduleFunc*> funcSchedules;

    explicit preScheduleProg(const quad::QuadProgram *program = nullptr)
        : quadProgram(program) {}

    void addFunc(preScheduleFunc *func) {
        if (func != nullptr) {
            funcSchedules.push_back(func);
        }
    }

    void printToStdout() const;
};

preScheduleProg *buildPreScheduleProg(const quad::QuadProgram *program);

} // namespace instr

#endif // PRESCHEDULE_HH
