#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <functional>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

void ControlFlowInfo::computeAllBlocks()
{
    // This one is done for you!
#ifdef DEBUG
    cout << "Computing all blocks in function: " << func->funcname << endl;
    cout << "#blocks = " << func->quadblocklist->size() << endl;
#endif
    // Compute all blocks in the function
    if (func == nullptr || func->quadblocklist == nullptr)
    {
        return; // Nothing to do
    }
    // Collect block information
    allBlocks = set<int>();                 // empty set
    labelToBlock = map<int, QuadBlock *>(); // empty map

    for (auto block : *func->quadblocklist)
    {
        if (block->entry_label)
        {
            allBlocks.insert(block->entry_label->num);
            labelToBlock[block->entry_label->num] = block;
        }
    }
#ifdef DEBUG
    cout << "All blocks in function: " << func->funcname << endl;
    for (auto block : allBlocks)
    {
        cout << block << " " << labelToBlock[block]->entry_label->str() << endl;
    }
    cout << endl;
#endif
}

void ControlFlowInfo::computeUnreachableBlocks()
{
#ifdef DEBUG
    cout << "Computing unreachable blocks in function: " << func->funcname << endl;
#endif
    unreachableBlocks.clear();
    if (func == nullptr || func->quadblocklist == nullptr || entryBlock == -1)
    {
        return;
    }

    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }
    if (allBlocks.empty())
    {
        return;
    }
    if (allBlocks.find(entryBlock) == allBlocks.end())
    {
        unreachableBlocks = allBlocks;
        return;
    }

    set<int> reachable;
    queue<int> worklist;
    reachable.insert(entryBlock);
    worklist.push(entryBlock);

    while (!worklist.empty())
    {
        int current = worklist.front();
        worklist.pop();

        auto blockIt = labelToBlock.find(current);
        if (blockIt == labelToBlock.end() || blockIt->second == nullptr || blockIt->second->exit_labels == nullptr)
        {
            continue;
        }

        for (auto *nextLabel : *(blockIt->second->exit_labels))
        {
            if (nextLabel == nullptr)
            {
                continue;
            }
            int nextId = nextLabel->num;
            if (allBlocks.find(nextId) == allBlocks.end())
            {
                continue;
            }
            if (reachable.insert(nextId).second)
            {
                worklist.push(nextId);
            }
        }
    }

    for (int blockId : allBlocks)
    {
        if (reachable.find(blockId) == reachable.end())
        {
            unreachableBlocks.insert(blockId);
        }
    }
}

void ControlFlowInfo::eliminateUnreachableBlocks()
{
#ifdef DEBUG
    cout << "Eliminating unreachable blocks in function: " << func->funcname << endl;
#endif
    if (func != nullptr && func->quadblocklist != nullptr && !unreachableBlocks.empty())
    {
        auto *blocks = func->quadblocklist;
        blocks->erase(
            remove_if(
                blocks->begin(),
                blocks->end(),
                [this](QuadBlock *block)
                {
                    if (block == nullptr || block->entry_label == nullptr)
                    {
                        return true;
                    }
                    return unreachableBlocks.find(block->entry_label->num) != unreachableBlocks.end();
                }),
            blocks->end());

        if (!blocks->empty() && blocks->at(0) != nullptr && blocks->at(0)->entry_label != nullptr)
        {
            entryBlock = blocks->at(0)->entry_label->num;
        }
        else
        {
            entryBlock = -1;
        }
    }

    // CFG may change after elimination; clear and recompute from scratch.
    allBlocks.clear();
    unreachableBlocks.clear();
    predecessors.clear();
    successors.clear();
    dominators.clear();
    immediateDominator.clear();
    dominanceFrontiers.clear();
    domTree.clear();
    labelToBlock.clear();
}

void ControlFlowInfo::computePredecessors()
{
    // Compute predecessors for each block
    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }

    predecessors.clear();
    for (int blockId : allBlocks)
    {
        predecessors[blockId] = set<int>();
    }

    for (int blockId : allBlocks)
    {
        auto blockIt = labelToBlock.find(blockId);
        if (blockIt == labelToBlock.end() || blockIt->second == nullptr || blockIt->second->exit_labels == nullptr)
        {
            continue;
        }
        for (auto *succLabel : *(blockIt->second->exit_labels))
        {
            if (succLabel == nullptr)
            {
                continue;
            }
            int succId = succLabel->num;
            if (allBlocks.find(succId) == allBlocks.end())
            {
                continue;
            }
            predecessors[succId].insert(blockId);
        }
    }
}

void ControlFlowInfo::computeSuccessors()
{
    // Compute successors for each block
    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }

    successors.clear();
    for (int blockId : allBlocks)
    {
        successors[blockId] = set<int>();
    }

    for (int blockId : allBlocks)
    {
        auto blockIt = labelToBlock.find(blockId);
        if (blockIt == labelToBlock.end() || blockIt->second == nullptr || blockIt->second->exit_labels == nullptr)
        {
            continue;
        }
        for (auto *succLabel : *(blockIt->second->exit_labels))
        {
            if (succLabel == nullptr)
            {
                continue;
            }
            int succId = succLabel->num;
            if (allBlocks.find(succId) == allBlocks.end())
            {
                continue;
            }
            successors[blockId].insert(succId);
        }
    }
}

