#ifndef INSTR_ASMPROG_HH
#define INSTR_ASMPROG_HH

#include <vector>
#include <string>
#include "assem.hh"

namespace instr {


struct AsmFunction {
    std::string name;
    std::vector<AssemInstr> instructions;
    AsmFunction(const std::string& n) : name(n) {}

    // Print function as string
    std::string to_string() const;
};

struct AsmProg {
    std::vector<AsmFunction> functions;
    AsmProg() = default;

    // Print program as string
    std::string to_string() const;
};

} // namespace instr

#endif // INSTR_ASMPROG_HH
