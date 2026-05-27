#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "quad.hh"
#include "loopinductionopt.hh"
#include "loopoptanalysis.hh"
#include "loopoptreport.hh"
#include "xml2quad.hh"
#include "xml2flow.hh"

using namespace std;
using namespace quad;

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " filename" << endl;
        return EXIT_FAILURE;
    }

    string baseFile = argv[1];
    string outputFile = baseFile + ".4-ssa-loopivopt.quad";

    // Read flow information from XML
    string file_quad_ssa_withflow_xml = baseFile + ".4-ssa-withflow-xml.quad";
    cout << "Reading Quad (SSA) from xml: " << file_quad_ssa_withflow_xml << endl;
    
    set<FuncFlowInfo*>* flowProgSet = xml2flow(file_quad_ssa_withflow_xml.c_str());
    if (flowProgSet == nullptr || flowProgSet->empty()) {
        cerr << "Error reading Quad with flow information from xml" << endl;
        return EXIT_FAILURE;
    }
    cout << "Successfully read SSA with flow information" << endl;

    // Track program-level metadata from first function
    int programLastLabelNum = 0;
    int programLastTempNum = 0;
    
    // Optimize each function individually
    auto optimizedFuncs = new vector<QuadFuncDecl*>();
    
    for (FuncFlowInfo* ffi : *flowProgSet) {
        if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) {
            continue;
        }
        
        QuadFuncDecl* func = ffi->cfi->func;
        ControlFlowInfo* flowInfo = ffi->cfi;
        
        cout << "\n=== Processing function: " << func->funcname << " ===" << endl;
        
        // Create single-function program for this function
        auto funcList = new vector<QuadFuncDecl*>();
        funcList->push_back(func);
        QuadProgram* funcProg = new QuadProgram(funcList, ffi->programLastLabelNum, ffi->programLastTempNum);
        
        // Analyze and print report
        LoopOptAnalysisResult* analysis = analyzeLoopInductionVars(func, flowInfo);
        LoopOptReporter::printFullReport(analysis);
        delete analysis;
        
        // Apply optimization passes to this function
        QuadProgram* stage1 = loopInductionStrengthReductionPass(funcProg, flowInfo);
        QuadProgram* stage2 = loopInductionCleanupPass(stage1);
        
        // Extract optimized function and pack into output program
        if (stage2 && stage2->quadFuncDeclList && !stage2->quadFuncDeclList->empty()) {
            QuadFuncDecl* optimizedFunc = stage2->quadFuncDeclList->at(0);
            optimizedFuncs->push_back(optimizedFunc);
            
            // Track metadata from each function (use last seen)
            programLastLabelNum = ffi->programLastLabelNum;
            programLastTempNum = ffi->programLastTempNum;
            
            cout << "Optimized function " << optimizedFunc->funcname << endl;
        }
    }
    
    // Create output program with all optimized functions
    QuadProgram* outputProg = new QuadProgram(optimizedFuncs, programLastLabelNum, programLastTempNum);

    // Write optimized output
    if (outputProg == nullptr) {
        cerr << "Error: null program to write" << endl;
        return EXIT_FAILURE;
    }
    
    string out;
    outputProg->print(out, 0, false);
    
    ofstream ofs(outputFile);
    if (!ofs) {
        cerr << "Error opening output file: " << outputFile << endl;
        return EXIT_FAILURE;
    }
    
    ofs << out;
    ofs.close();
    cout << "Writing optimized Quad to file: " << outputFile << endl;
    
    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}