void ControlFlowInfo::computeDominators()
{
#ifdef DEBUG
    std::cout << "Computing dominators for: " << func->funcname << endl;
#endif
    // Compute dominators for each block
    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }
    if (predecessors.empty())
    {
        computePredecessors();
    }
    if (allBlocks.empty())
    {
        return;
    }

    if (entryBlock == -1 || allBlocks.find(entryBlock) == allBlocks.end())
    {
        entryBlock = *allBlocks.begin();
    }

    dominators.clear();
    for (int blockId : allBlocks)
    {
        if (blockId == entryBlock)
        {
            dominators[blockId] = {blockId};
        }
        else
        {
            dominators[blockId] = allBlocks;
        }
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int blockId : allBlocks)
        {
            if (blockId == entryBlock)
            {
                continue;
            }

            set<int> newDom;
            bool hasPred = false;
            for (int pred : predecessors[blockId])
            {
                if (!hasPred)
                {
                    newDom = dominators[pred];
                    hasPred = true;
                }
                else
                {
                    set<int> inter;
                    set_intersection(
                        newDom.begin(), newDom.end(),
                        dominators[pred].begin(), dominators[pred].end(),
                        inserter(inter, inter.begin()));
                    newDom.swap(inter);
                }
            }

            if (!hasPred)
            {
                newDom.clear();
            }
            newDom.insert(blockId);

            if (newDom != dominators[blockId])
            {
                dominators[blockId] = newDom;
                changed = true;
            }
        }
    }
}

void ControlFlowInfo::computeImmediateDominator()
{
#ifdef DEBUG
    std::cout << "Start to find immediate dominators for: " << func->funcname << endl;
#endif
    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }
    if (dominators.empty())
    {
        computeDominators();
    }

    immediateDominator.clear();
    for (int blockId : allBlocks)
    {
        immediateDominator[blockId] = -1;
    }

    for (int blockId : allBlocks)
    {
        if (blockId == entryBlock)
        {
            immediateDominator[blockId] = -1;
            continue;
        }

        set<int> strictDom = dominators[blockId];
        strictDom.erase(blockId);

        int idom = -1;
        for (int candidate : strictDom)
        {
            bool dominatedByOtherStrictDom = false;
            for (int other : strictDom)
            {
                if (other == candidate)
                {
                    continue;
                }
                auto it = dominators.find(other);
                if (it != dominators.end() && it->second.find(candidate) != it->second.end())
                {
                    dominatedByOtherStrictDom = true;
                    break;
                }
            }
            if (!dominatedByOtherStrictDom)
            {
                idom = candidate;
                break;
            }
        }

        immediateDominator[blockId] = idom;
    }
}

void ControlFlowInfo::computeDomTree()
{
#ifdef DEBUG
    std::cout << "Computing dominator tree for: " << func->funcname << endl;
#endif
    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }
    if (immediateDominator.empty())
    {
        computeImmediateDominator();
    }

    domTree.clear();
    for (int blockId : allBlocks)
    {
        domTree[blockId] = set<int>();
    }

    for (auto &pair : immediateDominator)
    {
        int blockId = pair.first;
        int idom = pair.second;
        if (idom != -1 && allBlocks.find(idom) != allBlocks.end() && idom != blockId)
        {
            domTree[idom].insert(blockId);
        }
    }
}

void ControlFlowInfo::computeDominanceFrontiers()
{
#ifdef DEBUG
    std::cout << "Computing dominance frontier for: " << func->funcname << endl;
#endif

    if (allBlocks.empty() || labelToBlock.empty())
    {
        computeAllBlocks();
    }
    if (successors.empty())
    {
        computeSuccessors();
    }
    if (immediateDominator.empty())
    {
        computeImmediateDominator();
    }
    if (domTree.empty())
    {
        computeDomTree();
    }

    dominanceFrontiers.clear();
    for (int blockId : allBlocks)
    {
        dominanceFrontiers[blockId] = set<int>();
    }

    if (entryBlock == -1 || allBlocks.find(entryBlock) == allBlocks.end())
    {
        return;
    }

    function<void(int)> postOrderCompute = [&](int blockId)
    {
        for (int child : domTree[blockId])
        {
            postOrderCompute(child);
        }

        set<int> df;

        for (int succ : successors[blockId])
        {
            auto it = immediateDominator.find(succ);
            if (it == immediateDominator.end() || it->second != blockId)
            {
                df.insert(succ);
            }
        }

        for (int child : domTree[blockId])
        {
            for (int frontierNode : dominanceFrontiers[child])
            {
                auto it = immediateDominator.find(frontierNode);
                if (it == immediateDominator.end() || it->second != blockId)
                {
                    df.insert(frontierNode);
                }
            }
        }

        dominanceFrontiers[blockId] = df;
    };

    postOrderCompute(entryBlock);
}
