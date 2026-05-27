#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include "loopheader.hh"
#include "flowinfo.hh"
#include "quad.hh"

using namespace std;
using namespace quad;

LoopHeaderMap* findLoopHeadersWithFlow(QuadFuncDecl* func, ControlFlowInfo* flowInfo) {
    if (func == nullptr || flowInfo == nullptr) {
        return new LoopHeaderMap();  // Return empty map if missing required info
    }

    LoopHeaderMap* loopHeaderMap = new LoopHeaderMap();
    loopHeaderMap->initFunc(func);

    // You may copy your HW9 look header finding code here, but you need to use the flowInfo to find loop 
    // headers and their bodies using the CFG structure in flowInfo

    return loopHeaderMap;
}