#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include "loopheader.hh"
#include "flowinfo.hh"
#include "quad.hh"

using namespace std;
using namespace quad;

LoopHeaderMap* findLoopHeadersWithFlow(QuadFuncDecl* func, ControlFlowInfo* flowInfo) {
    if (func == nullptr || flowInfo == nullptr) {
        return new LoopHeaderMap();  // Return empty map if missing required info
    }

    LoopHeaderMap* loopHeaderMap = new LoopHeaderMap();
    loopHeaderMap->initFunc(func);

    set<int> allBlocks;
    for (QuadBlock* block : *func->quadblocklist) {
        if (block != nullptr && block->entry_label != nullptr) {
            allBlocks.insert(block->entry_label->num);
        }
    }

    map<int, set<int>> successors = flowInfo->successors;
    if (successors.empty()) {
        for (QuadBlock* block : *func->quadblocklist) {
            if (block == nullptr || block->entry_label == nullptr || block->exit_labels == nullptr) {
                continue;
            }
            int label = block->entry_label->num;
            successors[label] = {};
            for (Label* exitLabel : *block->exit_labels) {
                if (exitLabel != nullptr && allBlocks.count(exitLabel->num)) {
                    successors[label].insert(exitLabel->num);
                }
            }
        }
    }

    map<int, set<int>> predecessors = flowInfo->predecessors;
    if (predecessors.empty()) {
        for (int label : allBlocks) {
            predecessors[label] = {};
        }
        for (const auto& entry : successors) {
            int pred = entry.first;
            for (int succ : entry.second) {
                predecessors[succ].insert(pred);
            }
        }
    }

    map<int, set<int>> dominators = flowInfo->dominators;
    if (dominators.empty() && !allBlocks.empty()) {
        int entryBlock = flowInfo->entryBlock;
        if (!allBlocks.count(entryBlock)) {
            entryBlock = *allBlocks.begin();
        }

        for (int label : allBlocks) {
            dominators[label] = (label == entryBlock) ? set<int>{label} : allBlocks;
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (int label : allBlocks) {
                if (label == entryBlock) {
                    continue;
                }

                set<int> next;
                bool firstPred = true;
                for (int pred : predecessors[label]) {
                    if (!dominators.count(pred)) {
                        continue;
                    }
                    if (firstPred) {
                        next = dominators[pred];
                        firstPred = false;
                    } else {
                        set<int> intersection;
                        for (int dom : next) {
                            if (dominators[pred].count(dom)) {
                                intersection.insert(dom);
                            }
                        }
                        next = intersection;
                    }
                }

                if (firstPred) {
                    next.clear();
                }
                next.insert(label);
                if (dominators[label] != next) {
                    dominators[label] = next;
                    changed = true;
                }
            }
        }
    }

    map<int, set<int>> headerToBody;
    for (const auto& entry : successors) {
        int tail = entry.first;
        for (int header : entry.second) {
            if (!dominators.count(tail) || !dominators[tail].count(header)) {
                continue;
            }

            set<int> body;
            body.insert(header);
            body.insert(tail);

            stack<int> worklist;
            if (tail != header) {
                worklist.push(tail);
            }

            while (!worklist.empty()) {
                int label = worklist.top();
                worklist.pop();

                if (!predecessors.count(label)) {
                    continue;
                }

                for (int pred : predecessors[label]) {
                    if (!body.count(pred)) {
                        body.insert(pred);
                        worklist.push(pred);
                    }
                }
            }

            headerToBody[header].insert(body.begin(), body.end());
        }
    }

    set<LoopHeader*> loopHeaders;
    for (const auto& entry : headerToBody) {
        loopHeaders.insert(new LoopHeader(entry.first, entry.second));
    }
    loopHeaderMap->addLoopHeader(func, loopHeaders);

    return loopHeaderMap;
}
