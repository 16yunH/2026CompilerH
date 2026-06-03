#ifndef INSTR_ASSEM_HH
#define INSTR_ASSEM_HH

#include <string>
#include <vector>
#include "quad.hh"

namespace instr {

struct AssemTargets {
    std::vector<tree::Label*> labels;
    AssemTargets() = default;
    explicit AssemTargets(const std::vector<tree::Label*>& labs) : labels(labs) {}
};

struct AssemInstr {
    enum Kind { I_OPER, I_LABEL, I_MOVE, I_CALL, I_EXTCALL } kind;
    std::string assem; // Use `d0, `d1 for defs; `s0, `s1 for uses
    std::vector<tree::Temp*> dst;
    std::vector<tree::Temp*> src;
    AssemTargets jumps; // Only for OPER
    tree::Label* label; // Only for LABEL

    // Constructors for each kind
    static AssemInstr Oper(const std::string& a, const std::vector<tree::Temp*>& d, const std::vector<tree::Temp*>& s, const AssemTargets& j) {
        AssemInstr instr;
        instr.kind = I_OPER;
        instr.assem = a;
        instr.dst = d;
        instr.src = s;
        instr.jumps = j;
        instr.label = nullptr;
        return instr;
    }
    static AssemInstr Label(const std::string& a, tree::Label* l) {
        AssemInstr instr;
        instr.kind = I_LABEL;
        instr.assem = a;
        instr.label = l;
        return instr;
    }
    static AssemInstr Move(const std::string& a, const std::vector<tree::Temp*>& d, const std::vector<tree::Temp*>& s) {
        AssemInstr instr;
        instr.kind = I_MOVE;
        instr.assem = a;
        instr.dst = d;
        instr.src = s;
        instr.label = nullptr;
        return instr;
    }

    static AssemInstr Call(const std::string& funcName, const std::vector<tree::Temp*>& d, const std::vector<tree::Temp*>& s) {
        AssemInstr instr;
        instr.kind = I_CALL;
        instr.assem = funcName;
        instr.dst = d;
        instr.src = s;
        instr.label = nullptr;
        return instr;
    }

    static AssemInstr ExtCall(const std::string& funcName, const std::vector<tree::Temp*>& d, const std::vector<tree::Temp*>& s) {
        AssemInstr instr;
        instr.kind = I_EXTCALL;
        instr.assem = funcName;
        instr.dst = d;
        instr.src = s;
        instr.label = nullptr;
        return instr;
    }
};

struct AssemInstrList {
    std::vector<AssemInstr> instrs;
    void append(const AssemInstr& instr) { instrs.push_back(instr); }
    void extend(const AssemInstrList& other) { instrs.insert(instrs.end(), other.instrs.begin(), other.instrs.end()); }
};

} // namespace instr

#endif // INSTR_ASSEM_HH
