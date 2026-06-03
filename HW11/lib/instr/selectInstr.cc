#include "instrSelection.hh"

#include <cstdint>
#include <set>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace instr {

// Main instruction selection for a block
void selectInstructionsForBlock(
    const advDFGblock &blockGraph,
    preScheduleBlock &schedBlock,
    int &nextTempNum
) {
    const auto& graph = blockGraph.graph;
    const auto& nodes = graph.getNodes();
    if (nodes.empty()) {
        return;
    }

    //fill in necessary code 

    return;
}

void runInstructionSelectionPass(
    const advDFGprog &graphProgram,
    preScheduleProg &preScheduleProgram
) {
    size_t funcCount = std::min(graphProgram.fungraph.size(), preScheduleProgram.funcSchedules.size());
    for (size_t funcIndex = 0; funcIndex < funcCount; ++funcIndex) {
        auto *funcGraph = graphProgram.fungraph[funcIndex];
        auto *funcSchedule = preScheduleProgram.funcSchedules[funcIndex];
        if (funcGraph == nullptr || funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) {
            continue;
        }

        int nextTempNum = funcSchedule->quadFunc->last_temp_num + 1;
        size_t blockCount = std::min(funcGraph->blockgraph.size(), funcSchedule->blockSchedules.size());
        for (size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            auto *blockGraph = funcGraph->blockgraph[blockIndex];
            auto *blockSchedule = funcSchedule->blockSchedules[blockIndex];
            if (blockGraph == nullptr || blockSchedule == nullptr) {
                continue;
            }

            selectInstructionsForBlock(*blockGraph, *blockSchedule, nextTempNum);
        }

        auto *mutableFunc = const_cast<quad::QuadFuncDecl*>(funcSchedule->quadFunc);
        mutableFunc->last_temp_num = nextTempNum - 1;
    }
}

} // namespace instr
