#include "loopoptanalysis.hh"
#include "defusechain.hh"
#include <iostream>

using namespace std;
using namespace quad;

LoopOptAnalysisResult* analyzeLoopInductionVars(
    QuadFuncDecl* func,
    ControlFlowInfo* flowInfo)
{
    auto result = new LoopOptAnalysisResult();
    
    if (func == nullptr) {
        return result;
    }
    
    // Discover loops using flow information (required)
    if (flowInfo == nullptr) {
        cerr << "Error: flowInfo is required for loop analysis" << endl;
        return result;
    }
    result->loopHeaderMap = findLoopHeadersWithFlow(func, flowInfo);
    
    if (result->loopHeaderMap == nullptr) {
        return result;
    }
    
    // Build def-use chain for the function
    DefUseChain du(func);
    
    // Discover basic induction variables
    result->basicByHeader = discoverBasicInductionVars(func, result->loopHeaderMap, du, *flowInfo);
    
    // Classify related temps as useless (cyclic only) or useful (in computations)
    if (!result->basicByHeader.empty()) {
        classifyRelatedTemps(func, result->basicByHeader, du);
    }
    
    // Discover derived induction variables
    result->derivedByHeader = discoverDerivedInductionVars(func, result->loopHeaderMap, du, *flowInfo);
    
    // Generate strength reduction plan
    result->srPlan = generateStrengthReductionPlan(
        func,
        result->derivedByHeader,
        result->basicByHeader,
        result->loopHeaderMap
    );
    
    return result;
}
