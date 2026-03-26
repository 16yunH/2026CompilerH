# semant.hh Documentation

This document describes the semantic analysis infrastructure used for type checking and semantic validation in FDMJ2026.

## Overview

The `semant.hh` file provides three main components:
1. **AST_Semant**: Stores semantic information for an AST node (type, kind, lvalue status)
2. **AST_Semant_Map**: Maps AST nodes to their semantic information
3. **AST_Semant_Visitor**: Traverses the AST and performs semantic analysis

These components work together to:
- Perform type checking
- Validate semantic correctness
- Store type information for later phases (IR generation)
- Detect semantic errors

---

## AST_Semant Class

**Purpose**: Represents the semantic information associated with an AST node, primarily for expressions and variables. This information is essential for type checking and intermediate representation (IR) generation.

### Kind Enumeration

```cpp
enum Kind {Value, MethodName, ClassName};
```

- **Value**: Represents a computed value with a specific type (result of any expression)
- **MethodName**: Represents a method name reference (used for method calls)
- **ClassName**: Represents a class name reference (used for object creation and type checking)

### Private Data Members

#### s_kind
```cpp
Kind s_kind;
```
- **Purpose**: Indicates what kind of semantic entity this represents
- **Values**: Value, MethodName, or ClassName

#### typeKind
```cpp
TypeKind typeKind;
```
- **Purpose**: The type of the expression/node
- **Values**: 
  - `TypeKind::INT` - Integer type
  - `TypeKind::ARRAY` - Array type
  - `TypeKind::CLASS` - Class/object type

#### type_par
```cpp
variant<monostate, string, int> type_par;
```
- **Purpose**: Additional type parameter information
- **Values**:
  - `monostate` - No additional information (for INT type)
  - `string` - Class name (for CLASS type)
  - `int` - Array arity (for ARRAY type)

#### lvalue
```cpp
bool lvalue;
```
- **Purpose**: Indicates whether this expression can appear on the left-hand side of an assignment
- **Values**: 
  - `true` - Expression is an lvalue (can be assigned to)
  - `false` - Expression is an rvalue (cannot be assigned to)

---

### Public Methods

#### Constructor
```cpp
AST_Semant(AST_Semant::Kind s_kind, TypeKind typeKind, 
           variant<monostate, string, int> type_par, bool lvalue);
```
- **Purpose**: Creates a semantic information object
- **Parameters**:
  - `s_kind` - The kind of semantic entity
  - `typeKind` - The type of the expression
  - `type_par` - Additional type parameter
  - `lvalue` - Whether this is an lvalue

#### get_kind
```cpp
Kind get_kind();
```
- **Purpose**: Returns the semantic kind
- **Returns**: Value, MethodName, or ClassName

#### get_type
```cpp
TypeKind get_type();
```
- **Purpose**: Returns the type of the expression
- **Returns**: INT, ARRAY, or CLASS

#### get_type_par
```cpp
variant<monostate, string, int> get_type_par();
```
- **Purpose**: Returns additional type parameter information
- **Returns**: 
  - For CLASS type: the class name as string
  - For ARRAY type: the array arity as int
  - For INT type: monostate

#### is_lvalue
```cpp
bool is_lvalue();
```
- **Purpose**: Checks if the expression is an lvalue
- **Returns**: `true` if lvalue, `false` otherwise

#### s_kind_string (Static)
```cpp
static string s_kind_string(Kind s_kind);
```
- **Purpose**: Converts a Kind enum to its string representation
- **Parameters**: `s_kind` - The kind to convert
- **Returns**: String representation ("Value", "MethodName", "ClassName", or "Unknown")

---

## AST_Semant_Map Class

**Purpose**: Maintains a mapping from AST nodes to their semantic information. This allows semantic information to be retrieved for any node during later compilation phases.

### Private Data Members

#### name_maps
```cpp
Name_Maps *name_maps;
```
- **Purpose**: Pointer to the name maps structure (for reference during analysis)

#### semant_map
```cpp
map<AST*, AST_Semant*> semant_map;
```
- **Purpose**: Maps AST node pointers to their semantic information
- **Key**: Pointer to AST node
- **Value**: Pointer to AST_Semant object

---

### Public Methods

#### Constructor
```cpp
AST_Semant_Map();
```
- **Purpose**: Initializes an empty semantic map

#### Destructor
```cpp
~AST_Semant_Map();
```
- **Purpose**: Clears the semantic map

#### getSemant
```cpp
AST_Semant* getSemant(AST *node);
```
- **Purpose**: Retrieves semantic information for a given AST node
- **Parameters**: `node` - Pointer to the AST node
- **Returns**: Pointer to AST_Semant, or `nullptr` if not found or node is null

#### setSemant
```cpp
void setSemant(AST *node, AST_Semant* semant);
```
- **Purpose**: Associates semantic information with an AST node
- **Parameters**:
  - `node` - Pointer to the AST node
  - `semant` - Pointer to the semantic information
- **Side Effect**: Prints error if node is null

#### getNameMaps
```cpp
Name_Maps* getNameMaps();
```
- **Purpose**: Returns the associated name maps structure
- **Returns**: Pointer to Name_Maps

---

## AST_Semant_Visitor Class

**Purpose**: Implements the visitor pattern to traverse the AST and perform semantic analysis. This visitor:
- Performs type checking
- Validates semantic rules
- Populates the AST_Semant_Map with type information
- Reports semantic errors

### Private Data Members

#### semant_map
```cpp
AST_Semant_Map *semant_map;
```
- **Purpose**: The semantic map being populated during traversal
- **Initialized**: In constructor with `new AST_Semant_Map()`

