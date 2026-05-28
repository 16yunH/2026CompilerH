#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <utility>
#include <functional>
#include "quad.hh"
#include "flowinfo.hh"
#include "quadssa.hh"
#include "temp.hh"
#include "quadssa_diag.hh"

using namespace std;
using namespace quad;

static void addTempFromTerm(QuadTerm *term, set<Temp *> *out)
{
    if (!term || !out)
    {
        return;
    }
    QuadTemp *qt = term->get_temp();
    if (qt && qt->temp)
    {
        out->insert(qt->temp);
    }
}

static void addTempNumFromTerm(QuadTerm *term, set<int> &out)
{
    if (!term || term->kind != QuadTermKind::TEMP)
    {
        return;
    }
    QuadTemp *qt = term->get_temp();
    if (qt && qt->temp)
    {
        out.insert(qt->temp->num);
    }
}

static void recordTempInfo(QuadTemp *qt, map<int, QuadType> &tempTypes, map<int, Temp *> &tempPool)
{
    if (!qt || !qt->temp)
    {
        return;
    }
    int num = qt->temp->num;
    if (!tempPool.count(num))
    {
        tempPool[num] = qt->temp;
    }
    if (!tempTypes.count(num))
    {
        tempTypes[num] = qt->type;
    }
}

static void recordTermInfo(QuadTerm *term, map<int, QuadType> &tempTypes, map<int, Temp *> &tempPool)
{
    if (!term || term->kind != QuadTermKind::TEMP)
    {
        return;
    }
    QuadTemp *qt = term->get_temp();
    recordTempInfo(qt, tempTypes, tempPool);
}

static void collectTempInfo(QuadFuncDecl *func,
                            map<int, QuadType> &tempTypes,
                            map<int, Temp *> &tempPool,
                            set<int> &defTemps,
                            map<int, set<int>> &defBlocks)
{
    if (!func || !func->quadblocklist)
    {
        return;
    }

    for (auto *block : *func->quadblocklist)
    {
        if (!block || !block->quadlist || !block->entry_label)
        {
            continue;
        }
        int blockId = block->entry_label->num;
        for (auto *stm : *block->quadlist)
        {
            if (!stm)
            {
                continue;
            }

            if (stm->def)
            {
                for (auto *t : *stm->def)
                {
                    if (t)
                    {
                        defTemps.insert(t->num);
                        defBlocks[t->num].insert(blockId);
                        if (!tempPool.count(t->num))
                        {
                            tempPool[t->num] = t;
                        }
                    }
                }
            }

            switch (stm->kind)
            {
            case QuadKind::MOVE:
            {
                auto *q = static_cast<QuadMove *>(stm);
                recordTempInfo(q->dst, tempTypes, tempPool);
                recordTermInfo(q->src, tempTypes, tempPool);
                break;
            }
            case QuadKind::LOAD:
            {
                auto *q = static_cast<QuadLoad *>(stm);
                recordTempInfo(q->dst, tempTypes, tempPool);
                recordTermInfo(q->src, tempTypes, tempPool);
                break;
            }
            case QuadKind::STORE:
            {
                auto *q = static_cast<QuadStore *>(stm);
                recordTermInfo(q->src, tempTypes, tempPool);
                recordTermInfo(q->dst, tempTypes, tempPool);
                break;
            }
            case QuadKind::MOVE_BINOP:
            {
                auto *q = static_cast<QuadMoveBinop *>(stm);
                recordTempInfo(q->dst, tempTypes, tempPool);
                recordTermInfo(q->left, tempTypes, tempPool);
                recordTermInfo(q->right, tempTypes, tempPool);
                break;
            }
            case QuadKind::CALL:
            {
                auto *q = static_cast<QuadCall *>(stm);
                recordTermInfo(q->obj_term, tempTypes, tempPool);
                if (q->args)
                {
                    for (auto *arg : *q->args)
                    {
                        recordTermInfo(arg, tempTypes, tempPool);
                    }
                }
                break;
            }
            case QuadKind::MOVE_CALL:
            {
                auto *q = static_cast<QuadMoveCall *>(stm);
                recordTempInfo(q->dst, tempTypes, tempPool);
                if (q->call)
                {
                    recordTermInfo(q->call->obj_term, tempTypes, tempPool);
                    if (q->call->args)
                    {
                        for (auto *arg : *q->call->args)
                        {
                            recordTermInfo(arg, tempTypes, tempPool);
                        }
                    }
                }
                break;
            }
            case QuadKind::EXTCALL:
            {
                auto *q = static_cast<QuadExtCall *>(stm);
                if (q->args)
                {
                    for (auto *arg : *q->args)
                    {
                        recordTermInfo(arg, tempTypes, tempPool);
                    }
                }
                break;
            }
            case QuadKind::MOVE_EXTCALL:
            {
                auto *q = static_cast<QuadMoveExtCall *>(stm);
                recordTempInfo(q->dst, tempTypes, tempPool);
                if (q->extcall && q->extcall->args)
                {
                    for (auto *arg : *q->extcall->args)
                    {
                        recordTermInfo(arg, tempTypes, tempPool);
                    }
                }
                break;
            }
            case QuadKind::CJUMP:
            {
                auto *q = static_cast<QuadCJump *>(stm);
                recordTermInfo(q->left, tempTypes, tempPool);
                recordTermInfo(q->right, tempTypes, tempPool);
                break;
            }
            case QuadKind::PHI:
            {
                auto *q = static_cast<QuadPhi *>(stm);
                recordTempInfo(q->temp_exp, tempTypes, tempPool);
                if (q->args)
                {
                    for (auto &arg : *q->args)
                    {
                        if (arg.first)
                        {
                            if (!tempPool.count(arg.first->num))
                            {
                                tempPool[arg.first->num] = arg.first;
                            }
                        }
                    }
                }
                break;
            }
            case QuadKind::RETURN:
            {
                auto *q = static_cast<QuadReturn *>(stm);
                recordTermInfo(q->exp, tempTypes, tempPool);
                break;
            }
            case QuadKind::PTR_CALC:
            {
                auto *q = static_cast<QuadPtrCalc *>(stm);
                recordTermInfo(q->dst, tempTypes, tempPool);
                recordTermInfo(q->ptr, tempTypes, tempPool);
                recordTermInfo(q->offset, tempTypes, tempPool);
                break;
            }
            case QuadKind::LABEL:
            case QuadKind::JUMP:
            default:
                break;
            }
        }
    }
}

