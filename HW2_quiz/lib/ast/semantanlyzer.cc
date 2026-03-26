#define DEBUG
#undef DEBUG

#include "namemaps.hh"
#include "semant.hh"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <variant>
#include <vector>

using namespace std;
using namespace fdmj;

namespace {

class SemanticError : public runtime_error {
public:
  explicit SemanticError(const string &msg) : runtime_error(msg) {}
};

[[noreturn]] void semant_error(Pos *pos, const string &msg) {
  if (pos != nullptr) {
    cerr << "Error: at position " << pos->print() << endl;
  }
  cerr << "Error: " << msg << endl;
  throw SemanticError(msg);
}

bool is_subclass(Name_Maps *nm, const string &child, const string &parent) {
  if (child == parent)
    return true;
  string cur = child;
  while (!cur.empty()) {
    cur = nm->get_parent(cur);
    if (cur == parent) {
      return true;
    }
  }
  return false;
}

AST_Semant *semant_from_type(Type *t, bool lvalue) {
  if (t == nullptr) {
    return new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{},
                          lvalue);
  }
  switch (t->typeKind) {
  case TypeKind::INT:
    return new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{},
                          lvalue);
  case TypeKind::ARRAY: {
    int arity = 0;
    if (t->arity != nullptr) {
      arity = t->arity->val;
    }
    return new AST_Semant(AST_Semant::Kind::Value, TypeKind::ARRAY, arity,
                          lvalue);
  }
  case TypeKind::CLASS: {
    string cname = (t->cid == nullptr) ? "" : t->cid->id;
    return new AST_Semant(AST_Semant::Kind::Value, TypeKind::CLASS, cname,
                          lvalue);
  }
  default:
    return new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{},
                          lvalue);
  }
}

bool same_exact_type(AST_Semant *a, AST_Semant *b) {
  if (a == nullptr || b == nullptr)
    return false;
  if (a->get_type() != b->get_type())
    return false;
  if (a->get_type() == TypeKind::CLASS) {
    return get<string>(a->get_type_par()) == get<string>(b->get_type_par());
  }
  if (a->get_type() == TypeKind::ARRAY) {
    return get<int>(a->get_type_par()) == get<int>(b->get_type_par());
  }
  return true;
}

bool assign_compatible(Name_Maps *nm, AST_Semant *lhs, AST_Semant *rhs) {
  if (lhs == nullptr || rhs == nullptr)
    return false;
  if (lhs->get_type() != rhs->get_type())
    return false;
  if (lhs->get_type() == TypeKind::CLASS) {
    return is_subclass(nm, get<string>(rhs->get_type_par()),
                       get<string>(lhs->get_type_par()));
  }
  if (lhs->get_type() == TypeKind::ARRAY) {
    return get<int>(lhs->get_type_par()) == get<int>(rhs->get_type_par());
  }
  return true;
}

VarDecl *lookup_class_var(Name_Maps *nm, const string &class_name,
                          const string &var_name) {
  string cur = class_name;
  while (!cur.empty()) {
    VarDecl *vd = nm->get_class_var(cur, var_name);
    if (vd != nullptr) {
      return vd;
    }
    cur = nm->get_parent(cur);
  }
  return nullptr;
}

string lookup_method_owner(Name_Maps *nm, const string &class_name,
                           const string &method_name) {
  string cur = class_name;
  while (!cur.empty()) {
    if (nm->is_method(cur, method_name)) {
      return cur;
    }
    cur = nm->get_parent(cur);
  }
  return "";
}

bool same_type_decl(Type *a, Type *b) {
  if (a == nullptr || b == nullptr)
    return false;
  if (a->typeKind != b->typeKind)
    return false;
  if (a->typeKind == TypeKind::CLASS) {
    if (a->cid == nullptr || b->cid == nullptr)
      return false;
    return a->cid->id == b->cid->id;
  }
  if (a->typeKind == TypeKind::ARRAY) {
    int aa = (a->arity == nullptr) ? 0 : a->arity->val;
    int bb = (b->arity == nullptr) ? 0 : b->arity->val;
    return aa == bb;
  }
  return true;
}

