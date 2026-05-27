#include "loopoptreport.hh"

using namespace std;

void LoopOptReporter::printFullReport(const LoopOptAnalysisResult* result)
{
    if (result == nullptr) {
        return;
    }
    printBasicInductionVars(result);
    printDerivedInductionVars(result);
    printStrengthReductionPlan(result);
}

void LoopOptReporter::printBasicInductionVars(const LoopOptAnalysisResult* result)
{
    if (result == nullptr || result->basicByHeader.empty()) {
        cout << "=== NO BASIC INDUCTION VARIABLES FOUND ===" << endl << endl;
        return;
    }
    
    cout << "=== BASIC INDUCTION VARIABLES ===" << endl;
    for (const auto& entry : result->basicByHeader) {
        cout << "Loop at header L" << entry.first << ":" << endl;
        for (const auto& biv : entry.second) {
            biv.print();
        }
    }
    cout << endl;
}

void LoopOptReporter::printDerivedInductionVars(const LoopOptAnalysisResult* result)
{
    if (result == nullptr || result->derivedByHeader.empty()) {
        cout << "=== NO DERIVED INDUCTION VARIABLES FOUND ===" << endl << endl;
        return;
    }
    
    cout << "=== DERIVED INDUCTION VARIABLES ===" << endl;
    for (const auto& entry : result->derivedByHeader) {
        cout << "Loop at header L" << entry.first << ":" << endl;
        for (const auto& div : entry.second) {
            div.print();
        }
    }
    cout << endl;
}

void LoopOptReporter::printStrengthReductionPlan(const LoopOptAnalysisResult* result)
{
    if (result == nullptr) {
        return;
    }
    result->srPlan.print();
}
