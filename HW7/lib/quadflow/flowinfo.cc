#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

// Print liveness information for debugging
string DataFlowInfo::printLiveness() {
#ifdef DEBUG
    cout << "Printing liveness for function: " << func->funcname << endl;
#endif
    if (!func || !liveout || !livein) return "";
    
    string output_str; output_str.reserve(10000);
    output_str += "Liveness information for function: " + func->funcname + "\n";
    
    for (auto block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        
        output_str += "Block ";
        output_str += to_string(block->entry_label->num);
        output_str += ":\n";    
        for (QuadStm* qs: *block->quadlist) {
            if (!qs) continue;
#ifdef DEBUG
            cout << "Printing statement: " << quadKindToString(qs->kind) << endl;
            cout << "livein size=" << (livein->find(qs) != livein->end() ? (*livein)[qs].size() : 0) << endl;
            cout << "liveout size=" << (liveout->find(qs) != liveout->end() ? (*liveout)[qs].size() : 0) << endl;
#endif
            bool first= true;
            output_str += "Statement ";
            output_str += quadKindToString(qs->kind);
            //output_str += ":\n";
            output_str += "  Live-in:  {";
            for (int var : (*livein)[qs]) {
                if (first) {
                    first = false;
                } else {
                    output_str += ", ";
                }
                output_str += "t";
                output_str += to_string(var);
            }
            first=true;
            //output_str += "}\n Live-out: {";
            output_str += "} Live-out: {";
            for (int var : (*liveout)[qs]) {
                if (first) {
                    first = false;
                } else {
                    output_str += ", ";
                }
                output_str += "t";
                output_str += to_string(var);

            }
            output_str +=  "}\n";
        }
    }
    cout << output_str;
    return output_str;
}

static void pMap(map<int, set<int>>& map2set, bool rightarrow) {
    for (auto& pair : map2set) {
        cout << "Block: " << pair.first << (rightarrow?" -> ":" <- ");
        for (auto dom : pair.second) {
            cout << dom << " ";
        }
        cout << endl;
    }
}

void ControlFlowInfo::printPredecessors() {
    cout << "Predecessors:" << endl;
    pMap(predecessors, false); //leftarrow
} 

void ControlFlowInfo::printSuccessors() {
    cout << "Successors:" << endl;
    pMap(successors, true); //rightarrow
} 

void ControlFlowInfo::printDominators() {
    cout << "Dominators:" << endl;
    pMap(dominators, false); //leftarrow (been dominated)
}

void ControlFlowInfo::printImmediateDominators() {
    cout << "Immediate Dominators:" << endl;
    for (auto& pair : immediateDominator) {
        cout << "Block: " << pair.first << " -> " << pair.second << endl;
    }
}

void ControlFlowInfo::printDominanceFrontier() {
    cout << "Dominance Frontier:" << endl;
    pMap(dominanceFrontiers, true); //rightarrow
}

void ControlFlowInfo::printDomTree() {
    cout << "Dominator Tree:" << endl;
    pMap(domTree, true); //rightarrow
} 

void ControlFlowInfo::printEverything() {
    cout << "Control Flow Information for function: " << func->funcname << endl;
    printPredecessors();
    printSuccessors();
    printDominators();
    printImmediateDominators();
    printDominanceFrontier();
    printDomTree();
}

void ControlFlowInfo::computeEverything() {
    // Compute all control flow information
    computeAllBlocks();
    computeUnreachableBlocks();
    eliminateUnreachableBlocks();

    computePredecessors(); 
#ifdef DEBUG
    printPredecessors();
#endif
    computeSuccessors(); 
#ifdef DEBUG
    printSuccessors();
#endif
    computeDominators(); 
#ifdef DEBUG
    printDominators();
#endif
    computeImmediateDominator();
#ifdef DEBUG
    printImmediateDominators();
#endif
    computeDomTree();
#ifdef DEBUG
    printDomTree();
#endif
    computeDominanceFrontiers();
#ifdef DEBUG
    printDominanceFrontier();
#endif
}
