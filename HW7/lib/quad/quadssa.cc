#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <utility>
#include "quad.hh"
#include "flowinfo.hh"
#include "quadssa.hh"
#include "temp.hh"

using namespace std;
using namespace quad;

// Forward declarations for internal functions
static void placePhi(QuadFuncDecl* func, ControlFlowInfo* domInfo, DataFlowInfo* liveness);
static void renameVariables(QuadFuncDecl* func, ControlFlowInfo* domInfo);
static void cleanupUnusedPhi(QuadFuncDecl* func);

// Place Phi functions at appropriate locations
// HW7: You need to write this part!!
static void placePhi(QuadFuncDecl* func, ControlFlowInfo* domInfo, DataFlowInfo* liveness) {
#ifdef DEBUG
    cout << "Placing phi functions for function: " << func->funcname << endl;
#endif
    // Place phi functions at join points where variables defined in different paths merge
}

// Rename variables to ensure SSA property
// HW7: You need to write this part!!
static void renameVariables(QuadFuncDecl* func, ControlFlowInfo* domInfo) {
#ifdef DEBUG
    cout << "Entering renaming variables for function: " << func->funcname << endl;
#endif
    // Rename variables to ensure that each variable is assigned exactly once
}

// Remove unnecessary phi functions
// HW7: You need to write this part!!
static void cleanupUnusedPhi(QuadFuncDecl* func) {
#ifdef DEBUG
    cout << "Cleaning up unused phi functions for function: " << func->funcname << endl;
#endif
    // Find phi functions that are not used
}

// Convert blocked Quad with precomputed flow info to SSA form
quad::QuadProgram *quad2ssa(set<FuncFlowInfo*>* allFuncFlow) {
    if (!allFuncFlow || allFuncFlow->empty()) {
        return nullptr; // Invalid program
    }

    vector<QuadFuncDecl*>* funcs = new vector<QuadFuncDecl*>();
    funcs->reserve(allFuncFlow->size());
    int prog_last_label_num = -1;
    int prog_last_temp_num = -1;

    for (auto* ffi : *allFuncFlow) {
        if (!ffi || !ffi->cfi || !ffi->cfi->func) {
            continue;
        }
        QuadFuncDecl* funcdecl = ffi->cfi->func;
        int maxLastLabelNum = funcdecl->last_label_num;
        int maxLastTempNum = funcdecl->last_temp_num;

        ControlFlowInfo* domInfo = ffi->cfi;
        DataFlowInfo* liveness = ffi->dfi;

        // Now Place Phi functions at join points
        placePhi(funcdecl, domInfo, liveness);
        // Rename variables to ensure SSA property
        renameVariables(funcdecl, domInfo);
        // Clean up unnecessary phi nodes
        cleanupUnusedPhi(funcdecl);

        funcs->push_back(funcdecl);
        if (prog_last_label_num < funcdecl->last_label_num) {
            prog_last_label_num = funcdecl->last_label_num;
        }
        if (prog_last_temp_num < funcdecl->last_temp_num) {
            prog_last_temp_num = funcdecl->last_temp_num;
        }
    }
    return new QuadProgram(funcs, prog_last_label_num, prog_last_temp_num);
}