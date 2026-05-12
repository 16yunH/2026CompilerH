#define DEBUG
#undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <map>
#include "quad.hh"
#include "opt.hh"

QuadFuncDecl* Opt::optFunc() {
    // You need to implement this function. It should return an optimized version of the function.
    return func;
}

QuadProgram* optProg(QuadProgram* prog) {
    QuadProgram* newProg = new QuadProgram(new vector<QuadFuncDecl*>(), prog->last_label_num, prog->last_temp_num);
    for (int i=0; i < prog->quadFuncDeclList->size(); i++) {
        Opt optthis(prog->quadFuncDeclList->at(i));
        newProg->quadFuncDeclList->push_back(optthis.optFunc());
    }
    return newProg;
}