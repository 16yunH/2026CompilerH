#include <stack>
#include <map>
#include <set>
#include "loopopt.hh"
#include "flowinfo.hh"
#include "quad.hh"

// Function to find loop headers in a function and populate the LoopHeaderMap
// Complete the function!!

LoopHeaderMap *findLoopHeaders(QuadFuncDecl* func, FuncFlowInfo *ffi) {
    LoopHeaderMap *loopHeaderMap = new LoopHeaderMap();
    ControlFlowInfo *cfi = ffi->cfi;

    // Fill in the loop header map for the function

    return loopHeaderMap;
}