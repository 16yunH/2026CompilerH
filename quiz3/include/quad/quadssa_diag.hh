#ifndef __QUAD_SSA_DIAG_HH
#define __QUAD_SSA_DIAG_HH

#include <map>
#include <set>
#include <string>

namespace quad {
class QuadFuncDecl;
}

struct SsaDiagState {
    std::string funcName;
    std::map<int, std::set<int>> candidatePhiBlocksByVar;
    std::map<int, std::set<int>> actualPhiBlocksByVar;
    std::map<int, std::map<int, std::set<int>>> createdVersionBlocksByVar;
    std::map<int, std::map<int, std::set<int>>> eliminatedVersionBlocksByVar;
};

void printSsaDiagSummary(quad::QuadFuncDecl* func, SsaDiagState& diag);

#endif // __QUAD_SSA_DIAG_HH
