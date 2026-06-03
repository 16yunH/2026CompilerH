#define DEBUG_DATAFLOW
// #undef DEBUG_DATAFLOW

#include "asmdataflow.hh"
#include <iostream>

namespace instr {

AsmDataFlowInfo::AsmDataFlowInfo(AsmFunction* f) : func(f), instrs(&f->instructions) {
}

std::set<int> AsmDataFlowInfo::getUse(size_t instrIdx) const {
    std::set<int> useSet;
    if (instrIdx >= instrs->size()) return useSet;
    
    const AssemInstr& instr = (*instrs)[instrIdx];
    
    // Add all source temporaries
    for (tree::Temp* temp : instr.src) {
        if (temp != nullptr) {
            useSet.insert(temp->num);
        }
    }
    
    return useSet;
}

std::set<int> AsmDataFlowInfo::getDef(size_t instrIdx) const {
    std::set<int> defSet;
    if (instrIdx >= instrs->size()) return defSet;
    
    const AssemInstr& instr = (*instrs)[instrIdx];
    
    // Add all destination temporaries
    for (tree::Temp* temp : instr.dst) {
        if (temp != nullptr) {
            defSet.insert(temp->num);
        }
    }
    
    return defSet;
}

void AsmDataFlowInfo::computeLiveness() {
    if (instrs == nullptr || instrs->empty()) return;
    
    size_t numInstrs = instrs->size();
    bool changed = true;
    int iterations = 0;
    
    // Initialize liveout and livein to empty sets
    for (size_t i = 0; i < numInstrs; ++i) {
        livein[i] = std::set<int>();
        liveout[i] = std::set<int>();
    }
    
    // Iteratively compute liveness (fixed-point iteration)
    while (changed && iterations < 100) {
        changed = false;
        iterations++;
        
        // Process instructions in reverse order
        for (int i = static_cast<int>(numInstrs) - 1; i >= 0; --i) {
            size_t idx = static_cast<size_t>(i);
            
            // Save old livein for change detection
            std::set<int> oldLivein = livein[idx];
            
            // liveout[i] = union of livein[j] for all successors j
            // Successors include:
            // 1. Next instruction in linear flow (idx + 1)
            // 2. Jump target(s) if this is a jump/branch instruction
            if (idx + 1 < numInstrs) {
                for (int temp : livein[idx + 1]) {
                    liveout[idx].insert(temp);
                }
            }
            
            // Handle jump targets (for branches and unconditional jumps)
            const AssemInstr& instr = (*instrs)[idx];
            if (!instr.jumps.labels.empty()) {
                // instr.jumps.labels contains Label* objects
                // We need to find the instruction index for each label
                for (const tree::Label* label : instr.jumps.labels) {
                    // Find the instruction with this label
                    for (size_t j = 0; j < numInstrs; ++j) {
                        const AssemInstr& targetInstr = (*instrs)[j];
                        if (targetInstr.kind == AssemInstr::I_LABEL &&
                            targetInstr.label != nullptr &&
                            targetInstr.label->num == label->num) {
                            // j is the target instruction
                            for (int temp : livein[j]) {
                                liveout[idx].insert(temp);
                            }
                            break;
                        }
                    }
                }
            }
            
            // livein[i] = use[i] union (liveout[i] - def[i])
            std::set<int> use = getUse(idx);
            std::set<int> def = getDef(idx);
            
            livein[idx] = use;
            for (int temp : liveout[idx]) {
                if (def.find(temp) == def.end()) {
                    livein[idx].insert(temp);
                }
            }
            
            if (livein[idx] != oldLivein) {
                changed = true;
            }
        }
    }
}

void AsmDataFlowInfo::printLiveness() const {
    std::cout << "=== Liveness Information ===" << std::endl;
    for (size_t i = 0; i < instrs->size(); ++i) {
        std::cout << "Instruction " << i << " (" << (*instrs)[i].assem << ")" << std::endl;
        
        std::cout << "  Use: ";
        std::set<int> use = getUse(i);
        for (int t : use) std::cout << t << " ";
        std::cout << std::endl;
        
        std::cout << "  Def: ";
        std::set<int> def = getDef(i);
        for (int t : def) std::cout << t << " ";
        std::cout << std::endl;
        
        std::cout << "  Livein: ";
        if (livein.find(i) != livein.end()) {
            for (int t : livein.at(i)) std::cout << t << " ";
        }
        std::cout << std::endl;
        
        std::cout << "  Liveout: ";
        if (liveout.find(i) != liveout.end()) {
            for (int t : liveout.at(i)) std::cout << t << " ";
        }
        std::cout << std::endl;
    }
}

} // namespace instr
