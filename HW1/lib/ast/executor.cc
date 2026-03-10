#define DEBUG
#undef DEBUG

#include "executor.hh"

#include <iostream>

using namespace std;
using namespace fdmj;

namespace {

class Executor : public ASTVisitor {
public:
  map<string, int> env;
  int currentValue = 0;
  int returnValue = 0;
  bool hasReturn = false;

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

void Executor::visit(Program *node) {
  if (node == nullptr || node->main == nullptr)
    return;
  node->main->accept(*this);
}

void Executor::visit(MainMethod *node) {
  if (node == nullptr || node->sl == nullptr)
    return;
  for (Stm *stm : *(node->sl)) {
    if (stm == nullptr)
      continue;
    stm->accept(*this);
    if (hasReturn)
      return;
  }
}

void Executor::visit(Assign *node) {
  if (node == nullptr || node->left == nullptr || node->exp == nullptr)
    return;

  node->exp->accept(*this);
  int rhs = currentValue;

  if (node->left->getASTKind() != ASTKind::IdExp)
    return;
  string name = static_cast<IdExp *>(node->left)->id;
  env[name] = rhs;
}

void Executor::visit(Return *node) {
  if (node == nullptr || node->exp == nullptr)
    return;
  node->exp->accept(*this);
  returnValue = currentValue;
  hasReturn = true;
}

void Executor::visit(BinaryOp *node) {
  if (node == nullptr || node->left == nullptr || node->right == nullptr ||
      node->op == nullptr) {
    currentValue = 0;
    return;
  }

  node->left->accept(*this);
  int lhs = currentValue;
  node->right->accept(*this);
  int rhs = currentValue;

  if (node->op->op == "+") {
    currentValue = lhs + rhs;
  } else if (node->op->op == "-") {
    currentValue = lhs - rhs;
  } else if (node->op->op == "*") {
    currentValue = lhs * rhs;
  } else if (node->op->op == "/") {
    currentValue = (rhs == 0) ? 0 : (lhs / rhs);
  } else {
    currentValue = 0;
  }
}

void Executor::visit(UnaryOp *node) {
  if (node == nullptr || node->exp == nullptr || node->op == nullptr) {
    currentValue = 0;
    return;
  }

  node->exp->accept(*this);
  if (node->op->op == "-") {
    currentValue = -currentValue;
  }
}

void Executor::visit(IdExp *node) {
  if (node == nullptr) {
    currentValue = 0;
    return;
  }

  auto it = env.find(node->id);
  if (it == env.end()) {
    size_t line = 0;
    size_t column = 0;
    if (node->getPos() != nullptr) {
      line = node->getPos()->sline;
      column = node->getPos()->scolumn;
    }
    cerr << "Undefined variable use at line " << line << ", column " << column
         << ". Assume 0." << endl;
    currentValue = 0;
    return;
  }
  currentValue = it->second;
}

void Executor::visit(OpExp *node) {
  (void)node;
  currentValue = 0;
}

void Executor::visit(IntExp *node) {
  currentValue = (node == nullptr) ? 0 : node->val;
}

} // namespace

int execute(Program *root) {
  Executor v;
  if (root != nullptr)
    root->accept(v);
  return v.returnValue;
}
