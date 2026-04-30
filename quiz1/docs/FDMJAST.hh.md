# FDMJAST.hh Documentation

This document describes all the AST (Abstract Syntax Tree) node classes defined in FDMJAST.hh for the FDMJ2026 language.

## Overview

All AST classes inherit from the base `AST` class and implement:
- **Position tracking**: Each node stores its source code position
- **Clone capability**: Deep copy via `clone()` method
- **Visitor pattern**: `accept()` method for AST traversal
- **Type identification**: `getASTKind()` returns the node's ASTKind

---

## Program Structure Classes

### Program
**Purpose**: Root node representing the entire FDMJ program.

**Members**:
- `MainMethod *main` - The program's entry point (required)
- `vector<ClassDecl*> *cdl` - List of class declarations (optional)

**Constructors**:
- `Program(Pos* pos, MainMethod *main)` - Program with only main method
- `Program(Pos* pos, MainMethod *main, vector<ClassDecl*> *cdl)` - Program with classes

---

### MainMethod
**Purpose**: Represents the main method (program entry point).

**Members**:
- `vector<VarDecl*> *vdl` - Local variable declarations
- `vector<Stm*> *sl` - List of statements to execute

**Note**: The main method is implicit and doesn't have a return type declaration (always returns int).

---

### ClassDecl
**Purpose**: Represents a class declaration.

**Members**:
- `IdExp *id` - Class name identifier
- `IdExp *eid` - Parent class name (nullptr if no inheritance)
- `vector<VarDecl*> *vdl` - Class variable declarations
- `vector<MethodDecl*> *mdl` - Method declarations

**Constructors**:
- Without inheritance: `ClassDecl(Pos *pos, IdExp* id, vector<VarDecl*> *vdl, vector<MethodDecl*> *mdl)`
- With inheritance: `ClassDecl(Pos *pos, IdExp* id, IdExp* eid, vector<VarDecl*> *vdl, vector<MethodDecl*> *mdl)`

**FDMJ2026 Note**: Only single-level inheritance is allowed (a class cannot extend a class that already extends another).

---

## Type and Declaration Classes

### Type
**Purpose**: Represents type information for variables, parameters, and return values.

**Members**:
- `TypeKind typeKind` - Type classification: INT, CLASS, or ARRAY
- `IdExp *cid` - Class identifier (for CLASS type only)
- `IntExp *arity` - Array arity (for ARRAY type only, must be 0)

**Constructors**:
- `Type(Pos *pos)` - Creates INT type
- `Type(Pos *pos, IdExp *cid)` - Creates CLASS type
- `Type(Pos *pos, IntExp *arity)` - Creates ARRAY type
- `Type(Pos *pos, TypeKind typeKind, IdExp *cid, IntExp *arity)` - Full specification

---

### VarDecl
**Purpose**: Represents variable declarations with optional initialization.

**Members**:
- `Type *type` - Variable type
- `IdExp *id` - Variable name
- `variant<monostate, IntExp*, vector<IntExp*>*> init` - Initialization value:
  - `monostate{}` - No initialization
  - `IntExp*` - Integer initialization value
  - `vector<IntExp*>*` - Array initialization values

**Constructors**:
- Without init: `VarDecl(Pos *pos, Type *type, IdExp *id)`
- With int init: `VarDecl(Pos *pos, Type *type, IdExp *id, IntExp *init_int)`
- With array init: `VarDecl(Pos *pos, Type *type, IdExp *id, vector<IntExp*> *init_array)`

---

### MethodDecl
**Purpose**: Represents method declarations within classes.

**Members**:
- `Type *type` - Return type
- `IdExp *id` - Method name
- `vector<Formal*> *fl` - Formal parameters
- `vector<VarDecl*> *vdl` - Local variable declarations
- `vector<Stm*> *sl` - Method body statements

**Constructors** (various combinations of optional components):
- Full: `MethodDecl(Pos *pos, Type *type, IdExp *id, vector<Formal*> *fl, vector<VarDecl*> *vdl, vector<Stm*> *sl)`
- Without formals: `MethodDecl(Pos *pos, Type *type, IdExp *id, vector<VarDecl*> *vdl, vector<Stm*> *sl)`
- Without var decls: `MethodDecl(Pos *pos, Type *type, IdExp *id, vector<Formal*> *fl, vector<Stm*> *sl)`
- Without formals and var decls: `MethodDecl(Pos *pos, Type *type, IdExp *id, vector<Stm*> *sl)`

---

### Formal
**Purpose**: Represents formal parameters in method declarations.

**Members**:
- `Type *type` - Parameter type
- `IdExp *id` - Parameter name

**Constructor**: `Formal(Pos *pos, Type *type, IdExp *id)`

**Validation**: Array types must have arity specified.

---

## Statement Classes

### Stm (Abstract Base)
**Purpose**: Base class for all statement types.

---

### Nested
**Purpose**: Represents a block of statements (statement list).

**Members**:
- `vector<Stm*>* sl` - List of statements

**Constructor**: `Nested(Pos *pos, vector<Stm*>* sl)`

---

### If
**Purpose**: Represents if-else conditional statements.

**Members**:
- `Exp *exp` - Condition expression
- `Stm *stm1` - Then-branch statement
- `Stm *stm2` - Else-branch statement (nullptr if no else)

**Constructors**:
- With else: `If(Pos *pos, Exp *exp, Stm *stm1, Stm *stm2)`
- Without else: `If(Pos *pos, Exp *exp, Stm *stm1)`

---

### While
**Purpose**: Represents while loops.

**Members**:
- `Exp *exp` - Loop condition
- `Stm *stm` - Loop body (nullptr if empty)

**Constructors**:
- With body: `While(Pos *pos, Exp *exp, Stm *stm)`
- Empty body: `While(Pos *pos, Exp *exp)`

