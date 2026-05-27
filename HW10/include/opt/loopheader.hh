#ifndef __LOOPHEADER_HH__
#define __LOOPHEADER_HH__

#include <iostream>
#include <string>
#include <variant>
#include "temp.hh"
#include "quad.hh"

// Forward declare ControlFlowInfo to avoid circular includes
class ControlFlowInfo;

using namespace std;
using namespace tree;
using namespace quad;

class LoopHeader {
    public:
        int headerLabel; // The label number of the loop header block
        set<int> bodyBlocks; // The label numbers of the blocks that are part of the loop body
        LoopHeader(int headerLabel, const set<int>& bodyBlocks) : headerLabel(headerLabel), bodyBlocks(bodyBlocks) {}
};

class LoopHeaderMap {
public:
    // Map from function to vector loop headers and their corresponding loop body blocks
    map<QuadFuncDecl*, set<LoopHeader*>> funcLoopHeaders;
    
    //Initialize the loop header map for a given function
    void initFunc(QuadFuncDecl* func) {
        if (!funcLoopHeaders.count(func)) {
            funcLoopHeaders[func] = {};
        }
    }

    // Add a loop header and its body blocks for a given function
    void addLoopHeader(QuadFuncDecl* func, set<LoopHeader*> loopHeaders) {
        initFunc(func);
        for (LoopHeader* loopHeader : loopHeaders) {
            funcLoopHeaders[func].insert(loopHeader);
        }
    }


};

LoopHeaderMap* findLoopHeadersWithFlow(QuadFuncDecl* func, ControlFlowInfo* flowInfo);

#endif