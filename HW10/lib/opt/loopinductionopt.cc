#include "loopinductionopt.hh"
#include "loopstrengthreduction.hh"
#include "loopheader.hh"

using namespace std;
using namespace quad;

map<int, QuadBlock*> buildLabelToBlock(QuadFuncDecl* func) {
    map<int, QuadBlock*> labelToBlock;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return labelToBlock;
    }
    for (QuadBlock* block : *func->quadblocklist) {
        if (block != nullptr && block->entry_label != nullptr) {
            labelToBlock[block->entry_label->num] = block;
        }
    }
    return labelToBlock;
}

QuadProgram* loopInductionStrengthReductionPass(QuadProgram* prog, ControlFlowInfo* flowInfo) {
    if (prog == nullptr) {
        return nullptr;
    }

    QuadProgram* out = prog->clone();
    if (out == nullptr || out->quadFuncDeclList == nullptr) {
        return out;
    }

    for (size_t i = 0; i < out->quadFuncDeclList->size(); ++i) {
        QuadFuncDecl* func = out->quadFuncDeclList->at(i);
        if (func == nullptr) {
            continue;
        }

        // Discover loops using flow information
        LoopHeaderMap* loopHeaderMap = nullptr;
        if (flowInfo != nullptr) {
            loopHeaderMap = findLoopHeadersWithFlow(func, flowInfo);
        } else {
            loopHeaderMap = new LoopHeaderMap();
        }

        // Apply strength reduction optimization
        DefUseChain du(func);
        auto discoveredBasic = discoverBasicInductionVars(func, loopHeaderMap, du, *flowInfo);
        auto discoveredDerived = discoverDerivedInductionVars(func, loopHeaderMap, du, *flowInfo);
        auto srPlan = generateStrengthReductionPlan(func, discoveredDerived, discoveredBasic, loopHeaderMap);
        out->quadFuncDeclList->at(i) = applyStrengthReduction(func, srPlan);

        delete loopHeaderMap;
    }
    return out;
}

QuadProgram* loopInductionCleanupPass(QuadProgram* prog) {
    if (prog == nullptr) {
        return nullptr;
    }

    QuadProgram* out = prog->clone();
    if (out == nullptr || out->quadFuncDeclList == nullptr) {
        return out;
    }

    // Run elimination multiple times to catch all dead code cycles
    for (size_t i = 0; i < out->quadFuncDeclList->size(); ++i) {
        QuadFuncDecl* func = out->quadFuncDeclList->at(i);
        if (func == nullptr) {
            continue;
        }
        for (int iter = 0; iter < 3; ++iter) {
            func = eliminateUnusedInductionVars(func);
        }
        out->quadFuncDeclList->at(i) = func;
    }
    return out;
}
