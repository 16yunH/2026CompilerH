#include "loopinductionbasic.hh"
#include "defusechain.hh"
#include "loopinductionopt.hh"

using namespace std;
using namespace quad;

map<int, vector<BasicInductionVar>> discoverBasicInductionVars(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, 
        const DefUseChain& du, const ControlFlowInfo& cfi) {
    map<int, vector<BasicInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }
    // fill in the code to discover basic induction variables, 
    // and populate the result map with header label -> list of basic IVs in that loop
    // Note to fill in all the necessary information in the BasicInductionVar objects, 
    // including the related temps and their usefulness

    return result;
}

// Classify related temps as useless (only in backedges) or useful (in computations)
void classifyRelatedTemps(
    QuadFuncDecl* func,
    map<int, vector<BasicInductionVar>>& basicByHeader,
    const DefUseChain& du
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }
    // fill in the code to classify related temps of basic induction variables 
    // as useless (only in backedges) or useful (in computations),
    // by analyzing the def-use chains of the related temps, and 
    // checking if they are used in any statements other than the backedge updates of the IV family
    return ;
}