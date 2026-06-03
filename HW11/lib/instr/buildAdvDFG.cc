#include "advDFG.hh"

#include <unordered_map>
#include <vector>

namespace instr {

advDFGprog *buildAdvDFGprog(const quad::QuadProgram *program) {
    auto *out = new advDFGprog(program);
    if (program == nullptr || program->quadFuncDeclList == nullptr) {
        return out;
    }

    /// fill in the code ...

    return out;

}

} // namespace instr
