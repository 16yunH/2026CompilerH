#ifndef __ASMDATAFLOW_HH
#define __ASMDATAFLOW_HH

#include <map>
#include <set>
#include <vector>
#include "asmprog.hh"

namespace instr {

// DataFlowInfo for AsmProg: computes liveness information
class AsmDataFlowInfo {
public:
    AsmFunction* func;
    std::vector<AssemInstr>* instrs; // instructions in the function
    
    // Liveness sets for each instruction
    std::map<size_t, std::set<int>> livein;   // livein[i] = temps live at entry of instruction i
    std::map<size_t, std::set<int>> liveout;  // liveout[i] = temps live at exit of instruction i

    AsmDataFlowInfo(AsmFunction* f);
    ~AsmDataFlowInfo() = default;

    // Compute liveness information
    void computeLiveness();

    // Helper functions
    void printLiveness() const;
    std::set<int> getUse(size_t instrIdx) const;
    std::set<int> getDef(size_t instrIdx) const;
};

} // namespace instr

#endif // __ASMDATAFLOW_HH
