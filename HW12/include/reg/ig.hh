#ifndef __INTERFERENCEGRAPH_HH
#define __INTERFERENCEGRAPH_HH

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>

using namespace std;

class InterferenceGraph {
public:
    map<int, set<int>> graph; // Adjacency list representation of the interference graph
    set<pair<int, int>> movePairs; // Set of move pairs in the original graph

    //constructing the graph from the original graph and the move pairs
    InterferenceGraph(map<int, set<int>> graph, set<pair<int, int>> movePairs) : graph(graph), movePairs(movePairs) { 
    }; // Constructor

    string printGraph(); // Print the current graph with other info (move pairs, etc.) to string
    void printGraph(string filename); // Print to file
    void printGraph(ofstream &io); // Print stream

    //helper functions
    static bool isMachineReg(int n) {return n < 100;}; // Check if a node is a machine register
};

// Forward declarations
namespace instr {
    struct AsmFunction;
    class AsmDataFlowInfo;
    struct AsmProg;
}

InterferenceGraph *buildIg(instr::AsmFunction* asmFunc, instr::AsmDataFlowInfo* flowInfo); // build the interference graph for AsmFunction
std::vector<InterferenceGraph*> buildIgProg(instr::AsmProg* program); // build interference graphs for all functions in AsmProg

#endif // __INTERFERENCEGRAPH_HH