#### name_maps
```cpp
Name_Maps* const name_maps;
```
- **Purpose**: Constant pointer to the name maps (symbol table)
- **Usage**: Used to look up declarations and verify name existence

#### current_visiting_class
```cpp
string current_visiting_class = "";
```
- **Purpose**: Tracks the name of the class currently being analyzed
- **Usage**: Used to resolve names in context of the current class

#### current_visiting_method
```cpp
string current_visiting_method = "";
```
- **Purpose**: Tracks the name of the method currently being analyzed
- **Usage**: Used to resolve local variables and parameters

#### deferred_id
```cpp
set<IdExp*> deferred_id;
```
- **Purpose**: Set of identifier expressions that need deferred resolution
- **Usage**: Used for forward references in some contexts

#### in_a_while_loop
```cpp
int in_a_while_loop = 0;
```
- **Purpose**: Counter for nested while loops
- **Usage**: Used to validate break/continue statements (must be inside a loop)
- **Value**: 0 when not in a loop, >0 when inside one or more nested loops

#### mode
```cpp
int mode;
```
- **Purpose**: Analysis mode flag (for different analysis passes)
- **Usage**: Can be used to control analysis behavior

---

### Public Methods

#### Constructor
```cpp
AST_Semant_Visitor(Name_Maps* const nm);
```
- **Purpose**: Initializes the visitor with a name maps structure
- **Parameters**: `nm` - Constant pointer to the Name_Maps structure
- **Side Effect**: Creates a new AST_Semant_Map

#### getSemantMap
```cpp
AST_Semant_Map* getSemantMap();
```
- **Purpose**: Returns the populated semantic map
- **Returns**: Pointer to the AST_Semant_Map

---

### Visitor Methods

The class implements all visitor methods defined in AST_Visitor. Key methods and their responsibilities:

#### Program Structure Visitors

##### visit(Program*)
- Traverses main method and class declarations
- Initiates semantic analysis for the entire program

##### visit(MainMethod*)
- Analyzes the main method
- Validates variable declarations and statements

##### visit(ClassDecl*)
- **Validates single-level inheritance**: Checks that parent class doesn't already have a parent
- **Detects circular inheritance**: Walks parent chain to detect cycles
- Analyzes all methods in the class

##### visit(MethodDecl*)
- **Validates method signatures**: Checks overriding methods have compatible signatures
- **Supports covariant return types**: Child's return type can be subclass of parent's return type
- Analyzes method body

##### visit(VarDecl*)
- Validates variable declarations
- No semantic analysis needed (handled by name maps)

##### visit(Formal*)
- Validates formal parameters
- No semantic analysis needed

---

#### Statement Visitors

##### visit(If*)
- Validates condition is an expression
- Recursively analyzes then and else branches

##### visit(While*)
- Increments `in_a_while_loop` counter
- Analyzes loop body
- Decrements counter after analysis

##### visit(Assign*)
- **Type checking**: Ensures left and right sides have compatible types
- **Lvalue checking**: Verifies left side is an lvalue
- **Class assignment**: Validates class type compatibility (allows subclass assignment)

##### visit(CallStm*)
- Validates method call statement
- Checks method existence and parameter types

##### visit(Continue*) / visit(Break*)
- **Loop validation**: Checks that `in_a_while_loop > 0`
- Reports error if used outside a loop

##### visit(Return*)
- **Type checking**: Validates return expression matches method return type
- **Class return**: Allows covariant return types

---

#### I/O Statement Visitors

##### visit(PutInt*)
- Validates expression is integer type

##### visit(PutCh*)
- Validates expression is integer type (ASCII value)

##### visit(PutArray*)
- Validates size expression is integer
- Validates array expression is array type

##### visit(GetInt*) / visit(GetCh*) / visit(GetArray*)
- Validates I/O operations
- Sets appropriate semantic information

---

#### Timing Statement Visitors

##### visit(Starttime*) / visit(Stoptime*)
- No semantic validation needed

---

#### Expression Visitors

##### visit(BinaryOp*)
- **Type checking**: Validates operand types match operator requirements
- **Result type**: Determines result type based on operator
- Sets semantic information for the expression

##### visit(UnaryOp*)
- **Type checking**: Validates operand type matches operator
- Sets semantic information for the result

##### visit(ArrayExp*)
- **Array validation**: Ensures base expression is array type
- **Index validation**: Ensures index is integer type
- **Lvalue**: Result is an lvalue
- Sets semantic information with element type

##### visit(CallExp*)
- **Method resolution**: Finds method in class or parent
- **Parameter validation**: Checks argument types match formal parameters
- **Return type**: Sets semantic information with return type

##### visit(ClassVar*)
- **Field resolution**: Looks up field in class or parent class
- **Lvalue**: Result is an lvalue
- Sets semantic information with field type

##### visit(This*)
- **Type**: Sets semantic information as current class type
- **Lvalue**: Not an lvalue

##### visit(Length*)
- **Array validation**: Ensures expression is array type
- **Result type**: Integer
- Sets semantic information

##### visit(NewArray*)
- **Size validation**: Ensures size expression is integer
- **Result type**: Array type
- Sets semantic information with arity 0

##### visit(NewObject*)
- **Class validation**: Ensures class exists
- **Result type**: Class type
- Sets semantic information with class name

##### visit(IdExp*)
- **Name resolution**: Looks up identifier in current scope
  - Checks method variables first
  - Checks method formals
  - Checks class variables (via parent if not found)
- **Lvalue**: Result is an lvalue
- Sets semantic information based on variable type

##### visit(IntExp*)
- **Type**: Always integer
- **Lvalue**: Not an lvalue
- Sets semantic information with INT type

##### visit(OpExp*)
- No semantic analysis needed (operator token)

##### visit(Type*)
- No semantic analysis needed
