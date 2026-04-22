#define DEBUG
#undef DEBUG

#include <iostream>
#include <queue>
#include <algorithm>
#include <map>
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;
// Find all variables used or defined in the function
void DataFlowInfo::findAllVars()
{
#ifdef DEBUG
    cout << "Finding all variables in function: " << func->funcname << endl;
#endif
    allVars.clear();
    defs->clear();
    uses->clear();

    if (func == nullptr || func->quadblocklist == nullptr)
    {
        return;
    }

    for (auto *block : *func->quadblocklist)
    {
        if (block == nullptr || block->quadlist == nullptr)
        {
            continue;
        }

        for (auto *stmt : *block->quadlist)
        {
            if (stmt == nullptr)
            {
                continue;
            }

            if (stmt->def != nullptr)
            {
                for (auto *temp : *(stmt->def))
                {
                    if (temp == nullptr)
                    {
                        continue;
                    }
                    int varId = temp->num;
                    allVars.insert(varId);
                    (*defs)[varId].insert(make_pair(block, stmt));
                }
            }

            if (stmt->use != nullptr)
            {
                for (auto *temp : *(stmt->use))
                {
                    if (temp == nullptr)
                    {
                        continue;
                    }
                    int varId = temp->num;
                    allVars.insert(varId);
                    (*uses)[varId].insert(make_pair(block, stmt));
                }
            }
        }
    }
}

// Calculate both live-in and live-out sets for all statements
void DataFlowInfo::computeLiveness()
{
#ifdef DEBUG
    cout << "Computing liveness for function: " << func->funcname << endl;
#endif
    livein->clear();
    liveout->clear();

    if (func == nullptr || func->quadblocklist == nullptr)
    {
        return;
    }

    map<int, QuadBlock *> labelToBlock;
    for (auto *block : *func->quadblocklist)
    {
        if (block != nullptr && block->entry_label != nullptr)
        {
            labelToBlock[block->entry_label->num] = block;
        }
    }

    map<QuadStm *, vector<QuadStm *>> stmtSuccessors;

    for (auto *block : *func->quadblocklist)
    {
        if (block == nullptr || block->quadlist == nullptr || block->quadlist->empty())
        {
            continue;
        }

        for (auto *stmt : *block->quadlist)
        {
            if (stmt == nullptr)
            {
                continue;
            }
            (*livein)[stmt] = set<int>();
            (*liveout)[stmt] = set<int>();
        }

        for (size_t i = 0; i < block->quadlist->size(); ++i)
        {
            QuadStm *stmt = block->quadlist->at(i);
            if (stmt == nullptr)
            {
                continue;
            }

            vector<QuadStm *> &succs = stmtSuccessors[stmt];
            if (i + 1 < block->quadlist->size())
            {
                QuadStm *nextStmt = block->quadlist->at(i + 1);
                if (nextStmt != nullptr)
                {
                    succs.push_back(nextStmt);
                }
                continue;
            }

            if (block->exit_labels == nullptr)
            {
                continue;
            }

            for (auto *exitLabel : *(block->exit_labels))
            {
                if (exitLabel == nullptr)
                {
                    continue;
                }
                auto blockIt = labelToBlock.find(exitLabel->num);
                if (blockIt == labelToBlock.end() || blockIt->second == nullptr ||
                    blockIt->second->quadlist == nullptr || blockIt->second->quadlist->empty())
                {
                    continue;
                }

                QuadStm *firstStmt = blockIt->second->quadlist->at(0);
                if (firstStmt == nullptr)
                {
                    continue;
                }

                if (find(succs.begin(), succs.end(), firstStmt) == succs.end())
                {
                    succs.push_back(firstStmt);
                }
            }
        }
    }

    auto tempsToVarIds = [](set<tree::Temp *> *temps)
    {
        set<int> vars;
        if (temps == nullptr)
        {
            return vars;
        }
        for (auto *temp : *temps)
        {
            if (temp != nullptr)
            {
                vars.insert(temp->num);
            }
        }
        return vars;
    };

    bool changed = true;
    while (changed)
    {
        changed = false;

        for (auto blockIt = func->quadblocklist->rbegin(); blockIt != func->quadblocklist->rend(); ++blockIt)
        {
            QuadBlock *block = *blockIt;
            if (block == nullptr || block->quadlist == nullptr)
            {
                continue;
            }

            for (auto stmtIt = block->quadlist->rbegin(); stmtIt != block->quadlist->rend(); ++stmtIt)
            {
                QuadStm *stmt = *stmtIt;
                if (stmt == nullptr)
                {
                    continue;
                }

                set<int> newOut;
                auto succIt = stmtSuccessors.find(stmt);
                if (succIt != stmtSuccessors.end())
                {
                    for (auto *succStmt : succIt->second)
                    {
                        if (succStmt == nullptr)
                        {
                            continue;
                        }
                        const set<int> &succIn = (*livein)[succStmt];
                        newOut.insert(succIn.begin(), succIn.end());
                    }
                }

                set<int> newIn = tempsToVarIds(stmt->use);
                set<int> stmtDef = tempsToVarIds(stmt->def);

                for (int varId : newOut)
                {
                    if (stmtDef.find(varId) == stmtDef.end())
                    {
                        newIn.insert(varId);
                    }
                }

                if (newIn != (*livein)[stmt] || newOut != (*liveout)[stmt])
                {
                    (*livein)[stmt] = newIn;
                    (*liveout)[stmt] = newOut;
                    changed = true;
                }
            }
        }
    }
}

set<DataFlowInfo *> *dataFLowProg(QuadProgram *prog)
{
    // THIS ONE IS DONE FOR YOU!
    //  For each function in the program, compute its data flow information and
    //  return a set of DataFlowInfo for all functions
    if (!prog || !prog->quadFuncDeclList)
        return nullptr;
    set<DataFlowInfo *> *allDataFlows = new set<DataFlowInfo *>();
    for (auto func : *prog->quadFuncDeclList)
    {
        if (!func || !func->quadblocklist)
            continue;

        DataFlowInfo *dfInfo = new DataFlowInfo(func);
        dfInfo->findAllVars();
        dfInfo->computeLiveness();
#ifdef DEBUG
        cout << "Liveness information for function: " << func->funcname << endl;
        cout << dfInfo->printLiveness() << endl;
#endif
        allDataFlows->insert(dfInfo);
    }
    return allDataFlows;
}