bool return_type_compatible(Name_Maps *nm, Type *parent_ret, Type *child_ret) {
  if (parent_ret == nullptr || child_ret == nullptr)
    return false;
  if (parent_ret->typeKind != child_ret->typeKind)
    return false;
  if (parent_ret->typeKind == TypeKind::CLASS) {
    if (parent_ret->cid == nullptr || child_ret->cid == nullptr)
      return false;
    return is_subclass(nm, child_ret->cid->id, parent_ret->cid->id);
  }
  return same_type_decl(parent_ret, child_ret);
}

void ensure_int_expr(AST_Semant_Map *sm, Exp *e, Pos *err_pos,
                     const string &msg) {
  AST_Semant *s = sm->getSemant(e);
  if (s == nullptr) {
    semant_error(err_pos, msg + " has no semantic information");
  }
  if (s->get_type() != TypeKind::INT) {
    semant_error(err_pos, msg);
  }
}

} // namespace

constexpr const char *kImmutableSuffix = "_Immutable";

bool is_shallow_immutable_class_name(const string &class_name) {
  const size_t suffix_len = 10; // "_Immutable"
  return class_name.length() >= suffix_len &&
         class_name.substr(class_name.length() - suffix_len) ==
             kImmutableSuffix;
}

void check_immutable_hierarchy_rules(Program *node, Name_Maps *nm) {
  if (node == nullptr || node->cdl == nullptr || nm == nullptr) {
    return;
  }

  for (auto cl : *(node->cdl)) {
    if (cl == nullptr || cl->id == nullptr || cl->eid == nullptr) {
      continue;
    }

    const string child = cl->id->id;
    const string parent = cl->eid->id;

    if (!nm->is_class(parent)) {
      continue;
    }

    const bool child_imm = is_shallow_immutable_class_name(child);
    const bool parent_imm = is_shallow_immutable_class_name(parent);
    if (child_imm != parent_imm) {
      if (child_imm) {
        semant_error(
            cl->getPos(),
            "Immutable class " + child + " cannot extend mutable class " +
                parent +
                ". Parent of an _Immutable class must also be _Immutable.");
      } else {
        semant_error(cl->getPos(),
                     "Class " + child + " extends immutable class " + parent +
                         ", so it must also be named with suffix _Immutable.");
      }
    }
  }
}

bool writes_class_var_of_immutable_object(AST_Semant_Map *sm, Assign *node) {
  if (sm == nullptr || node == nullptr || node->left == nullptr) {
    return false;
  }

  ClassVar *lhs_class_var = dynamic_cast<ClassVar *>(node->left);
  if (lhs_class_var == nullptr || lhs_class_var->obj == nullptr) {
    return false;
  }

  AST_Semant *obj_sem = sm->getSemant(lhs_class_var->obj);
  if (obj_sem == nullptr || obj_sem->get_type() != TypeKind::CLASS) {
    return false;
  }

  const string obj_class = get<string>(obj_sem->get_type_par());

  return is_shallow_immutable_class_name(obj_class);
}

