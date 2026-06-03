#ifndef INSTR_SELECTION_HH
#define INSTR_SELECTION_HH

#include "advDFG.hh"
#include "preSchedule.hh"

namespace instr {

void selectInstructionsForBlock(
	const advDFGblock &blockGraph,
	preScheduleBlock &schedBlock,
	int &nextTempNum
);

void runInstructionSelectionPass(
	const advDFGprog &graphProgram,
	preScheduleProg &preScheduleProgram
);

} // namespace instr

#endif // INSTR_SELECTION_HH
