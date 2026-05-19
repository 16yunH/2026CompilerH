#include <stack>
#include <map>
#include <set>
#include "loopopt.hh"
#include "flowinfo.hh"
#include "quad.hh"

// Function to find loop headers in a function and populate the LoopHeaderMap
// Complete the function!!

static void collectBlocks(QuadFuncDecl *func,
                          set<int> &allBlocks,
                          map<int, QuadBlock *> &labelToBlock) {
    allBlocks.clear();
    labelToBlock.clear();
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }

    for (QuadBlock *block : *func->quadblocklist) {
        if (block != nullptr && block->entry_label != nullptr) {
            int label = block->entry_label->num;
            allBlocks.insert(label);
            labelToBlock[label] = block;
        }
    }
}

static map<int, set<int>> buildSuccessors(const set<int> &allBlocks,
                                          const map<int, QuadBlock *> &labelToBlock) {
    map<int, set<int>> successors;
    for (int blockLabel : allBlocks) {
        successors[blockLabel] = {};
        auto blockIt = labelToBlock.find(blockLabel);
        if (blockIt == labelToBlock.end() || blockIt->second == nullptr ||
            blockIt->second->exit_labels == nullptr) {
            continue;
        }

        for (Label *exitLabel : *blockIt->second->exit_labels) {
            if (exitLabel != nullptr && allBlocks.count(exitLabel->num)) {
                successors[blockLabel].insert(exitLabel->num);
            }
        }
    }
    return successors;
}

static map<int, set<int>> buildPredecessors(const set<int> &allBlocks,
                                            const map<int, set<int>> &successors) {
    map<int, set<int>> predecessors;
    for (int blockLabel : allBlocks) {
        predecessors[blockLabel] = {};
    }

    for (const auto &entry : successors) {
        int pred = entry.first;
        for (int succ : entry.second) {
            predecessors[succ].insert(pred);
        }
    }
    return predecessors;
}

static map<int, set<int>> computeDominators(const set<int> &allBlocks,
                                            const map<int, set<int>> &predecessors,
                                            int entryBlock) {
    map<int, set<int>> dominators;
    if (allBlocks.empty()) {
        return dominators;
    }

    if (!allBlocks.count(entryBlock)) {
        entryBlock = *allBlocks.begin();
    }

    for (int blockLabel : allBlocks) {
        if (blockLabel == entryBlock) {
            dominators[blockLabel] = {blockLabel};
        } else {
            dominators[blockLabel] = allBlocks;
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (int blockLabel : allBlocks) {
            if (blockLabel == entryBlock) {
                continue;
            }

            set<int> newDominators;
            bool firstPred = true;
            auto predIt = predecessors.find(blockLabel);
            if (predIt != predecessors.end()) {
                for (int pred : predIt->second) {
                    if (!dominators.count(pred)) {
                        continue;
                    }
                    if (firstPred) {
                        newDominators = dominators[pred];
                        firstPred = false;
                    } else {
                        set<int> intersection;
                        for (int dom : newDominators) {
                            if (dominators[pred].count(dom)) {
                                intersection.insert(dom);
                            }
                        }
                        newDominators = intersection;
                    }
                }
            }

            if (firstPred) {
                newDominators.clear();
            }
            newDominators.insert(blockLabel);

            if (dominators[blockLabel] != newDominators) {
                dominators[blockLabel] = newDominators;
                changed = true;
            }
        }
    }

    return dominators;
}

LoopHeaderMap *findLoopHeaders(QuadFuncDecl* func, FuncFlowInfo *ffi) {
    LoopHeaderMap *loopHeaderMap = new LoopHeaderMap();
    loopHeaderMap->initFunc(func);
    if (func == nullptr) {
        return loopHeaderMap;
    }

    ControlFlowInfo *cfi = ffi != nullptr ? ffi->cfi : nullptr;

    set<int> allBlocks;
    map<int, QuadBlock *> labelToBlock;
    collectBlocks(func, allBlocks, labelToBlock);

    map<int, set<int>> successors;
    if (cfi != nullptr && !cfi->successors.empty()) {
        successors = cfi->successors;
    } else {
        successors = buildSuccessors(allBlocks, labelToBlock);
    }

    map<int, set<int>> predecessors;
    if (cfi != nullptr && !cfi->predecessors.empty()) {
        predecessors = cfi->predecessors;
    } else {
        predecessors = buildPredecessors(allBlocks, successors);
    }

    map<int, set<int>> dominators;
    if (cfi != nullptr && !cfi->dominators.empty()) {
        dominators = cfi->dominators;
    } else {
        int entryBlock = cfi != nullptr ? cfi->entryBlock : -1;
        if (entryBlock == -1 && func->quadblocklist != nullptr && !func->quadblocklist->empty() &&
            func->quadblocklist->front() != nullptr && func->quadblocklist->front()->entry_label != nullptr) {
            entryBlock = func->quadblocklist->front()->entry_label->num;
        }
        dominators = computeDominators(allBlocks, predecessors, entryBlock);
    }

    map<int, set<int>> headerToBody;
    for (const auto &entry : successors) {
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
                int block = worklist.top();
                worklist.pop();

                auto predIt = predecessors.find(block);
                if (predIt == predecessors.end()) {
                    continue;
                }

                for (int pred : predIt->second) {
                    if (!body.count(pred)) {
                        body.insert(pred);
                        worklist.push(pred);
                    }
                }
            }

            headerToBody[header].insert(body.begin(), body.end());
        }
    }

    set<LoopHeader *> loopHeaders;
    for (const auto &entry : headerToBody) {
        loopHeaders.insert(new LoopHeader(entry.first, entry.second));
    }
    loopHeaderMap->addLoopHeader(func, loopHeaders);

    return loopHeaderMap;
}
