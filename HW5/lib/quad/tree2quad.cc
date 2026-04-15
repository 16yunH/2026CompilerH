#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "treep.hh"
#include "quad.hh"
#include "tree2quad.hh"

using namespace std;
using namespace tree;
using namespace quad;

/*
We use an instruction selection method (pattern matching) to convert the IR tree to Quad.
*/

// you need to implement the following function to convert the IR tree to Quad, 
// and the visit functions for each tree node (the visit functions should generate the 
// corresponding Quad instructions and add them to the visit_result vector)

QuadProgram* tree2quad(Program* prog) {
#ifdef DEBUG
    cout << "in Tree2Quad::Converting IR to Quad" << endl;
#endif
    if (prog == nullptr) {
        return nullptr;
    }

    Tree2Quad v;

    vector<QuadFuncDecl*>* quadfuncdecllist = new vector<QuadFuncDecl*>(); //to be filled in the visit function for FuncDecl
    int prog_last_label = 0; //to be determined
    int prog_last_temp = 0; //to be determined 

    return new QuadProgram(quadfuncdecllist, prog_last_label, prog_last_temp);
}

void Tree2Quad::visit(tree::Program *prog) { }
void Tree2Quad::visit(tree::FuncDecl *func) { }
void Tree2Quad::visit(tree::Jump *jump) { }
void Tree2Quad::visit(tree::Cjump *cjump) { }
void Tree2Quad::visit(tree::Move *move) { }
void Tree2Quad::visit(tree::Seq *seq) { }
void Tree2Quad::visit(tree::LabelStm *labelstm) { }
void Tree2Quad::visit(tree::Return *ret) { }
void Tree2Quad::visit(tree::ExpStm *exp) { }
void Tree2Quad::visit(tree::Binop *binop) { }
void Tree2Quad::visit(tree::Mem *mem) { }
void Tree2Quad::visit(tree::TempExp *tempexp) { }
void Tree2Quad::visit(tree::Eseq *eseq) { }
void Tree2Quad::visit(tree::Name *name) { }
void Tree2Quad::visit(tree::Const *const) { }
void Tree2Quad::visit(tree::Call *call) { }
void Tree2Quad::visit(tree::ExtCall *extcall) { }
