#ifndef __LOOPINDUCTIONOPT_HH__
#define __LOOPINDUCTIONOPT_HH__

#include "loopinductionderived.hh"
#include "loopstrengthreduction.hh"
#include "quad.hh"
#include "flowinfo.hh"

// Utility: Build a mapping from label number to QuadBlock
std::map<int, quad::QuadBlock*> buildLabelToBlock(quad::QuadFuncDecl* func);

// Staged program-level passes for loop induction optimization pipeline
quad::QuadProgram* loopInductionStrengthReductionPass(quad::QuadProgram* prog, ControlFlowInfo* flowInfo = nullptr);
quad::QuadFuncDecl* eliminateUnusedInductionVars(quad::QuadFuncDecl* func);
quad::QuadProgram* loopInductionCleanupPass(quad::QuadProgram* prog);

#endif
