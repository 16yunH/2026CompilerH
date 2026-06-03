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

    for (auto &func : program->functions) {
        instr::AsmDataFlowInfo flowInfo(&func);
        flowInfo.computeLiveness();
        graphs.push_back(buildIg(&func, &flowInfo));
    }

    return graphs;
}

namespace {

static void addNode(map<int, set<int>> &graph, int node) {
    if (graph.find(node) == graph.end()) {
        graph[node] = set<int>();
    }
}

static void addEdge(map<int, set<int>> &graph, int u, int v) {
    if (u == v) {
        addNode(graph, u);
        return;
    }
    addNode(graph, u);
    addNode(graph, v);
    graph[u].insert(v);
    graph[v].insert(u);
}

static int tempNum(tree::Temp *temp) {
    return temp == nullptr ? -1 : temp->num;
}

static pair<int, int> orderedMovePair(int left, int right) {
    if (left > right) {
        swap(left, right);
    }
    return {left, right};
}

static bool isSimpleMove(const instr::AssemInstr &instr, int &dst, int &src) {
    if (instr.kind != instr::AssemInstr::I_MOVE || instr.dst.size() != 1 || instr.src.size() != 1) {
        return false;
    }
    dst = tempNum(instr.dst.front());
    src = tempNum(instr.src.front());
    return dst >= 0 && src >= 0;
}

} // namespace

InterferenceGraph *buildIg(instr::AsmFunction* asmFunc, instr::AsmDataFlowInfo* flowInfo) {
    map<int, set<int>> graph;
    set<pair<int, int>> movePairs;

    if (asmFunc == nullptr || flowInfo == nullptr) {
        return new InterferenceGraph(graph, movePairs);
    }

    for (int reg = 0; reg <= 14; ++reg) {
        addNode(graph, reg);
    }
    for (int left = 0; left <= 14; ++left) {
        for (int right = left + 1; right <= 14; ++right) {
            addEdge(graph, left, right);
        }
    }

    for (size_t index = 0; index < asmFunc->instructions.size(); ++index) {
        const auto &instr = asmFunc->instructions[index];
        set<int> use = flowInfo->getUse(index);
        set<int> def = flowInfo->getDef(index);
        set<int> liveout = flowInfo->liveout[index];

        for (int temp : use) {
            addNode(graph, temp);
        }
        for (int temp : def) {
            addNode(graph, temp);
        }
        for (int temp : liveout) {
            addNode(graph, temp);
        }

        for (auto left = use.begin(); left != use.end(); ++left) {
            auto right = left;
            ++right;
            for (; right != use.end(); ++right) {
                addEdge(graph, *left, *right);
            }
        }

        int moveDst = -1;
        int moveSrc = -1;
        bool move = isSimpleMove(instr, moveDst, moveSrc);
        if (move && moveDst != moveSrc) {
            movePairs.insert(orderedMovePair(moveDst, moveSrc));
        }

        for (int d : def) {
            for (int live : liveout) {
                if (move && live == moveSrc) {
                    continue;
                }
                addEdge(graph, d, live);
            }
        }
    }

    return new InterferenceGraph(graph, movePairs);
}
