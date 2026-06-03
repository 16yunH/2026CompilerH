#ifndef INSTR_ASMPROGPASS_HH
#define INSTR_ASMPROGPASS_HH

#include <vector>
#include "asmprog.hh"
#include "coloring.hh"

namespace instr {

// Pass: append r0-r3 (caller-saved clobbers) to the dst of every bl/blx instruction.
// Must be run before dataflow analysis and interference-graph construction.
void preDataFlowPass(AsmProg* prog);

// Pass: apply register colors to assembly program and handle spilled temporaries.
// Uses r9-r10 as scratch registers for spill/reload operations.
AsmProg* asmprog2colored(AsmProg* program, const std::vector<Coloring*>& colorings);

// Helper: convert color number to ARM register name (r0..r15)
std::string getRegName(int colorNum);

// Helper: get register name for a temp given a coloring
std::string getTempRegName(int tempNum, const Coloring* coloring);

} // namespace instr

#endif // INSTR_ASMPROGPASS_HH
