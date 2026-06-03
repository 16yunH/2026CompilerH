#ifndef INSTR_SCHEDULE_HH
#define INSTR_SCHEDULE_HH

#include <string>
#include <vector>
#include "preSchedule.hh"
#include "assem.hh"

namespace instr {

struct ScheduleFunc {
    const quad::QuadFuncDecl *quadFunc;
    AssemInstrList linearizedInstructions;

    explicit ScheduleFunc(const quad::QuadFuncDecl *func = nullptr)
        : quadFunc(func) {}

    void addLinearizedInstruction(const AssemInstr &instr) {
        linearizedInstructions.append(instr);
    }
};

struct ScheduleProg {
    const quad::QuadProgram *quadProgram;
    std::vector<ScheduleFunc*> funcSchedules;

    explicit ScheduleProg(const quad::QuadProgram *program = nullptr)
        : quadProgram(program) {}

    void addFunc(ScheduleFunc *func) {
        if (func != nullptr) {
            funcSchedules.push_back(func);
        }
    }

    void writeToFile(const std::string &outputBaseName) const;
};

ScheduleProg *scheduleProg(preScheduleProg *preScheduleProgram);

} // namespace instr

#endif // INSTR_SCHEDULE_HH
