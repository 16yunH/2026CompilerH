#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "ig.hh"
#include "coloring.hh"

namespace {

static void eraseMovesWith(set<pair<int, int>> &moves, int node) {
    for (auto it = moves.begin(); it != moves.end();) {
        if (it->first == node || it->second == node) {
            it = moves.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace

//return true if any node is removed
bool Coloring::simplify() {
#ifdef DEBUG
    cout << "Simplifying..." << endl;
#endif
    bool changed = false;
    
    while (true) {
        int selected = -1;
        for (const auto &entry : graph) {
            int node = entry.first;
            if (!isMachineReg(node) && !isMove(node) && static_cast<int>(entry.second.size()) < k) {
                selected = node;
                break;
            }
        }
        if (selected < 0) {
            break;
        }

        simplifiedNodes.push(selected);
        eraseNode(selected);
        eraseMovesWith(movePairs, selected);
        changed = true;
    }

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
    return false;
}

//freeze the moves that are not coalesced
//return true if changed anything, false otherwise
bool Coloring::freeze() {
#ifdef DEBUG
    cout << "Freezing..." << endl;
#endif
    bool changed = false;

    int selected = -1;
    for (const auto &entry : graph) {
        int node = entry.first;
        if (!isMachineReg(node) && isMove(node) && static_cast<int>(entry.second.size()) < k) {
            selected = node;
            break;
        }
    }
    if (selected >= 0) {
        eraseMovesWith(movePairs, selected);
        changed = true;
    }

    return changed;
}

//This is a soft spill: we just remove the node from the graph and add it to the simplified nodes
//as if nothing happened. The actual spill happens when select&coloring
bool Coloring::spill() {
#ifdef DEBUG
    cout << "Spilling..." << endl;
#endif

    bool changed = false;
    
    int selected = -1;
    int bestDegree = -1;
    for (const auto &entry : graph) {
        int node = entry.first;
        if (isMachineReg(node)) {
            continue;
        }
        int degree = static_cast<int>(entry.second.size());
        if (degree > bestDegree) {
            bestDegree = degree;
            selected = node;
        }
    }

    if (selected >= 0) {
        simplifiedNodes.push(selected);
        eraseNode(selected);
        eraseMovesWith(movePairs, selected);
        changed = true;
    }

    return changed;
}

//now try to select the registers for the nodes
bool Coloring::select() {
#ifdef DEBUG
    cout << "Selecting..." << endl;
#endif

    colors.clear();
    spilled.clear();

    for (const auto &entry : ig->graph) {
        if (isMachineReg(entry.first)) {
            colors[entry.first] = entry.first;
        }
    }

    while (!simplifiedNodes.empty()) {
        int node = simplifiedNodes.top();
        simplifiedNodes.pop();
        if (isMachineReg(node)) {
            colors[node] = node;
            continue;
        }

        set<int> unavailable;
        auto igNode = ig->graph.find(node);
        if (igNode != ig->graph.end()) {
            for (int neighbor : igNode->second) {
                auto colorIt = colors.find(neighbor);
                if (colorIt != colors.end() && spilled.find(neighbor) == spilled.end()) {
                    unavailable.insert(colorIt->second);
                }
            }
        }

        int chosen = -1;
        for (int color = 0; color < k; ++color) {
            if (unavailable.find(color) == unavailable.end()) {
                chosen = color;
                break;
            }
        }

        if (chosen >= 0) {
            colors[node] = chosen;
        } else {
            spilled.insert(node);
        }
    }

    for (const auto &entry : coalescedMoves) {
        int representative = entry.first;
        for (int node : entry.second) {
            if (spilled.find(representative) != spilled.end()) {
                spilled.insert(node);
            } else if (colors.find(representative) != colors.end()) {
                colors[node] = colors[representative];
            } else if (!isMachineReg(node)) {
                spilled.insert(node);
            }
        }
    }

    bool all_covered = true;
    for (const auto &entry : ig->graph) {
        int node = entry.first;
        if (colors.find(node) == colors.end() && spilled.find(node) == spilled.end()) {
            all_covered = false;
            if (isMachineReg(node)) {
                colors[node] = node;
            } else {
                spilled.insert(node);
            }
        }
    }

    return all_covered; //return true if all nodes are colored, false otherwise
}