AST_Semant_Map *semant_analyze(Program *node) {
  std::cerr << "Start Semantic Analysis" << std::endl;
  if (node == nullptr) {
    return nullptr;
  }
  Name_Maps *name_maps = makeNameMaps(node);
  AST_Semant_Visitor semant_visitor(name_maps);
  try {
    node->accept(semant_visitor);
  } catch (const SemanticError &) {
    std::cerr << "Semantic Analysis failed due to errors" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cerr << "Semantic Analysis Done" << std::endl;
  return semant_visitor.getSemantMap();
}

void AST_Semant_Visitor::visit(Program *node) {
  if (node == nullptr)
    return;
  check_immutable_hierarchy_rules(node, name_maps);
  if (node->main != nullptr) {
    node->main->accept(*this);
  }
  if (node->cdl != nullptr) {
    for (auto cl : *(node->cdl)) {
      if (cl != nullptr)
        cl->accept(*this);
    }
  }
}

void AST_Semant_Visitor::visit(MainMethod *node) {
  if (node == nullptr)
    return;
  current_visiting_class = "__$main__";
  current_visiting_method = "main";

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

void AST_Semant_Visitor::visit(ClassDecl *node) {
  if (node == nullptr || node->id == nullptr)
    return;
  current_visiting_class = node->id->id;
  current_visiting_method = "";

  if (node->eid != nullptr) {
    if (!name_maps->is_class(node->eid->id)) {
      semant_error(node->eid->getPos(), "Class " + current_visiting_class +
                                            " extends undefined class " +
                                            node->eid->id);
    }
    string pp = name_maps->get_parent(node->eid->id);
    if (!pp.empty()) {
      semant_error(node->getPos(),
                   "Class " + current_visiting_class + " extends " +
                       node->eid->id + " which already extends " + pp +
                       ". FDMJ2026 only allows single-level inheritance.");
    }
    string cur = node->eid->id;
    while (!cur.empty()) {
      if (cur == current_visiting_class) {
        semant_error(node->getPos(),
                     "Circular inheritance detected for class " +
                         current_visiting_class);
      }
      cur = name_maps->get_parent(cur);
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

void AST_Semant_Visitor::visit(Type *node) { (void)node; }

void AST_Semant_Visitor::visit(VarDecl *node) {
  if (node == nullptr || node->type == nullptr || node->id == nullptr)
    return;

  if (node->type->typeKind == TypeKind::CLASS) {
    if (node->type->cid == nullptr ||
        !name_maps->is_class(node->type->cid->id)) {
      string cname = (node->type->cid == nullptr) ? "" : node->type->cid->id;
      semant_error(node->id->getPos(), "Variable " + node->id->id +
                                           " has undefined class type " +
                                           cname);
    }
  }

  if (holds_alternative<vector<IntExp *> *>(node->init) &&
      node->type->typeKind != TypeKind::ARRAY) {
    semant_error(node->getPos(),
                 "Only int[] variables can use array initializer");
  }
}

void AST_Semant_Visitor::visit(MethodDecl *node) {
  if (node == nullptr || node->id == nullptr || node->type == nullptr)
    return;
  current_visiting_method = node->id->id;

  if (node->type->typeKind == TypeKind::CLASS) {
    if (node->type->cid == nullptr ||
        !name_maps->is_class(node->type->cid->id)) {
      string cname = (node->type->cid == nullptr) ? "" : node->type->cid->id;
      semant_error(node->id->getPos(), "Method " + node->id->id +
                                           " has undefined return class type " +
                                           cname);
    }
  }

  string parent = name_maps->get_parent(current_visiting_class);
  if (!parent.empty() &&
      name_maps->is_method(parent, current_visiting_method)) {
    vector<Formal *> *pfl =
        name_maps->get_method_formal_list(parent, current_visiting_method);
    vector<Formal *> *cfl = name_maps->get_method_formal_list(
        current_visiting_class, current_visiting_method);
    if (pfl == nullptr || cfl == nullptr || pfl->empty() || cfl->empty()) {
      semant_error(node->getPos(),
                   "Cannot get formal list for overridden method " +
                       current_visiting_method);
    }

    if (pfl->size() != cfl->size()) {
      semant_error(
          node->getPos(),
          "Method " + current_visiting_method +
              " has different signature with the same method in class " +
              parent);
    }

    for (size_t i = 0; i + 1 < pfl->size(); ++i) {
      if (!same_type_decl(pfl->at(i)->type, cfl->at(i)->type)) {
        semant_error(
            node->getPos(),
            "Method " + current_visiting_method +
                " has different signature with the same method in class " +
                parent);
      }
    }

    Type *p_ret = pfl->back()->type;
    Type *c_ret = cfl->back()->type;
    if (!return_type_compatible(name_maps, p_ret, c_ret)) {
      semant_error(node->getPos(), "Method " + current_visiting_method +
                                       " has incompatible class for a return "
                                       "type with the same method in class " +
                                       parent);
    }
  }

  if (node->fl != nullptr) {
    for (auto f : *(node->fl)) {
      if (f != nullptr)
        f->accept(*this);
    }
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

void AST_Semant_Visitor::visit(Formal *node) {
  if (node == nullptr || node->type == nullptr || node->id == nullptr)
    return;
  if (node->type->typeKind == TypeKind::CLASS) {
    if (node->type->cid == nullptr ||
        !name_maps->is_class(node->type->cid->id)) {
      string cname = (node->type->cid == nullptr) ? "" : node->type->cid->id;
      semant_error(node->id->getPos(), "Formal " + node->id->id +
                                           " has undefined class type " +
                                           cname);
    }
  }
}

void AST_Semant_Visitor::visit(Nested *node) {
  if (node == nullptr || node->sl == nullptr)
    return;
  for (auto s : *(node->sl)) {
    if (s != nullptr)
      s->accept(*this);
  }
}

void AST_Semant_Visitor::visit(If *node) {
  if (node == nullptr)
    return;
  if (node->exp != nullptr)
    node->exp->accept(*this);
  AST_Semant *cond = semant_map->getSemant(node->exp);
  if (cond == nullptr || cond->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "If condition must be of integer type");
  }
  if (node->stm1 != nullptr)
    node->stm1->accept(*this);
  if (node->stm2 != nullptr)
    node->stm2->accept(*this);
}

void AST_Semant_Visitor::visit(While *node) {
  if (node == nullptr)
    return;
  if (node->exp != nullptr)
    node->exp->accept(*this);
  AST_Semant *cond = semant_map->getSemant(node->exp);
  if (cond == nullptr || cond->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "While condition must be of integer type");
  }
  in_a_while_loop++;
  if (node->stm != nullptr)
    node->stm->accept(*this);
  in_a_while_loop--;
}

void AST_Semant_Visitor::visit(Assign *node) {
  if (node == nullptr)
    return;
  if (node->left != nullptr)
    node->left->accept(*this);
  if (node->exp != nullptr)
    node->exp->accept(*this);

  AST_Semant *l = semant_map->getSemant(node->left);
  AST_Semant *r = semant_map->getSemant(node->exp);
  if (l == nullptr) {
    semant_error(
        node->getPos(),
        "Assign node has no semantic information for its left expression");
  }
  if (r == nullptr) {
    semant_error(
        node->getPos(),
        "Assign node has no semantic information for its right expression");
  }
  if (!l->is_lvalue()) {
    semant_error(node->getPos(), "Left side of assignment must be an lvalue");
  }
  if (writes_class_var_of_immutable_object(semant_map, node)) {
    ClassVar *lhs_class_var = dynamic_cast<ClassVar *>(node->left);
    string field_name =
        (lhs_class_var != nullptr && lhs_class_var->id != nullptr)
            ? lhs_class_var->id->id
            : "<unknown>";
    semant_error(node->getPos(), "Cannot assign to class variable " +
                                     field_name +
                                     " of a shallow immutable class object");
  }
  if (!assign_compatible(name_maps, l, r)) {
    semant_error(node->getPos(),
                 "Assign node has a different type between left and right");
  }
}

void AST_Semant_Visitor::visit(CallStm *node) {
  if (node == nullptr || node->obj == nullptr || node->name == nullptr)
    return;
  node->obj->accept(*this);
  AST_Semant *obj = semant_map->getSemant(node->obj);
  if (obj == nullptr) {
    semant_error(node->getPos(),
                 "CallStm node has no semantic information for its object");
  }
  if (obj->get_type() != TypeKind::CLASS) {
    semant_error(node->getPos(), "CallStm node has a non-class object");
  }

  string obj_class = get<string>(obj->get_type_par());
  string owner = lookup_method_owner(name_maps, obj_class, node->name->id);
  if (owner.empty()) {
    semant_error(node->name->getPos(), "Method " + node->name->id +
                                           " is undefined in class " +
                                           obj_class);
  }
  semant_map->setSemant(node->name,
                        new AST_Semant(AST_Semant::Kind::MethodName,
                                       TypeKind::INT, monostate{}, false));

  vector<Formal *> *fl =
      name_maps->get_method_formal_list(owner, node->name->id);
  if (fl == nullptr || fl->empty()) {
    semant_error(node->name->getPos(),
                 "Cannot get formal list for method " + node->name->id);
  }

  size_t expected = fl->size() - 1;
  size_t actual = (node->par == nullptr) ? 0 : node->par->size();
  if (expected != actual) {
    semant_error(node->getPos(), "Method " + node->name->id +
                                     " has unmatched number of parameters");
  }
  if (node->par != nullptr) {
    for (size_t i = 0; i < node->par->size(); ++i) {
      Exp *arg = node->par->at(i);
      arg->accept(*this);
      AST_Semant *as = semant_map->getSemant(arg);
      AST_Semant *fs = semant_from_type(fl->at(i)->type, true);
      if (!assign_compatible(name_maps, fs, as)) {
        semant_error(arg->getPos(),
                     "Method " + node->name->id +
                         " has unmatched parameter type at argument " +
                         to_string(i + 1));
      }
    }
  }
}

void AST_Semant_Visitor::visit(Continue *node) {
  if (node == nullptr)
    return;
  if (in_a_while_loop <= 0) {
    semant_error(node->getPos(), "Continue node is not in a loop");
  }
}

void AST_Semant_Visitor::visit(Break *node) {
  if (node == nullptr)
    return;
  if (in_a_while_loop <= 0) {
    semant_error(node->getPos(), "Break node is not in a loop");
  }
}

void AST_Semant_Visitor::visit(Return *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  AST_Semant *es = semant_map->getSemant(node->exp);
  if (es == nullptr) {
    semant_error(node->getPos(),
                 "Return node has no semantic information for its expression");
  }

  string ret_name = "_^return^_" + current_visiting_method;
  Formal *rf = name_maps->get_method_formal(current_visiting_class,
                                            current_visiting_method, ret_name);
  if (rf == nullptr || rf->type == nullptr) {
    semant_error(node->getPos(), "Cannot find return type of current method");
  }
  AST_Semant *rs = semant_from_type(rf->type, false);
  if (!assign_compatible(name_maps, rs, es)) {
    if (rs->get_type() == TypeKind::CLASS &&
        es->get_type() == TypeKind::CLASS) {
      semant_error(
          node->getPos(),
          "Return node has incompatible classes between return and method");
    }
    semant_error(node->getPos(),
                 "Return node has a different type between return and method");
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, es->get_type(),
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(PutInt *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  AST_Semant *e = semant_map->getSemant(node->exp);
  if (e == nullptr || e->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "PutInt expects an integer expression");
  }
}

void AST_Semant_Visitor::visit(PutCh *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  AST_Semant *e = semant_map->getSemant(node->exp);
  if (e == nullptr || e->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "PutCh expects an integer expression");
  }
}

void AST_Semant_Visitor::visit(PutArray *node) {
  if (node == nullptr)
    return;
  if (node->n != nullptr)
    node->n->accept(*this);
  if (node->arr != nullptr)
    node->arr->accept(*this);
  AST_Semant *n = semant_map->getSemant(node->n);
  AST_Semant *a = semant_map->getSemant(node->arr);
  if (n == nullptr || n->get_type() != TypeKind::INT) {
    semant_error(node->getPos(),
                 "PutArray expects integer n as first argument");
  }
  if (a == nullptr || a->get_type() != TypeKind::ARRAY) {
    semant_error(node->getPos(), "PutArray expects int[] as second argument");
  }
}

void AST_Semant_Visitor::visit(Starttime *node) { (void)node; }
void AST_Semant_Visitor::visit(Stoptime *node) { (void)node; }

void AST_Semant_Visitor::visit(BinaryOp *node) {
  if (node == nullptr || node->left == nullptr || node->right == nullptr)
    return;
  node->left->accept(*this);
  node->right->accept(*this);
  AST_Semant *l = semant_map->getSemant(node->left);
  AST_Semant *r = semant_map->getSemant(node->right);
  if (l == nullptr || r == nullptr) {
    semant_error(node->getPos(),
                 "BinaryOp node has no semantic information for its operands");
  }
  if (l->get_type() != TypeKind::INT || r->get_type() != TypeKind::INT) {
    semant_error(node->getPos(),
                 "BinaryOp operands must both be integer expressions");
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(UnaryOp *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  AST_Semant *e = semant_map->getSemant(node->exp);
  if (e == nullptr || e->get_type() != TypeKind::INT) {
    semant_error(node->getPos(),
                 "UnaryOp operand must be an integer expression");
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(ArrayExp *node) {
  if (node == nullptr || node->arr == nullptr || node->index == nullptr)
    return;
  node->arr->accept(*this);
  node->index->accept(*this);
  AST_Semant *a = semant_map->getSemant(node->arr);
  AST_Semant *i = semant_map->getSemant(node->index);
  if (a == nullptr) {
    semant_error(
        node->getPos(),
        "ArrayExp node has no semantic information for its array expression");
  }
  if (a->get_type() != TypeKind::ARRAY) {
    semant_error(node->getPos(),
                 "ArrayExp node has a non-array value expression");
  }
  if (i == nullptr || i->get_type() != TypeKind::INT) {
    semant_error(node->getPos(),
                 "ArrayExp node has a non-integer index expression");
  }
  semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                                             TypeKind::INT, monostate{}, true));
}

void AST_Semant_Visitor::visit(CallExp *node) {
  if (node == nullptr || node->obj == nullptr || node->name == nullptr)
    return;
  node->obj->accept(*this);
  AST_Semant *obj = semant_map->getSemant(node->obj);
  if (obj == nullptr) {
    semant_error(node->getPos(),
                 "CallExp node has no semantic information for its object");
  }
  if (obj->get_type() != TypeKind::CLASS) {
    semant_error(node->getPos(), "CallExp node has a non-class object");
  }

  string obj_class = get<string>(obj->get_type_par());
  string owner = lookup_method_owner(name_maps, obj_class, node->name->id);
  if (owner.empty()) {
    semant_error(node->name->getPos(), "Method " + node->name->id +
                                           " is undefined in class " +
                                           obj_class);
  }
  semant_map->setSemant(node->name,
                        new AST_Semant(AST_Semant::Kind::MethodName,
                                       TypeKind::INT, monostate{}, false));

  vector<Formal *> *fl =
      name_maps->get_method_formal_list(owner, node->name->id);
  if (fl == nullptr || fl->empty()) {
    semant_error(node->name->getPos(),
                 "Cannot get formal list for method " + node->name->id);
  }

  size_t expected = fl->size() - 1;
  size_t actual = (node->par == nullptr) ? 0 : node->par->size();
  if (expected != actual) {
    semant_error(node->getPos(), "Method " + node->name->id +
                                     " has unmatched number of parameters");
  }
  if (node->par != nullptr) {
    for (size_t i = 0; i < node->par->size(); ++i) {
      Exp *arg = node->par->at(i);
      arg->accept(*this);
      AST_Semant *as = semant_map->getSemant(arg);
      AST_Semant *fs = semant_from_type(fl->at(i)->type, true);
      if (!assign_compatible(name_maps, fs, as)) {
        semant_error(arg->getPos(),
                     "Method " + node->name->id +
                         " has unmatched parameter type at argument " +
                         to_string(i + 1));
      }
    }
  }

  Formal *retf = fl->back();
  AST_Semant *ret = semant_from_type(retf->type, false);
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, ret->get_type(),
                                       ret->get_type_par(), false));
}

void AST_Semant_Visitor::visit(ClassVar *node) {
  if (node == nullptr || node->obj == nullptr || node->id == nullptr)
    return;
  node->obj->accept(*this);
  AST_Semant *obj = semant_map->getSemant(node->obj);
  if (obj == nullptr) {
    semant_error(node->getPos(),
                 "ClassVar node has no semantic information for its object");
  }
  if (obj->get_type() != TypeKind::CLASS) {
    semant_error(node->getPos(), "ClassVar node object must be class type");
  }

  string cname = get<string>(obj->get_type_par());
  VarDecl *vd = lookup_class_var(name_maps, cname, node->id->id);
  if (vd == nullptr) {
    semant_error(node->id->getPos(),
                 "Class " + cname + " has no member variable " + node->id->id);
  }

  AST_Semant *s = semant_from_type(vd->type, true);
  semant_map->setSemant(node->id,
                        new AST_Semant(AST_Semant::Kind::Value, s->get_type(),
                                       s->get_type_par(), true));
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, s->get_type(),
                                       s->get_type_par(), true));
}

void AST_Semant_Visitor::visit(This *node) {
  if (node == nullptr)
    return;
  if (current_visiting_class.empty() || current_visiting_class == "__$main__") {
    semant_error(node->getPos(), "'this' is not allowed in main method");
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::CLASS,
                                       current_visiting_class, false));
}

void AST_Semant_Visitor::visit(Length *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  AST_Semant *e = semant_map->getSemant(node->exp);
  if (e == nullptr || e->get_type() != TypeKind::ARRAY) {
    semant_error(node->getPos(), "Length expects an int[] expression");
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(NewArray *node) {
  if (node == nullptr || node->size == nullptr)
    return;
  node->size->accept(*this);
  AST_Semant *s = semant_map->getSemant(node->size);
  if (s == nullptr || s->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "NewArray size expression must be integer");
  }
  semant_map->setSemant(
      node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::ARRAY, 0, false));
}

void AST_Semant_Visitor::visit(NewObject *node) {
  if (node == nullptr || node->id == nullptr)
    return;
  if (!name_maps->is_class(node->id->id)) {
    semant_error(node->id->getPos(),
                 "NewObject uses undefined class " + node->id->id);
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::CLASS,
                                       node->id->id, false));
}

void AST_Semant_Visitor::visit(GetInt *node) {
  if (node == nullptr)
    return;
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(GetCh *node) {
  if (node == nullptr)
    return;
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(GetArray *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  AST_Semant *arr = semant_map->getSemant(node->exp);
  if (arr == nullptr || arr->get_type() != TypeKind::ARRAY) {
    semant_error(node->getPos(), "GetArray expects an int[] expression");
  }
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}

void AST_Semant_Visitor::visit(IdExp *node) {
  if (node == nullptr)
    return;

  VarDecl *vd = nullptr;
  Formal *fm = nullptr;

  if (!current_visiting_method.empty()) {
    vd = name_maps->get_method_var(current_visiting_class,
                                   current_visiting_method, node->id);
    if (vd != nullptr) {
      AST_Semant *s = semant_from_type(vd->type, true);
      semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                                                 s->get_type(),
                                                 s->get_type_par(), true));
      return;
    }

    fm = name_maps->get_method_formal(current_visiting_class,
                                      current_visiting_method, node->id);
    if (fm != nullptr) {
      AST_Semant *s = semant_from_type(fm->type, true);
      semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                                                 s->get_type(),
                                                 s->get_type_par(), true));
      return;
    }
  }

  VarDecl *cv = lookup_class_var(name_maps, current_visiting_class, node->id);
  if (cv != nullptr && !current_visiting_method.empty() &&
      current_visiting_class != "__$main__") {
    semant_error(node->getPos(), "Class variable " + node->id +
                                     " must be accessed via object (this." +
                                     node->id + " or obj." + node->id + ")");
  }

  semant_error(node->getPos(), "Variable " + node->id + " is undefined");
}

void AST_Semant_Visitor::visit(OpExp *node) { (void)node; }

void AST_Semant_Visitor::visit(IntExp *node) {
  if (node == nullptr)
    return;
  semant_map->setSemant(node,
                        new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT,
                                       monostate{}, false));
}