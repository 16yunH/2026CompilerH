#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "ig.hh"
#include "coloring.hh"

//return true if any node is removed
bool Coloring::simplify() {
#ifdef DEBUG
    cout << "Simplifying..." << endl;
#endif
    bool changed = false;
    
    // fill in the code.

#ifdef DEBUG
    cout << "Simplifying done. Changed=" << changed << endl;
#endif
    return changed;
}

//return true if changed anything, false otherwise
bool Coloring::coalesce() {
#ifdef DEBUG
    cout << "Coalescing..." << endl;
#endif
    if (ig == nullptr) return false;
    bool changed = false;
    
    // fill in the code.

    return changed;
}

//freeze the moves that are not coalesced
//return true if changed anything, false otherwise
bool Coloring::freeze() {
#ifdef DEBUG
    cout << "Freezing..." << endl;
#endif
    bool changed = false;

    // fill in the code.

    return changed;
}

//This is a soft spill: we just remove the node from the graph and add it to the simplified nodes
//as if nothing happened. The actual spill happens when select&coloring
bool Coloring::spill() {
#ifdef DEBUG
    cout << "Spilling..." << endl;
#endif

    bool changed = false;
    
    //fill in the code.

    return changed;
}

//now try to select the registers for the nodes
bool Coloring::select() {
#ifdef DEBUG
    cout << "Selecting..." << endl;
#endif

    bool all_covered = false;

    // fill in the code.

    return all_covered; //return true if all nodes are colored, false otherwise
}
