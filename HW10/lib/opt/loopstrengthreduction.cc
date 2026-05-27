#include "loopstrengthreduction.hh"
#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <cstdlib>

using namespace std;
using namespace quad;

StrengthReductionPlan generateStrengthReductionPlan(
    QuadFuncDecl* func,
    const map<int, vector<DerivedInductionVar>>& derivedIVsByHeader,
    const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
    LoopHeaderMap* loopHeaderMap
) {
    StrengthReductionPlan plan;
    
    if (func == nullptr || loopHeaderMap == nullptr || derivedIVsByHeader.empty()) {
        return plan;
    }

    // fill in the code to generate a strength reduction plan based on the discovered basic and derived IVs,
    // and the loop structure in loopHeaderMap. The plan should include which derived IVs to replace with new PHI+update,
    // how to compute the new PHI and update values, where to place the new statements, and which original statements to remove. 

    return plan;
}

QuadFuncDecl* applyStrengthReduction(
    QuadFuncDecl* func,
    const StrengthReductionPlan& plan
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }

    if (plan.tempReplacement.empty()) {
        return func;
    }
    // fill in the code to apply the strength reduction plan to func, 
    // by modifying the quads in the func

    return func;
}
