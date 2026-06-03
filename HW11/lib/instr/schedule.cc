#include "schedule.hh"

#include "advDFG.hh"
#include "instrSelection.hh"

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace instr {

ScheduleProg *scheduleProg(preScheduleProg *preScheduleProgram) {
    if (preScheduleProgram == nullptr) {
        return nullptr;
    }

    auto *out = new ScheduleProg(preScheduleProgram->quadProgram);

    // fill in the code ... 

    return out;
}

} // namespace instr
