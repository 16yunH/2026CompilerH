#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

void ControlFlowInfo::computeAllBlocks() {
    // This one is done for you!
#ifdef DEBUG
    cout << "Computing all blocks in function: " << func->funcname << endl;
    cout << "#blocks = " << func->quadblocklist->size() << endl;
#endif
    // Compute all blocks in the function
    if (func == nullptr || func->quadblocklist == nullptr) {
        return ; // Nothing to do
    }
    //Collect block information
    allBlocks = set<int>(); //empty set
    labelToBlock = map<int, QuadBlock*>(); //empty map

    for (auto block : *func->quadblocklist) {
        if (block->entry_label) {
            allBlocks.insert(block->entry_label->num);
            labelToBlock[block->entry_label->num] = block;
        }
    }
#ifdef DEBUG
    cout << "All blocks in function: " << func->funcname << endl;
    for (auto block : allBlocks) {
        cout << block << " " << labelToBlock[block]->entry_label->str() << endl;
    }
    cout << endl;
#endif
}

void ControlFlowInfo::computeUnreachableBlocks() {
#ifdef DEBUG
    cout << "Computing unreachable blocks in function: " << func->funcname << endl;
#endif
    // FILE IN THE CODE HERE TO COMPUTE UNREACHABLE BLOCKS
}

void ControlFlowInfo::eliminateUnreachableBlocks() {
#ifdef DEBUG
    cout << "Eliminating unreachable blocks in function: " << func->funcname << endl;
#endif
    // FILE IN THE CODE HERE TO ELIMINATE UNREACHABLE BLOCKS from the function,
    // and emptying all the sets and maps above since everything may need to be recomputed!
}

void ControlFlowInfo::computePredecessors() {
    // Compute predecessors for each block
    // FILE IN THE CODE HERE TO COMPUTE PREDECESSORS for each block
}

void ControlFlowInfo::computeSuccessors() {
    // Compute successors for each block
    // FILE IN THE CODE HERE TO COMPUTE SUCCESSORS for each block
}

void ControlFlowInfo::computeDominators() {
#ifdef DEBUG
    std::cout << "Computing dominators for: " << func->funcname << endl;
#endif
    // Compute dominators for each block
    // FILE IN THE CODE HERE TO COMPUTE DOMINATORS for each block
}

void ControlFlowInfo::computeImmediateDominator() {
#ifdef DEBUG
    std::cout << "Start to find immediate dominators for: " << func->funcname << endl;
#endif
    //FILE IN THE CODE HERE TO COMPUTE IMMEDIATE DOMINATOR for each block, using the dominators information computed above
}

void ControlFlowInfo::computeDomTree() {
    #ifdef DEBUG
        std::cout << "Computing dominator tree for: " << func->funcname << endl;
    #endif
    // FILE IN THE CODE HERE TO COMPUTE DOMINATOR TREE using immediate dominators
}

void ControlFlowInfo::computeDominanceFrontiers() {
#ifdef DEBUG
    std::cout << "Computing dominance frontier for: " << func->funcname << endl;
#endif

    //FILE IN THE CODE HERE TO COMPUTE DOMINANCE FRONTIER for each block, using the successors, dominators, immediate dominator, and domTree information computed above

}
