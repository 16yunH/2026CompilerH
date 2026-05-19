#ifndef __LOOPOPT_HH__
#define __LOOPOPT_HH__

#include <iostream>
#include <string>
#include <variant>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

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

    // Get the loop body blocks for a given function and loop header
    set<int> getLoopBodyBlocks(QuadFuncDecl* func, int headerLabel) {
        if (funcLoopHeaders.count(func)) {
            for (LoopHeader* loopHeader : funcLoopHeaders[func]) {
                if (loopHeader->headerLabel == headerLabel) {
                    return loopHeader->bodyBlocks;
                }
            }
        }
        return {};
    }
};

LoopHeaderMap* findLoopHeaders(QuadFuncDecl* func, FuncFlowInfo *ffi);

QuadFuncDecl* loopHoistFunc(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap);

#endif