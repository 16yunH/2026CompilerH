#define DEBUG
#undef DEBUG

#include <iostream>
#include <variant>
#include <map>
#include <vector>
#include <algorithm>
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "namemaps.hh"

using namespace std;
using namespace fdmj;

void AST_Name_Map_Visitor::visit(Program* node) {
#ifdef DEBUG
    std::cout << "Visiting Program" << std::endl;
#endif
    if (node == nullptr) {
         return;
    }
    if (node->main != nullptr) {
        node->main->accept(*this);
    }
    if (node->cdl != nullptr) {
        for (auto cl : *(node->cdl)) {
            cl->accept(*this);
        }
    }
}

//rest of the visit functions here, you need to complete the code!
void AST_Name_Map_Visitor::visit(ClassDecl* node) { }
void AST_Name_Map_Visitor::visit(MainMethod* node) { }
void AST_Name_Map_Visitor::visit(Type* node) { }
void AST_Name_Map_Visitor::visit(VarDecl* node) { }
void AST_Name_Map_Visitor::visit(MethodDecl* node) { }
void AST_Name_Map_Visitor::visit(Formal* node) { }
void AST_Name_Map_Visitor::visit(Nested* node) { }
void AST_Name_Map_Visitor::visit(If* node) { }
void AST_Name_Map_Visitor::visit(While* node) { }
void AST_Name_Map_Visitor::visit(Assign* node) { }
void AST_Name_Map_Visitor::visit(CallStm* node) { }
void AST_Name_Map_Visitor::visit(Continue* node) { }
void AST_Name_Map_Visitor::visit(Break* node) { }
void AST_Name_Map_Visitor::visit(Return* node) { }
void AST_Name_Map_Visitor::visit(PutInt* node) { }
void AST_Name_Map_Visitor::visit(PutCh* node) { }
void AST_Name_Map_Visitor::visit(PutArray* node) { }
void AST_Name_Map_Visitor::visit(Starttime* node) { }
void AST_Name_Map_Visitor::visit(Stoptime* node) { }
void AST_Name_Map_Visitor::visit(BinaryOp* node) { }
void AST_Name_Map_Visitor::visit(UnaryOp* node) { }
void AST_Name_Map_Visitor::visit(ArrayExp* node) { }
void AST_Name_Map_Visitor::visit(CallExp* node) { }
void AST_Name_Map_Visitor::visit(ClassVar* node) { }
void AST_Name_Map_Visitor::visit(This* node) { }
void AST_Name_Map_Visitor::visit(Length* node) { }
void AST_Name_Map_Visitor::visit(NewArray* node) { }
void AST_Name_Map_Visitor::visit(NewObject* node) { }
void AST_Name_Map_Visitor::visit(GetInt* node) { }
void AST_Name_Map_Visitor::visit(GetCh* node) { }
void AST_Name_Map_Visitor::visit(GetArray* node) { }
void AST_Name_Map_Visitor::visit(IdExp* node) { }
void AST_Name_Map_Visitor::visit(OpExp* node) { }
void AST_Name_Map_Visitor::visit(IntExp* node) { }
