#include "defusechain.hh"
#include <algorithm>
#include <sstream>
#include "quad.hh"

using namespace std;
using namespace quad;

// Helper function to extract temp number from a QuadTerm
static int getTempNumFromTerm(QuadTerm* term) {
    if (term == nullptr) return -1;
    if (term->kind == QuadTermKind::TEMP) {
        QuadTemp* qt = term->get_temp();
        if (qt != nullptr && qt->temp != nullptr) {
            return qt->temp->num;
        }
    }
    return -1;
}

/**
 * extractDefsFromStm: Extract definitions from a statement
 * Identifies which temporary variables are defined by this statement
 */
void DefUseChain::extractDefsFromStm(quad::QuadStm* stm, quad::QuadBlock* block) {
    if (stm == nullptr) return;

    int defTemp = -1;

    switch (stm->kind) {
        case QuadKind::MOVE: {
            QuadMove* move = dynamic_cast<QuadMove*>(stm);
            if (move != nullptr && move->dst != nullptr && move->dst->temp != nullptr) {
                defTemp = move->dst->temp->num;
            }
            break;
        }
        case QuadKind::LOAD: {
            QuadLoad* load = dynamic_cast<QuadLoad*>(stm);
            if (load != nullptr && load->dst != nullptr && load->dst->temp != nullptr) {
                defTemp = load->dst->temp->num;
            }
            break;
        }
        case QuadKind::MOVE_BINOP: {
            QuadMoveBinop* binop = dynamic_cast<QuadMoveBinop*>(stm);
            if (binop != nullptr && binop->dst != nullptr && binop->dst->temp != nullptr) {
                defTemp = binop->dst->temp->num;
            }
            break;
        }
        case QuadKind::PTR_CALC: {
            QuadPtrCalc* ptrcalc = dynamic_cast<QuadPtrCalc*>(stm);
            if (ptrcalc != nullptr && ptrcalc->dst != nullptr) {
                defTemp = getTempNumFromTerm(ptrcalc->dst);
            }
            break;
        }
        case QuadKind::MOVE_CALL: {
            QuadMoveCall* movecall = dynamic_cast<QuadMoveCall*>(stm);
            if (movecall != nullptr && movecall->dst != nullptr && movecall->dst->temp != nullptr) {
                defTemp = movecall->dst->temp->num;
            }
            break;
        }
        case QuadKind::MOVE_EXTCALL: {
            QuadMoveExtCall* moveextcall = dynamic_cast<QuadMoveExtCall*>(stm);
            if (moveextcall != nullptr && moveextcall->dst != nullptr && moveextcall->dst->temp != nullptr) {
                defTemp = moveextcall->dst->temp->num;
            }
            break;
        }
        case QuadKind::PHI: {
            QuadPhi* phi = dynamic_cast<QuadPhi*>(stm);
            if (phi != nullptr && phi->temp_exp != nullptr && phi->temp_exp->temp != nullptr) {
                defTemp = phi->temp_exp->temp->num;
            }
            break;
        }
        // Other statement types (STORE, CALL, EXTCALL, LABEL, JUMP, CJUMP, RETURN) don't define temps
        default:
            break;
    }

    // If this statement defines a temporary, create/update VarDefInfo
    if (defTemp != -1) {
        if (defMap.find(defTemp) == defMap.end()) {
            defMap[defTemp] = new VarDefInfo(defTemp, stm, block);
        }
        stmToDefs[stm].insert(defTemp);
    }
}

/**
 * extractUsesFromStm: Extract uses from a statement
 * Identifies all temporary variables that are used by this statement
 */
