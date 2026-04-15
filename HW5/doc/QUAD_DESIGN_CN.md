# Quad中间表示设计

## 目录
1. [概述](#概述)
2. [架构](#架构)
3. [数据模型](#数据模型)
4. [Quad指令类型](#quad指令类型)
5. [类型系统](#类型系统)
6. [数据流分析](#数据流分析)
7. [内存管理](#内存管理)
8. [使用示例](#使用示例)
9. [设计原理](#设计原理)
10. [已知问题](#已知问题)

---

## 概述

### 目的
**Quad** (四元组) IR 是第二层三地址码表示，从 Tiger IR+（树形结构，位于 `treep.hh`）翻译而来。它将 Tiger IR+ 简化为适合代码生成、优化和活跃性分析的基本操作。

### 主要特征
- **三地址码**：大多数指令最多有 3 个操作数（目标、源1、源2）
- **平坦结构**：基本块内语句的线性序列
- **类型感知**：用 INT 和 PTR 类型跟踪中间值
- **SSA 兼容**：支持 Phi 节点用于数据流路径
- **访问者模式**：用于扩展设计的通过和转换

### 编译管道
```
源代码 (.fmj)
    ↓
词法/语法分析器 → AST
    ↓
语义分析 → Tiger IR+（Tree, treep.hh）[第一层 IR]
    ↓
[Tree2Quad 访问者]
    ↓
Quad IR（第二层 IR，三地址码）
    ↓
活跃性分析（def/use 计算）
    ↓
代码生成
```

---

## 架构

### 类层次结构

```
Quad (基类，抽象)
├── QuadProgram           (函数容器)
├── QuadFuncDecl          (块容器，代表一个函数)
├── QuadBlock             (语句容器，代表一个基本块)
└── QuadStm (扩展 Quad)   (所有语句的抽象基类)
    ├── QuadLabel         (跳转目标标签)
    ├── QuadJump          (无条件分支)
    ├── QuadCJump         (条件分支)
    ├── QuadMove          (寄存器赋值)
    ├── QuadLoad          (内存读取)
    ├── QuadStore         (内存写入)
    ├── QuadMoveBinop     (二元操作 + 赋值)
    ├── QuadPtrCalc       (指针算术)
    ├── QuadCall          (函数调用，忽略结果)
    ├── QuadMoveCall      (函数调用并赋值结果)
    ├── QuadExtCall       (外部 C 函数调用，忽略结果)
    ├── QuadMoveExtCall   (外部 C 函数调用并赋值结果)
    ├── QuadPhi           (SSA Phi 指令)
    └── QuadReturn        (函数返回)
```

### 访问者模式
所有 Quad 节点实现用于可扩展分析的访问者模式：
```cpp
class QuadVisitor {
    virtual void visit(QuadMove *quad) = 0;
    virtual void visit(QuadLoad *quad) = 0;
    // ... 其他访问方法
};
```

代码库中的常见访问者：
- **Quad2Xml**：将 Quad IR 转换为 XML 表示
- **LiveInOut**：计算活跃变量集合
- 用于优化和分析的自定义通过

---

## 数据模型

### 核心组件

#### 1. QuadTerm - 操作数
表示 Quad 指令中的操作数（终结符）。可以是三种类型之一：

```cpp
enum class QuadTermKind { 
    TEMP,    // 临时变量 (t100, t101, ...)
    CONST,   // 常数值 (1, 0, 42, ...)
    NAME     // 命名引用 (标签、方法名...)
};

class QuadTerm {
public:
    QuadTermKind kind;
    variant<monostate, QuadTemp*, int, string> term;
    
    QuadTerm(QuadTemp *quadtemp);  // 临时变量
    QuadTerm(int constant);         // 常数
    QuadTerm(string name);          // 名称
};
```

**QuadTemp** - 类型化临时变量：
```cpp
class QuadTemp {
public:
    Temp *temp;        // 对临时变量的引用
    QuadType type;     // INT 或 PTR
};
```

#### 2. QuadType - 类型系统
```cpp
enum class QuadType {
    INT,   // 整数类型 (32位有符号)
    PTR    // 指针类型 (内存地址)
};
```

**类型重要性：**
- INT：用于整数计算和算术
- PTR：用于对象引用、数组引用、内存位置
- 类型不匹配表示代码生成中的错误
- 影响寄存器分配和指令选择

#### 3. Def/Use 集合 - 数据流信息
```cpp
class QuadStm : public Quad {
public:
    set<Temp*> *def;  // 由此语句定义（修改）的临时变量
    set<Temp*> *use;  // 由此语句读取的临时变量
};
```

**动机：**
- 对活跃变量分析至关重要
- 启用死代码消除
- 寄存器分配所需
- 支持数据依赖分析

---

## Quad 指令类型

### 1. MOVE - 简单赋值
**语法：** `temp ← term`

```cpp
class QuadMove : public QuadStm {
public:
    QuadTemp *dst;      // 目标临时变量
    QuadTerm *src;      // 源（临时变量、常数或名称）
};
```

**示例：**
```
MOVE t100:int <- Const:42;           // 赋值常数
MOVE t101:ptr <- t100:ptr;           // 复制值
MOVE t102:int <- Name:array_length;  // 赋值命名值
```

**Def/Use：**
- `def = {t100}`
- `use = {t101 如果 src 是临时}`


### 2. LOAD - 内存读取
**语法：** `temp ← mem(term)`

```cpp
class QuadLoad : public QuadStm {
public:
    QuadTemp *dst;      // 目标临时变量（保存加载值）
    QuadTerm *src;      // 源地址（必须是指针）
};
```

**示例：**
```
LOAD t100:int <- Mem(t101:ptr);      // 从指针加载
LOAD t102:ptr <- Mem(t100:ptr+4);    // 从偏移加载
```

**Def/Use：**
- `def = {t100}`
- `use = {src 如果是临时}`

**内存语义：**
- 假设内存按字节寻址
- 源必须计算为指针地址
- 不同于用于地址计算的 PTR_CALC


### 3. STORE - 内存写入
**语法：** `mem(term) ← term`

```cpp
class QuadStore : public QuadStm {
public:
    QuadTerm *dst;      // 目标地址
    QuadTerm *src;      // 源值（要存储的内容）
};
```

**示例：**
```
STORE Const:1 -> Mem(t100:ptr);      // 存储常数到内存
STORE t102:int -> Mem(t100:ptr+8);   // 存储临时到偏移
```

**Def/Use：**
- `def = {}` (空 - 修改内存，不修改临时)
- `use = {dst_temp, src_temp 如果是临时}`

**重要：** Store 不定义任何临时，只修改内存！


### 4. MOVE_BINOP - 二元操作 + 赋值
**语法：** `temp ← term op term`

```cpp
class QuadMoveBinop : public QuadStm {
public:
    QuadTemp *dst;      // 目标临时变量
    QuadTerm *left;     // 左操作数
    QuadTerm *right;    // 右操作数
    BinOp op;           // 操作符 (+, -, *, /, %, <, >, ==, 等)
};

enum class BinOp { 
    PLUS, MINUS, MULT, DIV, MOD,           // 算术
    LT, GT, LE, GE, EQ, NE,                // 比较
    AND, OR, LSHIFT, RSHIFT                // 位操作
};
```

**示例：**
```
MOVE_BINOP t100:int <- (+, t101:int, Const:1);     // 加 1
MOVE_BINOP t102:ptr <- (+, t100:ptr, Const:4);     // 指针算术
MOVE_BINOP t103:int <- (<, t100:int, t101:int);    // 比较 (结果: 0/1)
```

**Def/Use：**
- `def = {t100}`
- `use = {left_temp, right_temp 如果是临时}`


### 5. PTR_CALC - 指针算术
**语法：** `temp ← term + term` (专用于指针算术)

```cpp
class QuadPtrCalc : public QuadStm {
public:
    QuadTemp *dst;      // 目标指针临时变量
    QuadTerm *ptr;      // 基指针
    QuadTerm *offset;   // 偏移值
};
```

**示例：**
```
PTR_CALC t100:ptr <- t101:ptr + Const:4;    // 加 4 字节
PTR_CALC t102:ptr <- t100:ptr + t103:int;   // 加可变偏移
```

**Def/Use：**
- `def = {t100}`
- `use = {t101, t103 如果是临时}`

**设计注：**
- 从 MOVE_BINOP 分离以获得清晰性和优化
- 假设指针语义（按元素大小缩放）
- 第一个操作数必须是指针类型
- 第二个操作数确定偏移


### 6. LABEL - 跳转目标
**语法：** `Label:123`

```cpp
class QuadLabel : public QuadStm {
public:
    string name;        // 标签名称/编号（通常是数字）
};
```

**示例：**
```
LABEL L100;   // main 的入口点
LABEL L105;   // 循环头
LABEL L110;   // if-else 后的块
```

**Def/Use：**
- `def = {}` (空)
- `use = {}` (空)

**目的：**
- 标记用于跳转的程序点
- 用于控制流分析
- 无实际计算


### 7. JUMP - 无条件分支
**语法：** `jump label`

```cpp
class QuadJump : public QuadStm {
public:
    string target;      // 目标标签名称/编号
};
```

**示例：**
```
JUMP L105;     // 跳转到循环头
JUMP L104;     // 跳转到退出块
```

**Def/Use：**
- `def = {}` (空)
- `use = {}` (空)

**控制流：**
- 始终将执行转移到目标标签
- 用于 if-else 分支后连接控制流
- 用于循环控制


### 8. CJUMP - 条件分支
**语法：** `cjump relop left, right, true_label, false_label`

```cpp
class QuadCJump : public QuadStm {
public:
    RelOp relop;        // 比较操作符
    QuadTerm *left;     // 左操作数
    QuadTerm *right;    // 右操作数
    string true_label;  // 条件为真时的目标
    string false_label; // 条件为假时的目标
};

enum class RelOp { 
    LT,  GT,  LE,  GE,  EQ,  NE,     // 所有比较操作
};
```

**示例：**
```
CJUMP < t100:int Const:0? L105 : L104;    // 如果 t100 < 0 跳转 L105 否则 L104
CJUMP == t101:int Const:0? L106 : L107;   // 如果 t101 == 0 跳转 L106 否则 L107
CJUMP != t102:int Const:0? L103 : L104;   // 如果 t102 != 0 跳转 L103 否则 L104
```

**Def/Use：**
- `def = {}` (空)
- `use = {left_temp, right_temp 如果是临时}`

**重要提示：**
- 两个比较值都必须可计算（临时或常数）
- 布尔结果隐式：真分支对假分支
- 编译器生成适当的条件跳转指令


### 9. CALL - 函数调用（忽略结果）
**语法：** `call(obj, arg1, arg2, ...)`

```cpp
class QuadCall : public QuadStm {
public:
    string name;            // 函数名
    QuadTerm *obj_term;     // 对象引用（用于方法调用）
    vector<QuadTerm*> *args;// 参数列表
};
```

**示例：**
```
CALL bubbleSort[t113:ptr] (t112:ptr, t111:ptr, t105:int);
```

**Def/Use：**
- `def = {}` (结果忽略)
- `use = {obj_temp, arg_temps}`

**上下文：**
- 用于函数调用结果被丢弃的情况
- 当结果被使用时包装在 QuadMoveCall 中


### 10. MOVE_CALL - 函数调用并赋值结果
**语法：** `temp ← call(...)`

```cpp
class QuadMoveCall : public QuadStm {
public:
    QuadTemp *dst;      // 返回值的目标
    QuadCall *call;     // 实际调用
};
```

**示例：**
```
MOVE_CALL t108:int <- f[t107:ptr] (t100:ptr);
```

**Def/Use：**
- `def = {t108}` (目标临时)
- `use = {call->use}` (调用中的所有操作数)

**方法调用分派：**
- `t107:ptr` 是指向方法表的指针（虚拟分派）
- `t100:ptr` 是对象（this 指针）


### 11. EXTCALL - 外部 C 函数调用（忽略结果）
**语法：** `extcall function_name(arg1, arg2, ...)`

```cpp
class QuadExtCall : public QuadStm {
public:
    string extfun;               // 外部函数名
    vector<QuadTerm*> *args;    // 参数列表
};
```

**示例：**
```
EXTCALL putint(t100:int);
EXTCALL exit(Const:-1);
EXTCALL malloc(Const:32);
```

**Def/Use：**
- `def = {}` (结果忽略)
- `use = {arg_temps}`

**常见外部函数：**
- `putint()` - 打印整数
- `putch()` - 打印字符
- `getint()` - 读取整数
- `malloc()` - 分配内存
- `exit()` - 终止程序


### 12. MOVE_EXTCALL - 外部函数调用并赋值结果
**语法：** `temp ← extcall(...)`

```cpp
class QuadMoveExtCall : public QuadStm {
public:
    QuadTemp *dst;          // 返回值的目标
    QuadExtCall *extcall;   // 实际调用
};
```

**示例：**
```
MOVE_EXTCALL t100:ptr <- malloc(Const:32);    // 分配 32 字节
MOVE_EXTCALL t104:int <- getint();             // 从输入读取整数
```

**Def/Use：**
- `def = {t100}` (目标临时)
- `use = {extcall->use}` (所有参数操作数)


### 13. PHI - SSA Phi 指令
**语法：** `temp ← φ(temp@label1, temp@label2, ...)`

```cpp
class QuadPhi : public QuadStm {
public:
    QuadTemp *dst;              // 目标临时
    vector<QuadPhi_in*> *phi_in;// (临时, 标签) 对的列表
};

struct QuadPhi_in {
    QuadTemp *temp;     // 来自该路径的源临时
    string label;       // 来自哪个基本块
};
```

**示例：**
```
PHI t100:int <- {t101@L100, t102@L101};
```

**Def/Use：**
- `def = {t100}`
- `use = phi_in 列表中的所有源临时`

**目的：**
- 用于 SSA 形式中有多个定义的变量
- 编译器根据控制流路径选择适当的源
- 每个标签指示哪个前驱块处于活动


### 14. RETURN - 函数返回
**语法：** `return term`

```cpp
class QuadReturn : public QuadStm {
public:
    QuadTerm *value;    // 要返回的值（如果有）
};
```

**示例：**
```
RETURN t100:int;       // 返回 t100 中的值
RETURN Const:0;        // 返回常数
```

**Def/Use：**
- `def = {}` (空)
- `use = {value_temp 如果是临时}`

**调用约定：**
- 返回值必须放在标准返回位置
- 实际指令取决于后端架构


---

## 类型系统

### 两层类型表示

#### 1. QuadType - 简单类型分类
```cpp
enum class QuadType {
    INT,   // 32位有符号整数
    PTR    // 指针（内存地址）
};
```

#### 2. 临时变量中的类型跟踪
每个临时变量携带一个限定类型：
```cpp
class QuadTemp {
    Temp *temp;         // 唯一标识符
    QuadType type;      // INT 或 PTR
};
```

### 类型推理规则

**来自源树：**
- INT：算术操作、比较、函数参数
- PTR：对象引用、数组引用、malloc 结果

**类型无关操作：**
- LABEL、JUMP - 无类型信息
- CJUMP relop - 在比较站点检查类型
- CALL/EXTCALL - 来自函数签名的类型

**类型转换：**
- 通常不在 Quad IR 中显式
- 后端负责适当的指令选择
- 类型不匹配表示编译器错误

### 类型安全

**违规表示错误：**
```
PTR_CALC t100:ptr <- (+, t101:int, t102:int);  // 错误：基类型错误
LOAD t100:int <- Mem(t101:int);                // 错误：地址必须是 PTR
STORE t100:ptr -> Mem(t101:int);               // 错误：地址必须是 PTR
```

---

## 数据流分析

### 活跃性分析 - 计算 IN/OUT 集合

**目标：** 确定在每个程序点哪些临时变量是"活跃的"（对未来使用需要）。

**后向数据流分析算法：**
```
对于每个基本块（逆序）：
    对于每个语句（逆序）：
        OUT[s] = ∪ IN[后继]
        IN[s]  = use[s] ∪ (OUT[s] - def[s])
```

**示例：**
```
L100: MOVE t100:int <- Const:1
      MOVE t101:int <- Const:2
      CJUMP < t100:int t101:int? L103 : L104

L103: MOVE_BINOP t102:int <- (+, t100:int, t101:int)    // 两者 IN
      EXTCALL putint(t102:int)
      JUMP L105

L104: ...
```

在 L103 语句 `MOVE_BINOP` 处：
- `use({t100, t101})`
- `def({t102})`
- `IN = {t100, t101}`

### Def/Use 集合语义

**定义 (def)：**
- 临时变量被**写入**（修改）
- 后续使用看到此新值
- 只有从定义的前向执行才能看到效果

**使用 (use)：**
- 临时变量被**读取**（使用当前值）
- 必须被之前定义
- 活跃性从使用处向后流动

**内存区分：**
- STORE 不定义临时（修改内存，不修改临时）
- LOAD 定义目标临时，使用源地址

### 关键设计决策

**为什么分离 def/use 集合？**
1. **寄存器分配**需要知道哪些值必须在寄存器中存活
2. **死代码消除**移除没有活跃使用的语句
3. **CSE（公共子表达式消除）**避免重新计算值
4. **溢出**通过移除不必要的活跃来最小化寄存器压力
5. **复制传播**传播单一赋值值

---

## 内存管理

### 指针所有权模型
- **QuadProgram** 拥有顶层函数列表
- **QuadFuncDecl** 拥有块
- **QuadBlock** 拥有语句
- **语句**拥有其操作数（QuadTemp、QuadTerm）

### 清理责任
Quad IR 的用户必须管理清理：
```cpp
QuadProgram* quad_program = tree_to_quad_visitor.get_quad();
// ... 使用 quad_program ...
// 最终：
delete quad_program;  // 递归删除所有子节点
```

### 克隆
在所有级别都可用克隆操作：
```cpp
QuadProgram* copy = original->clone();
QuadFuncDecl* func_copy = func->clone();
QuadTerm* term_copy = term->clone();
```

---

## 使用示例

### 示例 1：简单赋值
**源（irtest1.fmj）：**
```java
public int main() {
    int i;
    i = 1;
    putint(i);
    putch(10);
    return i;
}
```

**Quad 翻译：**
```
Function __$main__^main() last_label=100 last_temp=105:
  Block: Entry Label: L100
    LABEL L100
    MOVE t100:int <- Const:1          def:{t100} use:{}
    EXTCALL putint(t100:int)          def:{}     use:{t100}
    EXTCALL putch(Const:10)           def:{}     use:{}
    RETURN t100:int                   def:{}     use:{t100}
```

### 示例 2：带条件的循环
**源（irtest3.fmj）：**
```java
public int main() {
    int i;
    i = 1;
    while (i>0) {
      putint(i);
      putch(10);
      i = i-1;
    }
    return i;
}
```

**Quad 翻译：**
```
Function __$main__^main():
  Block: Entry Label: L105
    LABEL L105
    MOVE t100:int <- Const:1                      def:{t100} use:{}
    
  Block: Loop Header Label: L102
    LABEL L102
    CJUMP > t100:int Const:0? L103 : L104        def:{}     use:{t100}
    
  Block: Loop Body Label: L103
    LABEL L103
    EXTCALL putint(t100:int)                     def:{}     use:{t100}
    EXTCALL putch(Const:10)                      def:{}     use:{}
    MOVE_BINOP t100:int <- (-, t100:int, Const:1) def:{t100} use:{t100}
    JUMP L102                                    def:{}     use:{}
    
  Block: After Loop Label: L104
    LABEL L104
    RETURN t100:int                              def:{}     use:{t100}
```

**控制流图：**
```
L105 → L102 (入口)
L102 → [真: L103, 假: L104]
L103 → L102 (循环回)
L104 → 退出
```

**活跃性分析：**
```
L104: IN={t100}, OUT={}      // t100 需要用于返回
L102: IN={t100}, OUT={t100}  // t100 需要用于两个分支
L103: IN={t100}, OUT={t100}  // t100 被使用和重新定义
```

### 示例 3：对象和方法调用
**源（irtest22.fmj - 斐波那契）：**
```java
public int main() {
  class fib o;
  int i;
  i = 0;
  o = new fib();
  // ... putch 调用打印 "Enter the number of term: " ...
  
  while (i < n) {
      putint(o.f(i));      // 在对象上调用方法
      i = i + 1;
  }
}

public class fib {
  public int f(int n) {
    if (n == 0 || n == 1)
      return n;
    else
      return this.f(n - 1) + this.f(n - 2);
  }
}
```

**Quad 翻译（摘录）：**
```
Function __$main__^main():
  Block: 对象创建
    LABEL L113
    MOVE t102:ptr <- Const:0                      // o = null
    MOVE t100:int <- Const:0                      // i = 0
    MOVE_EXTCALL t104:ptr <- malloc(Const:4)      // 分配对象
    PTR_CALC t138:ptr <- t104:ptr + Const:0       // 方法表偏移
    STORE Name:fib^f -> Mem(t138:ptr)             // 存储方法指针
    MOVE t102:ptr <- t104:ptr                      // o = 已分配对象

  Block: 循环
    LABEL L110
    CJUMP < t100:int t101:int? L111 : L112       // while i < n
    
  Block: 循环体
    LABEL L111
    PTR_CALC t166:ptr <- t102:ptr + Const:0      // 获取方法表
    LOAD t132:ptr <- Mem(t166:ptr)                // 加载方法指针
    MOVE_CALL t133:int <- f[t132:ptr] (t102:ptr, t100:int)  // 调用 o.f(i)
    EXTCALL putint(t133:int)
    MOVE_BINOP t100:int <- (+, t100:int, Const:1) // i = i + 1
    JUMP L110                                     // 循环回
```

### 示例 4：带边界检查的数组操作
**源（irtest21.fmj - 冒泡排序）：**
```java
public int main() {
    int[] a = {6, 3, 0, 5, 9, 1, 2};
    // ... 访问 a[i] ...
}
```

**Quad 翻译（摘录）：**
```
Function __$main__^main():
  Block: 数组初始化
    LABEL L108
    MOVE_EXTCALL t100:ptr <- malloc(Const:32)    // 分配 7*4+4 字节
    STORE Const:7 -> Mem(t100:ptr)                // 存储长度
    PTR_CALC t123:ptr <- t100:ptr + Const:4
    STORE Const:6 -> Mem(t123:ptr)                // a[0] = 6
    PTR_CALC t124:ptr <- t100:ptr + Const:8
    STORE Const:3 -> Mem(t124:ptr)                // a[1] = 3
    // ... 更多元素 ...

  Block: 带边界检查的数组访问
    LABEL L103
    MOVE t115:ptr <- t100:ptr                    // 获取数组指针
    LOAD t107:int <- Mem(t100:ptr)               // 加载长度
    CJUMP >= t102:int Const:0? L106 : L105      // 检查下界
    
    LABEL L106
    CJUMP >= t102:int t107:int? L105 : L107     // 检查上界
    
    LABEL L105
    EXTCALL exit(Const:-1)                       // 边界错误时退出
    
    LABEL L107
    MOVE_BINOP t135:int <- (+, t102:int, Const:1) // 计算偏移
    MOVE_BINOP t136:int <- (*, t135:int, Const:4) // * 4 元素大小
    PTR_CALC t137:ptr <- t115:ptr + t136:int     // 计算元素地址
    LOAD t118:int <- Mem(t137:ptr)               // 加载元素
```

---

## 设计原理

### 1. 三地址码模型
**为什么不用两地址码（如 x86）？**
- 让源和目标分离
- 更清晰的语义（读 vs 写）
- 对多操作数指令更具扩展性
- 有利于分析和优化

### 2. 分离的指令类型
**为什么是 MOVE_BINOP 而不仅仅是 MOVE？**
- 区分计算和复制
- 启用代码生成中的模式匹配
- 简化优化通过
- 对交换操作进行不同处理

### 3. QuadTerm 变体设计
**为什么使用 std::variant 而不是单独的类？**
- 紧凑的表示
- 不同类型的单一操作数槽
- 不需要空指针
- 编译时类型安全

### 4. Def/Use 集合
**为什么不按需计算？**
- 活跃性分析需要后向遍历
- 缓存 def/use 提高性能
- 迭代数据流算法所必需
- 启用 SSA 转换（Phi 插入）

### 5. 分离 PTR_CALC
**为什么不仅使用 MOVE_BINOP 进行指针算术？**
- 区分地址计算和算术
- 启用指针特定优化
- 清晰的标量对指针操作语义
- 便于数组边界检查

### 6. Phi 指令
**为什么 Phi 与其他指令处理不同？**
- SSA 形式要求
- 不同的语义（多个定义对一个定义）
- 有助于寄存器分配
- 启用复杂的数据流分析

---


## 总结

Quad IR 是一个结构良好的三地址码表示，适合于：
- **数据流分析**（活跃性、到达定义）
- **代码生成**（模式匹配到汇编）
- **优化**（死代码消除、CSE）
- **程序验证**（类型检查、边界分析）

主要优势：
- 清晰的 def/use 语义
- 类型感知表示
- 可扩展的访问者模式
- SSA 形式支持

已知弱点：
- 类型限制（无 FLOAT、STRUCT）
- 缺少调用约定信息
- 无异常处理

尽管有这些限制，IR 成功处理了广泛范围的面向对象语言功能，包括继承、方法分派、数组操作和递归调用。