# HW2 现场小测预案：Semantic Check / NameMap 加检查

## 1. 明天你大概率会被要求做什么

结合老师的描述（在你现有 HW2 代码基础上，现场给一个新需求，主要改 semantic visitor，可能改 setnamemap visitor），小测常见形式通常是：

1. 给你一条新的语义规则，让你补到 AST_Semant_Visitor 里。
2. 要求错误在语义分析阶段报出（打印位置 + 错误信息），并中止。
3. 如果新增规则依赖名字信息，要求你同步扩展 AST_Name_Map_Visitor 或 Name_Maps 结构。
4. 用 1-2 个 fmj 测试程序现场验证。

你现在的代码已经比较完整，明天更像是“在已有 visitor 上加 1-3 个检查分支”，而不是重写框架。

---

## 2. 你当前代码里已经做了的关键检查（先心里有底）

你在下面这些文件里已经实现了大量核心语义检查：

- lib/ast/semantanlyzer.cc
- lib/ast/setnamemaps.cc
- lib/ast/namemaps.cc

已经覆盖的典型项：

1. 类型存在性（class type 必须已定义）
2. if/while 条件必须是 int
3. 赋值左右类型兼容（类支持子类赋给父类）
4. 方法调用：对象必须是 class、方法存在、参数个数和类型匹配
5. break/continue 必须在 while 中
6. this 不能在 main 中使用
7. 数组相关（下标必须 int、length 参数必须是数组、new int[size] 的 size 必须 int）
8. 继承相关（父类存在、单层继承、重写签名检查、返回类型协变检查）
9. 禁止在方法里裸用类成员名（必须 this.x 或 obj.x）

这意味着你明天最需要练的是：快速定位 visitor 中对应函数，然后补一个新 if 分支报错。

---

## 3. 明天最可能新增的检查点（按概率排序）

## 3.1 运算符合法性与分类检查（高概率）

### 3.1 现状

当前 BinaryOp / UnaryOp 主要只检查操作数是 int，但没有严格校验 op 字符串是否合法，也没区分哪些运算符允许二元/一元场景。

### 3.1 可能要求

1. BinaryOp 只允许 + - * / % < <= > >= == != && ||
2. UnaryOp 只允许 ! 和 -
3. 运算符非法时立刻报错

### 3.1 需要改的文件

- lib/ast/semantanlyzer.cc

### 代码示例（可直接嵌入）

~~~cpp
namespace {

bool is_valid_binary_op(const string &op) {
  static const set<string> kOps = {
      "+", "-", "*", "/", "%",
      "<", "<=", ">", ">=", "==", "!=",
      "&&", "||"
  };
  return kOps.find(op) != kOps.end();
}

bool is_valid_unary_op(const string &op) {
  return op == "!" || op == "-";
}

} // namespace

void AST_Semant_Visitor::visit(BinaryOp *node) {
  if (node == nullptr || node->left == nullptr || node->right == nullptr || node->op == nullptr)
    return;

  if (!is_valid_binary_op(node->op->op)) {
    semant_error(node->op->getPos(), "Unsupported binary operator: " + node->op->op);
  }

  node->left->accept(*this);
  node->right->accept(*this);
  AST_Semant *l = semant_map->getSemant(node->left);
  AST_Semant *r = semant_map->getSemant(node->right);

  if (l == nullptr || r == nullptr || l->get_type() != TypeKind::INT || r->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "BinaryOp operands must both be integer expressions");
  }

  semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(UnaryOp *node) {
  if (node == nullptr || node->exp == nullptr || node->op == nullptr)
    return;

  if (!is_valid_unary_op(node->op->op)) {
    semant_error(node->op->getPos(), "Unsupported unary operator: " + node->op->op);
  }

  node->exp->accept(*this);
  AST_Semant *e = semant_map->getSemant(node->exp);
  if (e == nullptr || e->get_type() != TypeKind::INT) {
    semant_error(node->getPos(), "UnaryOp operand must be an integer expression");
  }

  semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}
~~~

---

## 3.2 方法必须有 return（高概率）

### 3.2 现状

目前 Return 节点会做“返回值类型匹配”检查，但没有在 MethodDecl 层检查“方法体里是否至少出现一次 return”。

### 3.2 可能要求

非 main 方法必须出现 return 语句，否则语义错误。

### 3.2 需要改的文件

- lib/ast/semantanlyzer.cc

### 代码示例（快速版）

~~~cpp
namespace {

bool contains_return_stm(Stm *s) {
  if (s == nullptr) return false;

  if (dynamic_cast<Return*>(s) != nullptr) return true;

  if (auto n = dynamic_cast<Nested*>(s)) {
    if (n->sl == nullptr) return false;
    for (auto st : *(n->sl)) {
      if (contains_return_stm(st)) return true;
    }
    return false;
  }

  if (auto i = dynamic_cast<If*>(s)) {
    return contains_return_stm(i->stm1) || contains_return_stm(i->stm2);
  }

  if (auto w = dynamic_cast<While*>(s)) {
    return contains_return_stm(w->stm);
  }

  return false;
}

} // namespace

