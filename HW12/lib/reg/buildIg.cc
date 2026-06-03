// #define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "temp.hh"
#include "ig.hh"
#include "asmdataflow.hh"
#include "asmprog.hh"

using namespace std;

// Build interference graphs for all functions in AsmProg

vector<InterferenceGraph*> buildIgProg(instr::AsmProg* program) {
    vector<InterferenceGraph*> graphs;
    
    if (program == nullptr || program->functions.empty()) {
        return graphs;
    }

#ifdef DEBUG
    cout << "Building interference graphs for program with " << program->functions.size() << " functions" << endl;
#endif

    //fill the code. Build Ig for each function.

    return graphs;
}