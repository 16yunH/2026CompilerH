#include "loopinductionderived.hh"
#include "defusechain.hh"
#include "loopinductionopt.hh"

using namespace std;
using namespace quad;

map<int, vector<DerivedInductionVar>> discoverDerivedInductionVars(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, 
        const DefUseChain& du, const ControlFlowInfo& cfi) {
    map<int, vector<DerivedInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }
    // fill in the code to discover derived induction variables,
    // and populate the result map with header label -> list of derived IVs in that loop
    // Note to fill in all the necessary information in the DerivedInductionVar objects,
    // including the related temps and their usefulness, and the expression for how the derived IV is computed

    return result;
}
