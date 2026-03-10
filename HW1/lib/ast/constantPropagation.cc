#define DEBUG
#undef DEBUG

#include "constantPropagation.hh"
#include "MinusIntConverter.hh"

#include <optional>
#include <vector>

using namespace std;
using namespace fdmj;

#define StmList vector<Stm *>

namespace {

class ConstantPropagation : public ASTVisitor {
public:
  AST *newNode = nullptr;

  void visit(Program *node) override;
  void visit(MainMethod *node) override;
  void visit(Assign *node) override;
  void visit(Return *node) override;
  void visit(BinaryOp *node) override;
  void visit(UnaryOp *node) override;
  void visit(IdExp *node) override;
  void visit(OpExp *node) override;
  void visit(IntExp *node) override;
};

template <typename T>
static vector<T *> *visitList(ConstantPropagation &v, vector<T *> *tl) {
  if (tl == nullptr || tl->empty())
    return nullptr;
  vector<T *> *vt = new vector<T *>();
  for (T *x : *tl) {
    if (x == nullptr)
      continue;
    x->accept(v);
    if (v.newNode == nullptr)
      continue;
    vt->push_back(static_cast<T *>(v.newNode));
  }
  if (vt->empty()) {
    delete vt;
    return nullptr;
  }
  return vt;
}

static optional<int> foldBinary(const string &op, int lhs, int rhs) {
  if (op == "+")
    return lhs + rhs;
  if (op == "-")
    return lhs - rhs;
  if (op == "*")
    return lhs * rhs;
  if (op == "/") {
    if (rhs == 0)
      return nullopt;
    return lhs / rhs;
  }
  return nullopt;
}

void ConstantPropagation::visit(Program *node) {
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  MainMethod *m = nullptr;
  if (node->main != nullptr) {
    node->main->accept(*this);
    m = static_cast<MainMethod *>(newNode);
  }
  newNode = new Program(node->getPos()->clone(), m);
}

void ConstantPropagation::visit(MainMethod *node) {
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  StmList *sl =
      (node->sl == nullptr) ? nullptr : visitList<Stm>(*this, node->sl);
  newNode = new MainMethod(node->getPos()->clone(), sl);
}

void ConstantPropagation::visit(Assign *node) {
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  Exp *left = nullptr;
  Exp *exp = nullptr;
  if (node->left != nullptr) {
    node->left->accept(*this);
    left = static_cast<Exp *>(newNode);
  }
  if (node->exp != nullptr) {
    node->exp->accept(*this);
    exp = static_cast<Exp *>(newNode);
  }
  newNode = new Assign(node->getPos()->clone(), left, exp);
}

void ConstantPropagation::visit(Return *node) {
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  Exp *exp = nullptr;
  if (node->exp != nullptr) {
    node->exp->accept(*this);
    exp = static_cast<Exp *>(newNode);
  }
  newNode = new Return(node->getPos()->clone(), exp);
}

void ConstantPropagation::visit(BinaryOp *node) {
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  Exp *left = nullptr;
  Exp *right = nullptr;
  OpExp *op = (node->op == nullptr) ? nullptr : node->op->clone();

  if (node->left != nullptr) {
    node->left->accept(*this);
    left = static_cast<Exp *>(newNode);
  }
  if (node->right != nullptr) {
    node->right->accept(*this);
    right = static_cast<Exp *>(newNode);
  }

  if (left != nullptr && right != nullptr && op != nullptr &&
      left->getASTKind() == ASTKind::IntExp &&
      right->getASTKind() == ASTKind::IntExp) {
    int lhs = static_cast<IntExp *>(left)->val;
    int rhs = static_cast<IntExp *>(right)->val;
    optional<int> folded = foldBinary(op->op, lhs, rhs);
    if (folded.has_value()) {
      newNode = new IntExp(node->getPos()->clone(), folded.value());
      return;
    }
  }

  newNode = new BinaryOp(node->getPos()->clone(), left, op, right);
}

void ConstantPropagation::visit(UnaryOp *node) {
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  Exp *exp = nullptr;
  OpExp *op = (node->op == nullptr) ? nullptr : node->op->clone();
  if (node->exp != nullptr) {
    node->exp->accept(*this);
    exp = static_cast<Exp *>(newNode);
  }
  newNode = new UnaryOp(node->getPos()->clone(), op, exp);
}

void ConstantPropagation::visit(IdExp *node) {
  newNode = (node == nullptr) ? nullptr : static_cast<IdExp *>(node->clone());
}

void ConstantPropagation::visit(OpExp *node) {
  newNode = (node == nullptr) ? nullptr : static_cast<OpExp *>(node->clone());
}

void ConstantPropagation::visit(IntExp *node) {
  newNode = (node == nullptr) ? nullptr : static_cast<IntExp *>(node->clone());
}

} // namespace

Program *constantPropagate(Program *root) {
  if (root == nullptr)
    return nullptr;

  Program *rewritten = minusIntRewrite(root);
  if (rewritten == nullptr)
    return nullptr;

  ConstantPropagation v;
  rewritten->accept(v);
  return dynamic_cast<Program *>(v.newNode);
}