static Temp *getOrCreateTemp(map<int, Temp *> &tempPool, int tempNum)
{
    auto it = tempPool.find(tempNum);
    if (it != tempPool.end())
    {
        return it->second;
    }
    Temp *temp = new Temp(tempNum);
    tempPool[tempNum] = temp;
    return temp;
}

static QuadType getTempType(const map<int, QuadType> &tempTypes, int tempNum)
{
    auto it = tempTypes.find(tempNum);
    if (it != tempTypes.end())
    {
        return it->second;
    }
    return QuadType::INT;
}

static bool getBlockLiveIn(DataFlowInfo *liveness, QuadBlock *block, set<int> &out)
{
    if (!liveness || !liveness->livein || !block || !block->quadlist)
    {
        return false;
    }
    for (auto *stm : *block->quadlist)
    {
        auto it = liveness->livein->find(stm);
        if (it != liveness->livein->end())
        {
            out = it->second;
            return true;
        }
    }
    return false;
}

static bool getDefTempNum(QuadStm *stm, int &out)
{
    if (!stm)
    {
        return false;
    }
    switch (stm->kind)
    {
    case QuadKind::MOVE:
    {
        auto *q = static_cast<QuadMove *>(stm);
        if (q->dst && q->dst->temp)
        {
            out = q->dst->temp->num;
            return true;
        }
        break;
    }
    case QuadKind::LOAD:
    {
        auto *q = static_cast<QuadLoad *>(stm);
        if (q->dst && q->dst->temp)
        {
            out = q->dst->temp->num;
            return true;
        }
        break;
    }
    case QuadKind::MOVE_BINOP:
    {
        auto *q = static_cast<QuadMoveBinop *>(stm);
        if (q->dst && q->dst->temp)
        {
            out = q->dst->temp->num;
            return true;
        }
        break;
    }
    case QuadKind::MOVE_CALL:
    {
        auto *q = static_cast<QuadMoveCall *>(stm);
        if (q->dst && q->dst->temp)
        {
            out = q->dst->temp->num;
            return true;
        }
        break;
    }
    case QuadKind::MOVE_EXTCALL:
    {
        auto *q = static_cast<QuadMoveExtCall *>(stm);
        if (q->dst && q->dst->temp)
        {
            out = q->dst->temp->num;
            return true;
        }
        break;
    }
    case QuadKind::PHI:
    {
        auto *q = static_cast<QuadPhi *>(stm);
        if (q->temp_exp && q->temp_exp->temp)
        {
            out = q->temp_exp->temp->num;
            return true;
        }
        break;
    }
    case QuadKind::PTR_CALC:
    {
        auto *q = static_cast<QuadPtrCalc *>(stm);
        if (q->dst && q->dst->kind == QuadTermKind::TEMP)
        {
            QuadTemp *dt = q->dst->get_temp();
            if (dt && dt->temp)
            {
                out = dt->temp->num;
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

static void resetDefUse(QuadStm *stm, set<Temp *> *def, set<Temp *> *use)
{
    if (!stm)
    {
        return;
    }
    if (stm->def)
    {
        delete stm->def;
    }
    if (stm->use)
    {
        delete stm->use;
    }
    stm->def = def;
    stm->use = use;
}

static void rebuildDefUseForStmt(QuadStm *stm)
{
    if (!stm)
    {
        return;
    }
    set<Temp *> *def = new set<Temp *>();
    set<Temp *> *use = new set<Temp *>();

    switch (stm->kind)
    {
    case QuadKind::MOVE:
    {
        auto *q = static_cast<QuadMove *>(stm);
        if (q->dst && q->dst->temp)
        {
            def->insert(q->dst->temp);
        }
        addTempFromTerm(q->src, use);
        break;
    }
    case QuadKind::LOAD:
    {
        auto *q = static_cast<QuadLoad *>(stm);
        if (q->dst && q->dst->temp)
        {
            def->insert(q->dst->temp);
        }
        addTempFromTerm(q->src, use);
        break;
    }
    case QuadKind::STORE:
    {
        auto *q = static_cast<QuadStore *>(stm);
        addTempFromTerm(q->src, use);
        addTempFromTerm(q->dst, use);
        break;
    }
    case QuadKind::MOVE_BINOP:
    {
        auto *q = static_cast<QuadMoveBinop *>(stm);
        if (q->dst && q->dst->temp)
        {
            def->insert(q->dst->temp);
        }
        addTempFromTerm(q->left, use);
        addTempFromTerm(q->right, use);
        break;
    }
    case QuadKind::CALL:
    {
        auto *q = static_cast<QuadCall *>(stm);
        addTempFromTerm(q->obj_term, use);
        if (q->args)
        {
            for (auto *arg : *q->args)
            {
                addTempFromTerm(arg, use);
            }
        }
        break;
    }
    case QuadKind::MOVE_CALL:
    {
        auto *q = static_cast<QuadMoveCall *>(stm);
        if (q->dst && q->dst->temp)
        {
            def->insert(q->dst->temp);
        }
        if (q->call)
        {
            addTempFromTerm(q->call->obj_term, use);
            if (q->call->args)
            {
                for (auto *arg : *q->call->args)
                {
                    addTempFromTerm(arg, use);
                }
            }
        }
        break;
    }
    case QuadKind::EXTCALL:
    {
        auto *q = static_cast<QuadExtCall *>(stm);
        if (q->args)
        {
            for (auto *arg : *q->args)
            {
                addTempFromTerm(arg, use);
            }
        }
        break;
    }
    case QuadKind::MOVE_EXTCALL:
    {
        auto *q = static_cast<QuadMoveExtCall *>(stm);
        if (q->dst && q->dst->temp)
        {
            def->insert(q->dst->temp);
        }
        if (q->extcall && q->extcall->args)
        {
            for (auto *arg : *q->extcall->args)
            {
                addTempFromTerm(arg, use);
            }
        }
        break;
    }
    case QuadKind::CJUMP:
    {
        auto *q = static_cast<QuadCJump *>(stm);
        addTempFromTerm(q->left, use);
        addTempFromTerm(q->right, use);
        break;
    }
    case QuadKind::PHI:
    {
        auto *q = static_cast<QuadPhi *>(stm);
        if (q->temp_exp && q->temp_exp->temp)
        {
            def->insert(q->temp_exp->temp);
        }
        if (q->args)
        {
            for (auto &arg : *q->args)
            {
                if (arg.first)
                {
                    use->insert(arg.first);
                }
            }
        }
        break;
    }
    case QuadKind::RETURN:
    {
        auto *q = static_cast<QuadReturn *>(stm);
        addTempFromTerm(q->exp, use);
        break;
    }
    case QuadKind::PTR_CALC:
    {
        auto *q = static_cast<QuadPtrCalc *>(stm);
        addTempFromTerm(q->dst, def);
        addTempFromTerm(q->ptr, use);
        addTempFromTerm(q->offset, use);
        break;
    }
    case QuadKind::LABEL:
    case QuadKind::JUMP:
    default:
        break;
    }

    resetDefUse(stm, def, use);
}

static void rebuildDefUseForFunc(QuadFuncDecl *func)
{
    if (!func || !func->quadblocklist)
    {
        return;
    }
    for (auto *block : *func->quadblocklist)
    {
        if (!block || !block->quadlist)
        {
            continue;
        }
        for (auto *stm : *block->quadlist)
        {
            rebuildDefUseForStmt(stm);
        }
    }
}

// Forward declarations for internal functions
static void placePhi(QuadFuncDecl *func, ControlFlowInfo *domInfo, DataFlowInfo *liveness, SsaDiagState &diag);
static void renameVariables(QuadFuncDecl *func, ControlFlowInfo *domInfo, SsaDiagState &diag);
static void cleanupUnusedPhi(QuadFuncDecl *func, SsaDiagState &diag);

// Place Phi functions at appropriate locations
// HW7: You need to write this part!!
static void placePhi(QuadFuncDecl *func, ControlFlowInfo *domInfo, DataFlowInfo *liveness, SsaDiagState &diag)
{
#ifdef DEBUG
    cout << "Placing phi functions for function: " << func->funcname << endl;
#endif
    if (!func || !func->quadblocklist || !domInfo)
    {
        return;
    }

    map<int, QuadType> tempTypes;
    map<int, Temp *> tempPool;
    set<int> defTemps;
    map<int, set<int>> defBlocks;
    collectTempInfo(func, tempTypes, tempPool, defTemps, defBlocks);

    map<int, set<int>> blockLiveIn;
    map<int, bool> hasLiveInfo;
    for (auto *block : *func->quadblocklist)
    {
        if (!block || !block->entry_label)
        {
            continue;
        }
        set<int> livein;
        bool ok = getBlockLiveIn(liveness, block, livein);
        hasLiveInfo[block->entry_label->num] = ok;
        if (ok)
        {
            blockLiveIn[block->entry_label->num] = livein;
        }
    }

    map<int, set<int>> phiPlaced;
    vector<int> vars(defTemps.begin(), defTemps.end());
    sort(vars.begin(), vars.end());

    for (int var : vars)
    {
        queue<int> work;
        set<int> visited;
        for (int b : defBlocks[var])
        {
            work.push(b);
            visited.insert(b);
        }

        while (!work.empty())
        {
            int x = work.front();
            work.pop();
            auto dfIt = domInfo->dominanceFrontiers.find(x);
            if (dfIt == domInfo->dominanceFrontiers.end())
            {
                continue;
            }

            vector<int> dfBlocksSorted(dfIt->second.begin(), dfIt->second.end());
            sort(dfBlocksSorted.begin(), dfBlocksSorted.end());

            for (int y : dfBlocksSorted)
            {
                diag.candidatePhiBlocksByVar[var].insert(y);
                if (phiPlaced[y].count(var))
                {
                    continue;
                }

                bool liveOk = true;
                auto liveFlagIt = hasLiveInfo.find(y);
                if (liveFlagIt != hasLiveInfo.end() && liveFlagIt->second)
                {
                    liveOk = blockLiveIn[y].count(var) > 0;
                }
                if (!liveOk)
                {
                    continue;
                }

                auto blockIt = domInfo->labelToBlock.find(y);
                if (blockIt == domInfo->labelToBlock.end() || !blockIt->second)
                {
                    continue;
                }

                QuadBlock *block = blockIt->second;
                Temp *temp = getOrCreateTemp(tempPool, var);
                QuadType type = getTempType(tempTypes, var);
                QuadTemp *dst = new QuadTemp(temp, type);

                vector<pair<Temp *, Label *>> *args = new vector<pair<Temp *, Label *>>();
                auto predIt = domInfo->predecessors.find(y);
                if (predIt != domInfo->predecessors.end())
                {
                    vector<int> preds(predIt->second.begin(), predIt->second.end());
                    sort(preds.begin(), preds.end());
                    for (int predId : preds)
                    {
                        auto predBlockIt = domInfo->labelToBlock.find(predId);
                        Label *predLabel = nullptr;
                        if (predBlockIt != domInfo->labelToBlock.end() && predBlockIt->second)
                        {
                            predLabel = predBlockIt->second->entry_label;
                        }
                        else
                        {
                            predLabel = new Label(predId);
                        }
                        args->push_back({temp, predLabel});
                    }
                }

                set<Temp *> *def = new set<Temp *>();
                def->insert(temp);
                set<Temp *> *use = new set<Temp *>();
                for (auto &arg : *args)
                {
                    if (arg.first)
                    {
                        use->insert(arg.first);
                    }
                }

                QuadPhi *phi = new QuadPhi(dst, args, def, use);

                size_t insertPos = 0;
                if (block->quadlist && !block->quadlist->empty())
                {
                    if (block->quadlist->at(0)->kind == QuadKind::LABEL)
                    {
                        insertPos = 1;
                    }
                    while (insertPos < block->quadlist->size() &&
                           block->quadlist->at(insertPos)->kind == QuadKind::PHI)
                    {
                        insertPos++;
                    }
                }
                block->quadlist->insert(block->quadlist->begin() + insertPos, phi);
                phiPlaced[y].insert(var);

                if (!defBlocks[var].count(y))
                {
                    defBlocks[var].insert(y);
                    if (!visited.count(y))
                    {
                        work.push(y);
                        visited.insert(y);
                    }
                }
            }
        }
    }
}

// Rename variables to ensure SSA property
// HW7: You need to write this part!!
static void renameVariables(QuadFuncDecl *func, ControlFlowInfo *domInfo, SsaDiagState &diag)
{
#ifdef DEBUG
    cout << "Entering renaming variables for function: " << func->funcname << endl;
#endif
    if (!func || !func->quadblocklist || !domInfo)
    {
        return;
    }

    map<int, QuadType> tempTypes;
    map<int, Temp *> tempPool;
    set<int> defTemps;
    map<int, set<int>> defBlocks;
    collectTempInfo(func, tempTypes, tempPool, defTemps, defBlocks);

    set<int> ssaTemps = defTemps;
    map<int, vector<int>> versionStack;
    map<int, int> nextVersion;
    for (int v : ssaTemps)
    {
        nextVersion[v] = 0;
    }

    auto hasVersion = [&](int tempNum)
    {
        auto it = versionStack.find(tempNum);
        return it != versionStack.end() && !it->second.empty();
    };

    auto newVersionTemp = [&](int tempNum, int blk)
    {
        int version = nextVersion[tempNum]++;
        versionStack[tempNum].push_back(version);
        int newNum = VersionedTemp::versionedTempNum(tempNum, version);
        diag.createdVersionBlocksByVar[tempNum][version].insert(blk);
        return getOrCreateTemp(tempPool, newNum);
    };

    auto currentTemp = [&](int tempNum)
    {
        int version = versionStack[tempNum].back();
        int newNum = VersionedTemp::versionedTempNum(tempNum, version);
        return getOrCreateTemp(tempPool, newNum);
    };

    auto renameTerm = [&](QuadTerm *term)
    {
        if (!term || term->kind != QuadTermKind::TEMP)
        {
            return;
        }
        QuadTemp *qt = term->get_temp();
        if (!qt || !qt->temp)
        {
            return;
        }
        int orig = qt->temp->num;
        if (!ssaTemps.count(orig))
        {
            return;
        }
        if (!hasVersion(orig))
        {
            return;
        }
        qt->temp = currentTemp(orig);
    };

    set<int> visitedBlocks;

    function<void(int)> renameBlock = [&](int blockId)
    {
        if (visitedBlocks.count(blockId))
        {
            return;
        }
        visitedBlocks.insert(blockId);

        auto blockIt = domInfo->labelToBlock.find(blockId);
        if (blockIt == domInfo->labelToBlock.end() || !blockIt->second)
        {
            return;
        }
        QuadBlock *block = blockIt->second;
        vector<int> definedHere;

        if (block->quadlist)
        {
            size_t idx = 0;
            if (!block->quadlist->empty() && block->quadlist->at(0)->kind == QuadKind::LABEL)
            {
                idx = 1;
            }

            for (; idx < block->quadlist->size(); ++idx)
            {
                QuadStm *stm = block->quadlist->at(idx);
                if (!stm || stm->kind != QuadKind::PHI)
                {
                    break;
                }
                auto *phi = static_cast<QuadPhi *>(stm);
                if (!phi->temp_exp || !phi->temp_exp->temp)
                {
                    continue;
                }
                int orig = phi->temp_exp->temp->num;
                if (!ssaTemps.count(orig))
                {
                    continue;
                }
                Temp *newTemp = newVersionTemp(orig, blockId);
                phi->temp_exp->temp = newTemp;
                definedHere.push_back(orig);
            }

            for (auto *stm : *block->quadlist)
            {
                if (!stm || stm->kind == QuadKind::PHI || stm->kind == QuadKind::LABEL)
                {
                    continue;
                }

                switch (stm->kind)
                {
                case QuadKind::MOVE:
                {
                    auto *q = static_cast<QuadMove *>(stm);
                    renameTerm(q->src);
                    if (q->dst && q->dst->temp)
                    {
                        int orig = q->dst->temp->num;
                        if (ssaTemps.count(orig))
                        {
                            Temp *newTemp = newVersionTemp(orig, blockId);
                            q->dst->temp = newTemp;
                            definedHere.push_back(orig);
                        }
                    }
                    break;
                }
                case QuadKind::LOAD:
                {
                    auto *q = static_cast<QuadLoad *>(stm);
                    renameTerm(q->src);
                    if (q->dst && q->dst->temp)
                    {
                        int orig = q->dst->temp->num;
                        if (ssaTemps.count(orig))
                        {
                            Temp *newTemp = newVersionTemp(orig, blockId);
                            q->dst->temp = newTemp;
                            definedHere.push_back(orig);
                        }
                    }
                    break;
                }
                case QuadKind::STORE:
                {
                    auto *q = static_cast<QuadStore *>(stm);
                    renameTerm(q->src);
                    renameTerm(q->dst);
                    break;
                }
                case QuadKind::MOVE_BINOP:
                {
                    auto *q = static_cast<QuadMoveBinop *>(stm);
                    renameTerm(q->left);
                    renameTerm(q->right);
                    if (q->dst && q->dst->temp)
                    {
                        int orig = q->dst->temp->num;
                        if (ssaTemps.count(orig))
                        {
                            Temp *newTemp = newVersionTemp(orig, blockId);
                            q->dst->temp = newTemp;
                            definedHere.push_back(orig);
                        }
                    }
                    break;
                }
                case QuadKind::CALL:
                {
                    auto *q = static_cast<QuadCall *>(stm);
                    renameTerm(q->obj_term);
                    if (q->args)
                    {
                        for (auto *arg : *q->args)
                        {
                            renameTerm(arg);
                        }
                    }
                    break;
                }
                case QuadKind::MOVE_CALL:
                {
                    auto *q = static_cast<QuadMoveCall *>(stm);
                    if (q->call)
                    {
                        renameTerm(q->call->obj_term);
                        if (q->call->args)
                        {
                            for (auto *arg : *q->call->args)
                            {
                                renameTerm(arg);
                            }
                        }
                    }
                    if (q->dst && q->dst->temp)
                    {
                        int orig = q->dst->temp->num;
                        if (ssaTemps.count(orig))
                        {
                            Temp *newTemp = newVersionTemp(orig, blockId);
                            q->dst->temp = newTemp;
                            definedHere.push_back(orig);
                        }
                    }
                    break;
                }
                case QuadKind::EXTCALL:
                {
                    auto *q = static_cast<QuadExtCall *>(stm);
                    if (q->args)
                    {
                        for (auto *arg : *q->args)
                        {
                            renameTerm(arg);
                        }
                    }
                    break;
                }
                case QuadKind::MOVE_EXTCALL:
                {
                    auto *q = static_cast<QuadMoveExtCall *>(stm);
                    if (q->extcall && q->extcall->args)
                    {
                        for (auto *arg : *q->extcall->args)
                        {
                            renameTerm(arg);
                        }
                    }
                    if (q->dst && q->dst->temp)
                    {
                        int orig = q->dst->temp->num;
                        if (ssaTemps.count(orig))
                        {
                            Temp *newTemp = newVersionTemp(orig, blockId);
                            q->dst->temp = newTemp;
                            definedHere.push_back(orig);
                        }
                    }
                    break;
                }
                case QuadKind::CJUMP:
                {
                    auto *q = static_cast<QuadCJump *>(stm);
                    renameTerm(q->left);
                    renameTerm(q->right);
                    break;
                }
                case QuadKind::RETURN:
                {
                    auto *q = static_cast<QuadReturn *>(stm);
                    renameTerm(q->exp);
                    break;
                }
                case QuadKind::PTR_CALC:
                {
                    auto *q = static_cast<QuadPtrCalc *>(stm);
                    renameTerm(q->ptr);
                    renameTerm(q->offset);
                    if (q->dst && q->dst->kind == QuadTermKind::TEMP)
                    {
                        QuadTemp *dt = q->dst->get_temp();
                        if (dt && dt->temp)
                        {
                            int orig = dt->temp->num;
                            if (ssaTemps.count(orig))
                            {
                                Temp *newTemp = newVersionTemp(orig, blockId);
                                dt->temp = newTemp;
                                definedHere.push_back(orig);
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        auto succIt = domInfo->successors.find(blockId);
        if (succIt != domInfo->successors.end())
        {
            for (int succId : succIt->second)
            {
                auto succBlockIt = domInfo->labelToBlock.find(succId);
                if (succBlockIt == domInfo->labelToBlock.end() || !succBlockIt->second)
                {
                    continue;
                }
                QuadBlock *succ = succBlockIt->second;
                if (!succ->quadlist)
                {
                    continue;
                }
                for (auto *stm : *succ->quadlist)
                {
                    if (!stm)
                    {
                        continue;
                    }
                    if (stm->kind == QuadKind::LABEL)
                    {
                        continue;
                    }
                    if (stm->kind != QuadKind::PHI)
                    {
                        break;
                    }
                    auto *phi = static_cast<QuadPhi *>(stm);
                    if (!phi->temp_exp || !phi->temp_exp->temp)
                    {
                        continue;
                    }
                    if (phi->args)
                    {
                        for (auto &arg : *phi->args)
                        {
                            if (arg.second && arg.second->num == blockId)
                            {
                                if (!arg.first)
                                {
                                    continue;
                                }
                                int base = arg.first->num;
                                if (!ssaTemps.count(base))
                                {
                                    continue;
                                }
                                if (!hasVersion(base))
                                {
                                    continue;
                                }
                                arg.first = currentTemp(base);
                            }
                        }
                    }
                }
            }
        }

        auto domIt = domInfo->domTree.find(blockId);
        if (domIt != domInfo->domTree.end())
        {
            vector<int> children(domIt->second.begin(), domIt->second.end());
            sort(children.begin(), children.end());
            for (int child : children)
            {
                renameBlock(child);
            }
        }

        for (auto it = definedHere.rbegin(); it != definedHere.rend(); ++it)
        {
            int tempNum = *it;
            if (!versionStack[tempNum].empty())
            {
                versionStack[tempNum].pop_back();
            }
        }
    };

    if (domInfo->entryBlock != -1)
    {
        renameBlock(domInfo->entryBlock);
    }

    if (func->quadblocklist)
    {
        for (auto *block : *func->quadblocklist)
        {
            if (block && block->entry_label && !visitedBlocks.count(block->entry_label->num))
            {
                renameBlock(block->entry_label->num);
            }
        }
    }

    rebuildDefUseForFunc(func);
}

// Remove unnecessary phi functions
// HW7: You need to write this part!!
static void cleanupUnusedPhi(QuadFuncDecl *func, SsaDiagState &diag)
{
#ifdef DEBUG
    cout << "Cleaning up unused phi functions for function: " << func->funcname << endl;
#endif
    if (!func || !func->quadblocklist)
    {
        return;
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        set<int> usedTemps;
        for (auto *block : *func->quadblocklist)
        {
            if (!block || !block->quadlist)
            {
                continue;
            }
            for (auto *stm : *block->quadlist)
            {
                if (!stm)
                {
                    continue;
                }
                switch (stm->kind)
                {
                case QuadKind::MOVE:
                {
                    auto *q = static_cast<QuadMove *>(stm);
                    addTempNumFromTerm(q->src, usedTemps);
                    break;
                }
                case QuadKind::LOAD:
                {
                    auto *q = static_cast<QuadLoad *>(stm);
                    addTempNumFromTerm(q->src, usedTemps);
                    break;
                }
                case QuadKind::STORE:
                {
                    auto *q = static_cast<QuadStore *>(stm);
                    addTempNumFromTerm(q->src, usedTemps);
                    addTempNumFromTerm(q->dst, usedTemps);
                    break;
                }
                case QuadKind::MOVE_BINOP:
                {
                    auto *q = static_cast<QuadMoveBinop *>(stm);
                    addTempNumFromTerm(q->left, usedTemps);
                    addTempNumFromTerm(q->right, usedTemps);
                    break;
                }
                case QuadKind::CALL:
                {
                    auto *q = static_cast<QuadCall *>(stm);
                    addTempNumFromTerm(q->obj_term, usedTemps);
                    if (q->args)
                    {
                        for (auto *arg : *q->args)
                        {
                            addTempNumFromTerm(arg, usedTemps);
                        }
                    }
                    break;
                }
                case QuadKind::MOVE_CALL:
                {
                    auto *q = static_cast<QuadMoveCall *>(stm);
                    if (q->call)
                    {
                        addTempNumFromTerm(q->call->obj_term, usedTemps);
                        if (q->call->args)
                        {
                            for (auto *arg : *q->call->args)
                            {
                                addTempNumFromTerm(arg, usedTemps);
                            }
                        }
                    }
                    break;
                }
                case QuadKind::EXTCALL:
                {
                    auto *q = static_cast<QuadExtCall *>(stm);
                    if (q->args)
                    {
                        for (auto *arg : *q->args)
                        {
                            addTempNumFromTerm(arg, usedTemps);
                        }
                    }
                    break;
                }
                case QuadKind::MOVE_EXTCALL:
                {
                    auto *q = static_cast<QuadMoveExtCall *>(stm);
                    if (q->extcall && q->extcall->args)
                    {
                        for (auto *arg : *q->extcall->args)
                        {
                            addTempNumFromTerm(arg, usedTemps);
                        }
                    }
                    break;
                }
                case QuadKind::CJUMP:
                {
                    auto *q = static_cast<QuadCJump *>(stm);
                    addTempNumFromTerm(q->left, usedTemps);
                    addTempNumFromTerm(q->right, usedTemps);
                    break;
                }
                case QuadKind::PHI:
                {
                    auto *q = static_cast<QuadPhi *>(stm);
                    if (q->args)
                    {
                        for (auto &arg : *q->args)
                        {
                            if (arg.first)
                            {
                                usedTemps.insert(arg.first->num);
                            }
                        }
                    }
                    break;
                }
                case QuadKind::RETURN:
                {
                    auto *q = static_cast<QuadReturn *>(stm);
                    addTempNumFromTerm(q->exp, usedTemps);
                    break;
                }
                case QuadKind::PTR_CALC:
                {
                    auto *q = static_cast<QuadPtrCalc *>(stm);
                    addTempNumFromTerm(q->ptr, usedTemps);
                    addTempNumFromTerm(q->offset, usedTemps);
                    break;
                }
                default:
                    break;
                }
            }
        }

        for (auto *block : *func->quadblocklist)
        {
            if (!block || !block->quadlist)
            {
                continue;
            }
            auto &ql = *block->quadlist;
            for (size_t i = 0; i < ql.size();)
            {
                QuadStm *stm = ql[i];
                if (stm && stm->kind == QuadKind::PHI)
                {
                    auto *phi = static_cast<QuadPhi *>(stm);
                    if (phi->temp_exp && phi->temp_exp->temp)
                    {
                        int dstNum = phi->temp_exp->temp->num;
                        if (!usedTemps.count(dstNum))
                        {
                            int encodedNum = phi->temp_exp->temp->num;
                            int origVar = VersionedTemp::origTempNum(encodedNum);
                            int version = encodedNum % 100;
                            diag.eliminatedVersionBlocksByVar[origVar][version].insert(block->entry_label->num);
                            ql.erase(ql.begin() + i);
                            changed = true;
                            continue;
                        }
                    }
                }
                i++;
            }
        }
    }
}

// Convert blocked Quad with precomputed flow info to SSA form
quad::QuadProgram *quad2ssa(set<FuncFlowInfo *> *allFuncFlow)
{
    if (!allFuncFlow || allFuncFlow->empty())
    {
        return nullptr; // Invalid program
    }

    vector<QuadFuncDecl *> *funcs = new vector<QuadFuncDecl *>();
    funcs->reserve(allFuncFlow->size());
    int prog_last_label_num = -1;
    int prog_last_temp_num = -1;

    for (auto *ffi : *allFuncFlow)
    {
        if (!ffi || !ffi->cfi || !ffi->cfi->func)
        {
            continue;
        }
        QuadFuncDecl *funcdecl = ffi->cfi->func;

        ControlFlowInfo *domInfo = ffi->cfi;
        DataFlowInfo *liveness = ffi->dfi;
        SsaDiagState diag;
        diag.funcName = funcdecl->funcname;

        // Now Place Phi functions at join points
        placePhi(funcdecl, domInfo, liveness, diag);
        // Rename variables to ensure SSA property
        renameVariables(funcdecl, domInfo, diag);
        // Clean up unnecessary phi nodes
        cleanupUnusedPhi(funcdecl, diag);
        printSsaDiagSummary(funcdecl, diag);

        funcs->push_back(funcdecl);
        if (prog_last_label_num < funcdecl->last_label_num)
        {
            prog_last_label_num = funcdecl->last_label_num;
        }
        if (prog_last_temp_num < funcdecl->last_temp_num)
        {
            prog_last_temp_num = funcdecl->last_temp_num;
        }
    }

    sort(funcs->begin(), funcs->end(), [](QuadFuncDecl *a, QuadFuncDecl *b)
         {
             if (!a || !b)
             {
                 return a != nullptr;
             }
             const bool aIsMain = a->funcname == "__$main__^main";
             const bool bIsMain = b->funcname == "__$main__^main";
             if (aIsMain != bIsMain)
             {
                 return aIsMain;
             }
             return a->funcname < b->funcname; });

    return new QuadProgram(funcs, prog_last_label_num, prog_last_temp_num);
}