void AST_Semant_Visitor::visit(MethodDecl *node) {
  // 保留你已有的 override / type / body 检查逻辑

  bool has_return = false;
  if (node->sl != nullptr) {
    for (auto s : *(node->sl)) {
      if (contains_return_stm(s)) {
        has_return = true;
        break;
      }
    }
  }

  if (!has_return) {
    semant_error(node->getPos(), "Method " + node->id->id + " must contain at least one return statement");
  }
}
~~~

说明：

1. 这是“至少有一个 return”的检查，不是严格控制流完备（不要求每条路径都 return）。
2. 实验课临时加需求时，老师一般接受这种版本。

---

## 3.3 main 中不允许 return（中概率）

### 3.3 现状

语法里 main 是 MainMethod，不是 MethodDecl。通常不会写 return；但如果 AST 能出现 Return，可能要求显式报错。

### 3.3 需要改的文件

- lib/ast/semantanlyzer.cc

### 代码示例

~~~cpp
void AST_Semant_Visitor::visit(Return *node) {
  if (node == nullptr || node->exp == nullptr)
    return;

  if (current_visiting_class == "__$main__" && current_visiting_method == "main") {
    semant_error(node->getPos(), "Return is not allowed in main method");
  }

  // 保留你已有的返回表达式类型检查
}
~~~

---

## 3.4 setnamemap 侧可能临时要求的扩展（中概率）

这类题一般不是“改遍全工程”，而是让你在建表时多记一类信息，供 semant 用。

### 典型扩展 A：记录 MethodDecl 节点指针

如果老师要你做更复杂的重写检查（比如精确定位父类方法定义位置），可以在 Name_Maps 加一个 methodDeclMap。

### 3.4 需要改的文件

- include/ast/namemaps.hh
- lib/ast/namemaps.cc
- lib/ast/setnamemaps.cc

### 3.4 代码示例

~~~cpp
// include/ast/namemaps.hh
private:
  map<pair<string, string>, MethodDecl*> methodDeclMap;

public:
  bool add_method_decl(string class_name, string method_name, MethodDecl* md);
  MethodDecl* get_method_decl(string class_name, string method_name);
~~~

~~~cpp
// lib/ast/namemaps.cc
bool Name_Maps::add_method_decl(string class_name, string method_name, MethodDecl* md) {
  pair<string, string> key(class_name, method_name);
  if (methodDeclMap.find(key) != methodDeclMap.end()) return false;
  methodDeclMap[key] = md;
  return true;
}

MethodDecl* Name_Maps::get_method_decl(string class_name, string method_name) {
  pair<string, string> key(class_name, method_name);
  auto it = methodDeclMap.find(key);
  if (it == methodDeclMap.end()) return nullptr;
  return it->second;
}
~~~

~~~cpp
// lib/ast/setnamemaps.cc 里 AST_Name_Map_Visitor::visit(MethodDecl*)
if (!name_maps->add_method_decl(current_visiting_class, node->id->id, node)) {
  name_map_error(node->id->getPos(), "Duplicated method declaration node record: " + node->id->id);
}
~~~

说明：

1. 这类改动是“给语义分析补索引能力”，很适合现场临时需求。
2. 不会破坏你现在已有逻辑。

---

## 4. 明天你应该优先改哪些文件

按频率排序：

1. lib/ast/semantanlyzer.cc
2. lib/ast/setnamemaps.cc
3. include/ast/namemaps.hh
4. lib/ast/namemaps.cc
5. test/ 下新增 1-2 个 fmj 用例验证

通常 1 号文件就能完成 70% 以上临时需求。

---

## 5. 现场改题时的最快操作流程（断网可执行）

1. 先读题，把新增规则写成一句 if 条件（谁、在什么场景、必须满足什么类型）
2. 定位 visitor 函数（语句类看 Stm visit，表达式类看 Exp visit）
3. 先写报错分支，再补 setSemant
4. 新建最小 fmj 反例 + 正例各 1 个
5. 运行：

~~~bash
cd HW2
make build
make run-one FILE=your_test_name
~~~

---

## 6. 你可以提前准备的 3 个“万能模板”

### 6.1 模板 A：类型必须是 int

~~~cpp
AST_Semant *s = semant_map->getSemant(node->exp);
if (s == nullptr || s->get_type() != TypeKind::INT) {
  semant_error(node->getPos(), "xxx expects integer expression");
}
~~~

### 6.2 模板 B：必须是类对象

~~~cpp
AST_Semant *obj = semant_map->getSemant(node->obj);
if (obj == nullptr || obj->get_type() != TypeKind::CLASS) {
  semant_error(node->getPos(), "xxx expects class object");
}
~~~

### 6.3 模板 C：赋值兼容

~~~cpp
if (!assign_compatible(name_maps, lhs_semant, rhs_semant)) {
  semant_error(node->getPos(), "type mismatch in assignment");
}
~~~

---

## 7. 明天最该防的两个坑

1. 只做了检查但忘了 setSemant，后续节点会报“no semantic information”。
2. 把本该在 semant 阶段报的错提前到 setnamemap 报，导致行为和现有框架不一致。

---

## 8. 一个简短结论

你现在的 HW2 代码已经具备完整骨架，明天更像“在现有 visitor 的具体 visit 函数里加约束分支”。

最优策略是：

1. 优先改 lib/ast/semantanlyzer.cc。
2. 只有当新规则依赖额外符号索引时，再扩展 Name_Maps + setnamemap。
3. 每加一条规则，立刻配一个最小 fmj 反例验证。

