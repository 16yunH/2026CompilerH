#ifndef __LOOPOPTREPORT_HH__
#define __LOOPOPTREPORT_HH__

#include "loopoptanalysis.hh"
#include <iostream>

/**
 * Print analysis results for loop induction variable optimization
 */
class LoopOptReporter {
public:
    /**
     * Print the complete analysis report including:
     * - Basic induction variables
     * - Derived induction variables
     * - Strength reduction optimization plan
     */
    static void printFullReport(const LoopOptAnalysisResult* result);
    
    /**
     * Print only basic induction variables
     */
    static void printBasicInductionVars(const LoopOptAnalysisResult* result);
    
    /**
     * Print only derived induction variables
     */
    static void printDerivedInductionVars(const LoopOptAnalysisResult* result);
    
    /**
     * Print only the strength reduction plan
     */
    static void printStrengthReductionPlan(const LoopOptAnalysisResult* result);
};

#endif
