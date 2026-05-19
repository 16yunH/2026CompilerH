#define DEBUG
#undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <map>
#include "quad.hh"
#include "flowinfo.hh"
#include "loopopt.hh"

using namespace std;
using namespace quad;

// Main entry point for loop optimization
// Complete the function!!

QuadFuncDecl* loopHoistFunc(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap) {
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return func;
    }

    // Fill in the loop hoisting logic here

    return func;
}