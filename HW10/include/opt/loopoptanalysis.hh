#ifndef __LOOPOPTANALYSIS_HH__
#define __LOOPOPTANALYSIS_HH__

#include "quad.hh"
#include "flowinfo.hh"
#include "loopheader.hh"
#include "loopinductionbasic.hh"
#include "loopinductionderived.hh"
#include "loopstrengthreduction.hh"
#include <map>
#include <vector>

/**
 * Analysis results from loop induction variable optimization
 */
struct LoopOptAnalysisResult {
    LoopHeaderMap* loopHeaderMap;
    std::map<int, std::vector<BasicInductionVar>> basicByHeader;
    std::map<int, std::vector<DerivedInductionVar>> derivedByHeader;
    StrengthReductionPlan srPlan;
    
    LoopOptAnalysisResult() 
        : loopHeaderMap(nullptr) {}
    
    ~LoopOptAnalysisResult() {
        if (loopHeaderMap != nullptr) {
            delete loopHeaderMap;
        }
    }
};

/**
 * Perform comprehensive loop induction variable analysis
 * 
 * @param func The function to analyze
 * @param flowInfo Optional control flow information for improved accuracy
 * @return Analysis results containing all discovered IVs and optimization plan
 */
LoopOptAnalysisResult* analyzeLoopInductionVars(
    quad::QuadFuncDecl* func,
    ControlFlowInfo* flowInfo = nullptr
);

#endif
