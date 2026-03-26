#define DEBUG
#undef DEBUG

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "namemaps.hh"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <variant>
#include <vector>

using namespace std;
using namespace fdmj;

namespace {

[[noreturn]] void name_map_error(Pos *pos, const string &msg) {
  if (pos != nullptr) {
    cerr << "Error: at position " << pos->print() << endl;
  }
  cerr << "Error: " << msg << endl;
  cerr << "Name mapping failed due to errors. Compilation aborted." << endl;
  exit(EXIT_FAILURE);
}

Formal *build_return_formal(Type *ret_type, const string &method_name,
                            Pos *pos) {
  Pos *p = (pos == nullptr) ? new Pos(0, 0, 0, 0) : pos->clone();
  IdExp *rid = new IdExp(p->clone(), "_^return^_" + method_name);
  Type *rtype = (ret_type == nullptr) ? nullptr : ret_type->clone();
  return new Formal(p, rtype, rid);
}

} // namespace

void AST_Name_Map_Visitor::visit(Program *node) {
#ifdef DEBUG
  std::cout << "Visiting Program" << std::endl;
#endif
  if (node == nullptr) {
    return;
  }
  if (!name_maps->add_class("__$main__")) {
    name_map_error(node->getPos(),
                   "Internal error: duplicated synthetic class __$main__");
  }

  if (node->cdl != nullptr) {
    for (auto cl : *(node->cdl)) {
      if (cl == nullptr || cl->id == nullptr) {
        continue;
      }
      if (!name_maps->add_class(cl->id->id)) {
        name_map_error(cl->id->getPos(),
                       "Class " + cl->id->id + " is already declared");
      }
    }
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

void AST_Name_Map_Visitor::visit(ClassDecl *node) {
  if (node == nullptr || node->id == nullptr) {
    return;
  }
  current_visiting_class = node->id->id;
  current_visiting_method = "";

  if (node->eid != nullptr) {
    if (name_maps->is_class(node->eid->id)) {
      name_maps->add_class_hiearchy(current_visiting_class, node->eid->id);
    }
  }

  if (node->vdl != nullptr) {
    for (auto vd : *(node->vdl)) {
      if (vd != nullptr)
        vd->accept(*this);
    }
  }
  if (node->mdl != nullptr) {
    for (auto md : *(node->mdl)) {
      if (md != nullptr)
        md->accept(*this);
    }
  }

  current_visiting_class = "";
}

void AST_Name_Map_Visitor::visit(MainMethod *node) {
  if (node == nullptr) {
    return;
  }
  current_visiting_class = "__$main__";
  current_visiting_method = "main";

  if (!name_maps->add_method(current_visiting_class, current_visiting_method)) {
    name_map_error(node->getPos(),
                   "Method main is already declared in class __$main__");
  }

  vector<string> formal_names;
  Formal *retf = build_return_formal(
      new Type(node->getPos() == nullptr ? nullptr : node->getPos()->clone()),
      current_visiting_method, node->getPos());
  if (!name_maps->add_method_formal(current_visiting_class,
                                    current_visiting_method, retf->id->id,
                                    retf)) {
    name_map_error(
        node->getPos(),
        "Internal error: duplicated synthetic return formal for main");
  }
  formal_names.push_back(retf->id->id);
  name_maps->add_method_formal_list(current_visiting_class,
                                    current_visiting_method, formal_names);

  if (node->vdl != nullptr) {
    for (auto vd : *(node->vdl)) {
      if (vd != nullptr)
        vd->accept(*this);
    }
  }
  if (node->sl != nullptr) {
    for (auto s : *(node->sl)) {
      if (s != nullptr)
        s->accept(*this);
    }
  }

  current_visiting_method = "";
  current_visiting_class = "";
}

void AST_Name_Map_Visitor::visit(Type *node) { (void)node; }

void AST_Name_Map_Visitor::visit(VarDecl *node) {
  if (node == nullptr || node->id == nullptr) {
    return;
  }

  if (current_visiting_method.empty()) {
    if (!name_maps->add_class_var(current_visiting_class, node->id->id, node)) {
      name_map_error(node->id->getPos(), "Variable " + node->id->id +
                                             " is already declared in class " +
                                             current_visiting_class);
    }
    return;
  }

  if (!name_maps->add_method_var(current_visiting_class,
                                 current_visiting_method, node->id->id, node)) {
    name_map_error(
        node->id->getPos(),
        "Variable " + node->id->id + " is already declared in method " +
            current_visiting_method + " of class " + current_visiting_class);
  }
}

void AST_Name_Map_Visitor::visit(MethodDecl *node) {
  if (node == nullptr || node->id == nullptr) {
    return;
  }

  if (!name_maps->add_method(current_visiting_class, node->id->id)) {
    name_map_error(node->id->getPos(), "Method " + node->id->id +
                                           " is already declared in class " +
                                           current_visiting_class);
  }

  current_visiting_method = node->id->id;
  vector<string> formal_names;

  if (node->fl != nullptr) {
    for (auto f : *(node->fl)) {
      if (f == nullptr || f->id == nullptr) {
        continue;
      }
      f->accept(*this);
      formal_names.push_back(f->id->id);
    }
  }

  Formal *retf =
      build_return_formal(node->type, current_visiting_method, node->getPos());
  if (!name_maps->add_method_formal(current_visiting_class,
                                    current_visiting_method, retf->id->id,
                                    retf)) {
    name_map_error(
        node->getPos(),
        "Internal error: duplicated synthetic return formal in method " +
            current_visiting_method);
  }
  formal_names.push_back(retf->id->id);
  if (!name_maps->add_method_formal_list(
          current_visiting_class, current_visiting_method, formal_names)) {
    name_map_error(node->getPos(),
                   "Internal error: cannot record formal list for method " +
                       current_visiting_method);
  }

  if (node->vdl != nullptr) {
    for (auto vd : *(node->vdl)) {
      if (vd != nullptr)
        vd->accept(*this);
    }
  }
  if (node->sl != nullptr) {
    for (auto s : *(node->sl)) {
      if (s != nullptr)
        s->accept(*this);
    }
  }

  current_visiting_method = "";
}

void AST_Name_Map_Visitor::visit(Formal *node) {
  if (node == nullptr || node->id == nullptr) {
    return;
  }
    if (!name_maps->add_method_formal(current_visiting_class,
                                      current_visiting_method, node->id->id,
                                      node)) {
    name_map_error(
        node->id->getPos(),
        "Variable " + node->id->id + " is already declared in method " +
            current_visiting_method + " of class " + current_visiting_class);
  }
}

void AST_Name_Map_Visitor::visit(Nested *node) {
  if (node == nullptr || node->sl == nullptr)
    return;
  for (auto s : *(node->sl)) {
    if (s != nullptr)
      s->accept(*this);
  }
}

void AST_Name_Map_Visitor::visit(If *node) {
  if (node == nullptr)
    return;
  if (node->exp != nullptr)
    node->exp->accept(*this);
  if (node->stm1 != nullptr)
    node->stm1->accept(*this);
  if (node->stm2 != nullptr)
    node->stm2->accept(*this);
}

void AST_Name_Map_Visitor::visit(While *node) {
  if (node == nullptr)
    return;
  if (node->exp != nullptr)
    node->exp->accept(*this);
  if (node->stm != nullptr)
    node->stm->accept(*this);
}

void AST_Name_Map_Visitor::visit(Assign *node) {
  if (node == nullptr)
    return;
  if (node->left != nullptr)
    node->left->accept(*this);
  if (node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(CallStm *node) {
  if (node == nullptr)
    return;
  if (node->obj != nullptr)
    node->obj->accept(*this);
  if (node->name != nullptr)
    node->name->accept(*this);
  if (node->par != nullptr) {
    for (auto p : *(node->par)) {
      if (p != nullptr)
        p->accept(*this);
    }
  }
}

void AST_Name_Map_Visitor::visit(Continue *node) { (void)node; }
void AST_Name_Map_Visitor::visit(Break *node) { (void)node; }

void AST_Name_Map_Visitor::visit(Return *node) {
  if (node != nullptr && node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(PutInt *node) {
  if (node != nullptr && node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(PutCh *node) {
  if (node != nullptr && node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(PutArray *node) {
  if (node == nullptr)
    return;
  if (node->n != nullptr)
    node->n->accept(*this);
  if (node->arr != nullptr)
    node->arr->accept(*this);
}

void AST_Name_Map_Visitor::visit(Starttime *node) { (void)node; }
void AST_Name_Map_Visitor::visit(Stoptime *node) { (void)node; }

void AST_Name_Map_Visitor::visit(BinaryOp *node) {
  if (node == nullptr)
    return;
  if (node->left != nullptr)
    node->left->accept(*this);
  if (node->op != nullptr)
    node->op->accept(*this);
  if (node->right != nullptr)
    node->right->accept(*this);
}

void AST_Name_Map_Visitor::visit(UnaryOp *node) {
  if (node == nullptr)
    return;
  if (node->op != nullptr)
    node->op->accept(*this);
  if (node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(ArrayExp *node) {
  if (node == nullptr)
    return;
  if (node->arr != nullptr)
    node->arr->accept(*this);
  if (node->index != nullptr)
    node->index->accept(*this);
}

void AST_Name_Map_Visitor::visit(CallExp *node) {
  if (node == nullptr)
    return;
  if (node->obj != nullptr)
    node->obj->accept(*this);
  if (node->name != nullptr)
    node->name->accept(*this);
  if (node->par != nullptr) {
    for (auto p : *(node->par)) {
      if (p != nullptr)
        p->accept(*this);
    }
  }
}

void AST_Name_Map_Visitor::visit(ClassVar *node) {
  if (node == nullptr)
    return;
  if (node->obj != nullptr)
    node->obj->accept(*this);
  if (node->id != nullptr)
    node->id->accept(*this);
}

void AST_Name_Map_Visitor::visit(This *node) { (void)node; }

void AST_Name_Map_Visitor::visit(Length *node) {
  if (node != nullptr && node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(NewArray *node) {
  if (node != nullptr && node->size != nullptr)
    node->size->accept(*this);
}

void AST_Name_Map_Visitor::visit(NewObject *node) {
  if (node != nullptr && node->id != nullptr)
    node->id->accept(*this);
}

void AST_Name_Map_Visitor::visit(GetInt *node) { (void)node; }
void AST_Name_Map_Visitor::visit(GetCh *node) { (void)node; }

void AST_Name_Map_Visitor::visit(GetArray *node) {
  if (node != nullptr && node->exp != nullptr)
    node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(IdExp *node) { (void)node; }
void AST_Name_Map_Visitor::visit(OpExp *node) { (void)node; }
void AST_Name_Map_Visitor::visit(IntExp *node) { (void)node; }