---

### Assign
**Purpose**: Represents assignment statements.

**Members**:
- `Exp *left` - Left-hand side (lvalue)
- `Exp *exp` - Right-hand side expression

**Constructor**: `Assign(Pos *pos, Exp *left, Exp *exp)`

---

### CallStm
**Purpose**: Represents method call statements (calls without using return value).

**Members**:
- `Exp *obj` - Object expression (nullptr for static calls)
- `IdExp *name` - Method name
- `vector<Exp*> *par` - Parameter expressions

**Constructors**:
- With parameters: `CallStm(Pos *pos, Exp *obj, IdExp *name, vector<Exp*> *par)`
- Without parameters: `CallStm(Pos *pos, Exp *obj, IdExp *name)`

---

### Continue
**Purpose**: Represents continue statement for loop control.

**Constructor**: `Continue(Pos *pos)`

---

### Break
**Purpose**: Represents break statement for loop control.

**Constructor**: `Break(Pos *pos)`

---

### Return
**Purpose**: Represents return statement.

**Members**:
- `Exp *exp` - Return value expression

**Constructor**: `Return(Pos *pos, Exp *exp)`

---

### PutInt
**Purpose**: Represents putint() statement for integer output.

**Members**:
- `Exp *exp` - Expression to print

**Constructor**: `PutInt(Pos *pos, Exp *exp)`

---

### PutCh
**Purpose**: Represents putch() statement for character output.

**Members**:
- `Exp *exp` - Expression (ASCII value) to print as character

**Constructor**: `PutCh(Pos *pos, Exp *exp)`

---

### PutArray
**Purpose**: Represents putarray() statement for array output.

**Members**:
- `Exp *n` - Number of elements to print
- `Exp *arr` - Array expression

**Constructor**: `PutArray(Pos *pos, Exp *n, Exp *arr)`

---

### Starttime
**Purpose**: Represents starttime() statement for performance timing.

**Constructor**: `Starttime(Pos *pos)`

---

### Stoptime
**Purpose**: Represents stoptime() statement for performance timing.

**Constructor**: `Stoptime(Pos *pos)`

---

## Expression Classes

### Exp (Abstract Base)
**Purpose**: Base class for all expression types.

---

### BinaryOp
**Purpose**: Represents binary operations (+, -, *, /, &&, ||, <, >, <=, >=, ==, !=).

**Members**:
- `Exp *left` - Left operand
- `OpExp *op` - Operator
- `Exp *right` - Right operand

**Constructor**: `BinaryOp(Pos *pos, Exp *left, OpExp *op, Exp *right)`

---

### UnaryOp
**Purpose**: Represents unary operations (-, !).

**Members**:
- `OpExp *op` - Operator
- `Exp *exp` - Operand expression

**Constructor**: `UnaryOp(Pos *pos, OpExp *op, Exp *exp)`

---

### ArrayExp
**Purpose**: Represents array element access.

**Members**:
- `Exp *arr` - Array expression
- `Exp *index` - Index expression

**Constructor**: `ArrayExp(Pos *pos, Exp *arr, Exp *index)`

---

### CallExp
**Purpose**: Represents method call expressions (using return value).

**Members**:
- `Exp *obj` - Object expression
- `IdExp *name` - Method name
- `vector<Exp*> *par` - Parameter expressions

**Constructors**:
- With parameters: `CallExp(Pos *pos, Exp *obj, IdExp *name, vector<Exp*> *par)`
- Without parameters: `CallExp(Pos *pos, Exp *obj, IdExp *name)`

---

### ClassVar
**Purpose**: Represents class variable access (obj.field).

**Members**:
- `Exp *obj` - Object expression
- `IdExp *id` - Field name

**Constructor**: `ClassVar(Pos *pos, Exp *obj, IdExp *id)`

---

### This
**Purpose**: Represents 'this' reference for current object.

**Constructor**: `This(Pos *pos)`

---

### Length
**Purpose**: Represents length() expression for arrays.

**Members**:
- `Exp *exp` - Array expression

**Constructor**: `Length(Pos *pos, Exp *exp)`

---

### NewArray
**Purpose**: Represents new int[] expression for array creation.

**Members**:
- `Exp *size` - Array size expression

**Constructor**: `NewArray(Pos *pos, Exp *size)`

**FDMJ2026 Note**: Array creation uses `new int[size]` syntax.

---

### NewObject
**Purpose**: Represents new Class() expression for object creation.

**Members**:
- `IdExp *id` - Class name to instantiate

**Constructor**: `NewObject(Pos *pos, IdExp *id)`

**FDMJ2026 Note**: Object creation requires `new` keyword with class name.

---

### GetInt
**Purpose**: Represents getint() expression for integer input.

**Constructor**: `GetInt(Pos *pos)`

---

### GetCh
**Purpose**: Represents getch() expression for character input.

**Constructor**: `GetCh(Pos *pos)`

---

### GetArray
**Purpose**: Represents getarray() expression for array input.

**Members**:
- `Exp *exp` - Array expression to fill

**Constructor**: `GetArray(Pos *pos, Exp *exp)`

---

### IdExp
**Purpose**: Represents identifier expressions (variable/method/class names).

**Members**:
- `string id` - The identifier string

**Constructor**: `IdExp(Pos *pos, string id)`

---

### IntExp
**Purpose**: Represents integer literal expressions.

**Members**:
- `int val` - The integer value

**Constructor**: `IntExp(Pos *pos, int val)`

---

### OpExp
**Purpose**: Represents operator tokens in expressions.

**Members**:
- `string op` - The operator string (+, -, *, /, &&, ||, etc.)

**Constructor**: `OpExp(Pos *pos, string op)`