void DefUseChain::extractUsesFromStm(quad::QuadStm* stm, quad::QuadBlock* block) {
    if (stm == nullptr) return;

    set<int> usedTemps;

    auto addTempFromTerm = [&](QuadTerm* term) {
        if (term != nullptr) {
            int tempNum = getTempNumFromTerm(term);
            if (tempNum != -1) {
                usedTemps.insert(tempNum);
            }
        }
    };

    switch (stm->kind) {
        case QuadKind::MOVE: {
            QuadMove* move = dynamic_cast<QuadMove*>(stm);
            if (move != nullptr) {
                addTempFromTerm(move->src);
            }
            break;
        }
        case QuadKind::LOAD: {
            QuadLoad* load = dynamic_cast<QuadLoad*>(stm);
            if (load != nullptr) {
                addTempFromTerm(load->src);
            }
            break;
        }
        case QuadKind::STORE: {
            QuadStore* store = dynamic_cast<QuadStore*>(stm);
            if (store != nullptr) {
                addTempFromTerm(store->src);
                addTempFromTerm(store->dst);
            }
            break;
        }
        case QuadKind::MOVE_BINOP: {
            QuadMoveBinop* binop = dynamic_cast<QuadMoveBinop*>(stm);
            if (binop != nullptr) {
                addTempFromTerm(binop->left);
                addTempFromTerm(binop->right);
            }
            break;
        }
        case QuadKind::PTR_CALC: {
            QuadPtrCalc* ptrcalc = dynamic_cast<QuadPtrCalc*>(stm);
            if (ptrcalc != nullptr) {
                addTempFromTerm(ptrcalc->ptr);
                addTempFromTerm(ptrcalc->offset);
            }
            break;
        }
        case QuadKind::CALL: {
            QuadCall* call = dynamic_cast<QuadCall*>(stm);
            if (call != nullptr) {
                addTempFromTerm(call->obj_term);
                if (call->args != nullptr) {
                    for (QuadTerm* arg : *(call->args)) {
                        addTempFromTerm(arg);
                    }
                }
            }
            break;
        }
        case QuadKind::MOVE_CALL: {
            QuadMoveCall* movecall = dynamic_cast<QuadMoveCall*>(stm);
            if (movecall != nullptr && movecall->call != nullptr) {
                QuadCall* call = movecall->call;
                addTempFromTerm(call->obj_term);
                if (call->args != nullptr) {
                    for (QuadTerm* arg : *(call->args)) {
                        addTempFromTerm(arg);
                    }
                }
            }
            break;
        }
        case QuadKind::EXTCALL: {
            QuadExtCall* extcall = dynamic_cast<QuadExtCall*>(stm);
            if (extcall != nullptr && extcall->args != nullptr) {
                for (QuadTerm* arg : *(extcall->args)) {
                    addTempFromTerm(arg);
                }
            }
            break;
        }
        case QuadKind::MOVE_EXTCALL: {
            QuadMoveExtCall* moveextcall = dynamic_cast<QuadMoveExtCall*>(stm);
            if (moveextcall != nullptr && moveextcall->extcall != nullptr) {
                QuadExtCall* extcall = moveextcall->extcall;
                if (extcall->args != nullptr) {
                    for (QuadTerm* arg : *(extcall->args)) {
                        addTempFromTerm(arg);
                    }
                }
            }
            break;
        }
        case QuadKind::CJUMP: {
            QuadCJump* cjump = dynamic_cast<QuadCJump*>(stm);
            if (cjump != nullptr) {
                addTempFromTerm(cjump->left);
                addTempFromTerm(cjump->right);
            }
            break;
        }
        case QuadKind::PHI: {
            QuadPhi* phi = dynamic_cast<QuadPhi*>(stm);
            if (phi != nullptr && phi->args != nullptr) {
                for (const auto& arg : *(phi->args)) {
                    if (arg.first != nullptr) {
                        usedTemps.insert(arg.first->num);
                    }
                }
            }
            break;
        }
        case QuadKind::RETURN: {
            QuadReturn* ret = dynamic_cast<QuadReturn*>(stm);
            if (ret != nullptr) {
                addTempFromTerm(ret->exp);
            }
            break;
        }
        // LABEL, JUMP don't use any temps
        default:
            break;
    }

    stmToUses[stm] = usedTemps;
}

/**
 * linkDefUseChain: Link definitions to their uses
 * For each use of a temporary, link it to its definition
 */
void DefUseChain::linkDefUseChain() {
    // Iterate through all statements and their uses
    for (auto& stmUsePair : stmToUses) {
        quad::QuadStm* useStm = stmUsePair.first;
        const set<int>& usedTemps = stmUsePair.second;

        // Find the block containing this statement
        quad::QuadBlock* useBlock = nullptr;
        if (func != nullptr && func->quadblocklist != nullptr) {
            for (quad::QuadBlock* block : *(func->quadblocklist)) {
                if (block->quadlist != nullptr) {
                    auto it = find(block->quadlist->begin(), block->quadlist->end(), useStm);
                    if (it != block->quadlist->end()) {
                        useBlock = block;
                        break;
                    }
                }
            }
        }

        // For each temp used by this statement, link it to its definition
        for (int tempNum : usedTemps) {
            if (defMap.find(tempNum) != defMap.end()) {
                VarDefInfo* varDef = defMap[tempNum];
                if (useBlock != nullptr) {
                    varDef->addUse(useBlock, useStm);
                }
            }
        }
    }
}

// ============================================================================
// VarDefInfo implementation
// ============================================================================

// ============================================================================
// UseInfo implementation
// ============================================================================


// ============================================================================
// DefUseChain implementation
// ============================================================================

DefUseChain::DefUseChain(quad::QuadFuncDecl* func)
    : func(func) {
    if (func != nullptr && func->quadblocklist != nullptr) {
        // Extract all definitions and uses
        for (quad::QuadBlock* block : *(func->quadblocklist)) {
            if (block->quadlist != nullptr) {
                for (quad::QuadStm* stm : *(block->quadlist)) {
                    extractDefsFromStm(stm, block);
                    extractUsesFromStm(stm, block);
                }
            }
        }
        // Link definitions to uses
        linkDefUseChain();
    }
}

DefUseChain::~DefUseChain() {
    for (auto& pair : defMap) {
        delete pair.second;
    }
    defMap.clear();
    stmToDefs.clear();
    stmToUses.clear();
}

VarDefInfo* DefUseChain::getDef(int tempNum) const {
    auto it = defMap.find(tempNum);
    if (it != defMap.end()) {
        return it->second;
    }
    return nullptr;
}

set<int> DefUseChain::getDefsBy(quad::QuadStm* stm) const {
    auto it = stmToDefs.find(stm);
    if (it != stmToDefs.end()) {
        return it->second;
    }
    return set<int>();
}

set<int> DefUseChain::getUsesBy(quad::QuadStm* stm) const {
    auto it = stmToUses.find(stm);
    if (it != stmToUses.end()) {
        return it->second;
    }
    return set<int>();
}

