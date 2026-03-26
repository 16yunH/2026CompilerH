# namemaps.hh Documentation

This document describes the name mapping infrastructure used for semantic analysis in FDMJ2026.

## Overview

The `namemaps.hh` file provides two main components:
1. **Name_Maps**: A data structure that stores all name declarations and their relationships in a program
2. **AST_Name_Map_Visitor**: A visitor that traverses the AST and populates the Name_Maps structure

These components are essential for:
- Symbol table management
- Name resolution during semantic analysis
- Tracking class hierarchies and method signatures

---

## Name_Maps Class

**Purpose**: Maintains comprehensive mappings of all names (classes, methods, variables) and their relationships within a program. This class serves as a central repository for declarations, facilitating easy access during compilation phases.

### Private Data Members

#### classes
```cpp
set<string> classes;
```
- **Purpose**: Set of all class names declared in the program
- **Usage**: Used to check if a class exists before referencing it

#### classHierachy
```cpp
map<string, string> classHierachy;
```
- **Purpose**: Maps each class name to its direct parent class name
- **Key**: Child class name
- **Value**: Parent class name
- **Note**: FDMJ2026 only allows single-level inheritance

#### methods
```cpp
set<pair<string, string>> methods;
```
- **Purpose**: Set of (class_name, method_name) pairs representing all methods
- **Usage**: Used to check if a method exists in a specific class

#### classVar
```cpp
map<pair<string, string>, VarDecl*> classVar;
```
- **Purpose**: Maps (class_name, var_name) to variable declaration AST nodes
- **Key**: Pair of (class_name, variable_name)
- **Value**: Pointer to VarDecl AST node containing type information

#### methodVar
```cpp
map<tuple<string, string, string>, VarDecl*> methodVar;
```
- **Purpose**: Maps (class_name, method_name, var_name) to local variable declarations
- **Key**: Tuple of (class_name, method_name, variable_name)
- **Value**: Pointer to VarDecl AST node

#### methodFormal
```cpp
map<tuple<string, string, string>, Formal*> methodFormal;
```
- **Purpose**: Maps (class_name, method_name, formal_name) to formal parameter declarations
- **Key**: Tuple of (class_name, method_name, parameter_name)
- **Value**: Pointer to Formal AST node

#### methodFormalList
```cpp
map<pair<string, string>, vector<string>> methodFormalList;
```
- **Purpose**: Maps (class_name, method_name) to ordered list of formal parameter names
- **Key**: Pair of (class_name, method_name)
- **Value**: Vector of parameter names in declaration order
- **Note**: The last element in the vector represents the return type (stored as a Formal)

---

### Public Methods

#### Class Management

##### is_class
```cpp
bool is_class(string class_name);
```
- **Purpose**: Checks if a class with the given name exists
- **Parameters**: `class_name` - Name of the class to check
- **Returns**: `true` if class exists, `false` otherwise

##### add_class
```cpp
bool add_class(string class_name);
```
- **Purpose**: Adds a new class to the class set
- **Parameters**: `class_name` - Name of the class to add
- **Returns**: `true` if successfully added, `false` if class already exists

##### add_class_hiearchy
```cpp
bool add_class_hiearchy(string class_name, string parent_name);
```
- **Purpose**: Records the inheritance relationship between two classes
- **Parameters**: 
  - `class_name` - Child class name
  - `parent_name` - Parent class name
- **Returns**: `false` if either class doesn't exist, `true` otherwise
- **Note**: Does not check for circular inheritance (done in semantic analyzer)

##### get_parent
```cpp
string get_parent(string class_name);
```
- **Purpose**: Returns the parent class name for a given class
- **Parameters**: `class_name` - Name of the class
- **Returns**: Parent class name, or empty string if class has no parent
- **FDMJ2026**: Only single-level inheritance allowed

---

#### Method Management

##### is_method
```cpp
bool is_method(string class_name, string method_name);
```
- **Purpose**: Checks if a method exists in a specific class
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
- **Returns**: `true` if method exists, `false` otherwise

##### add_method
```cpp
bool add_method(string class_name, string method_name);
```
- **Purpose**: Adds a method to a class's method set
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
- **Returns**: `true` if successfully added, `false` if method already exists

---

#### Class Variable Management

##### is_class_var
```cpp
bool is_class_var(string class_name, string var_name);
```
- **Purpose**: Checks if a class has a variable with the given name
- **Parameters**:
  - `class_name` - Name of the class
  - `var_name` - Name of the variable
- **Returns**: `true` if variable exists, `false` otherwise

##### add_class_var
```cpp
bool add_class_var(string class_name, string var_name, VarDecl* vd);
```
- **Purpose**: Records a class variable declaration
- **Parameters**:
  - `class_name` - Name of the class
  - `var_name` - Name of the variable
  - `vd` - Pointer to VarDecl AST node
- **Returns**: `true` if successfully added, `false` if variable already exists

##### get_class_var
```cpp
VarDecl* get_class_var(string class_name, string var_name);
```
- **Purpose**: Retrieves the declaration of a class variable
- **Parameters**:
  - `class_name` - Name of the class
  - `var_name` - Name of the variable
- **Returns**: Pointer to VarDecl node, or `nullptr` if not found

---

#### Method Variable Management

##### is_method_var
```cpp
bool is_method_var(string class_name, string method_name, string var_name);
```
- **Purpose**: Checks if a local variable exists in a method
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `var_name` - Name of the variable
- **Returns**: `true` if variable exists, `false` otherwise

##### add_method_var
```cpp
bool add_method_var(string class_name, string method_name, string var_name, VarDecl* vd);
```
- **Purpose**: Records a method local variable declaration
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `var_name` - Name of the variable
  - `vd` - Pointer to VarDecl AST node
- **Returns**: `true` if successfully added, `false` if variable already exists

##### get_method_var
```cpp
VarDecl* get_method_var(string class_name, string method_name, string var_name);
```
- **Purpose**: Retrieves the declaration of a method local variable
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `var_name` - Name of the variable
- **Returns**: Pointer to VarDecl node, or `nullptr` if not found

---

#### Formal Parameter Management

##### is_method_formal
```cpp
bool is_method_formal(string class_name, string method_name, string var_name);
```
- **Purpose**: Checks if a formal parameter exists in a method
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `var_name` - Name of the parameter
- **Returns**: `true` if parameter exists, `false` otherwise

##### add_method_formal
```cpp
bool add_method_formal(string class_name, string method_name, string var_name, Formal* f);
```
- **Purpose**: Records a method formal parameter declaration
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `var_name` - Name of the parameter
  - `f` - Pointer to Formal AST node
- **Returns**: `true` if successfully added, `false` if parameter already exists

##### get_method_formal
```cpp
Formal* get_method_formal(string class_name, string method_name, string var_name);
```
- **Purpose**: Retrieves the declaration of a method formal parameter
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `var_name` - Name of the parameter
- **Returns**: Pointer to Formal node, or `nullptr` if not found

##### add_method_formal_list
```cpp
bool add_method_formal_list(string class_name, string method_name, vector<string> vl);
```
- **Purpose**: Records the ordered list of formal parameters for a method
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
  - `vl` - Vector of parameter names in declaration order
- **Returns**: `true` if successfully added, `false` if method doesn't exist
- **Note**: Last element is the return type

##### get_method_formal_list
```cpp
vector<Formal*>* get_method_formal_list(string class_name, string method_name);
```
- **Purpose**: Retrieves the ordered list of formal parameters for a method
- **Parameters**:
  - `class_name` - Name of the class
  - `method_name` - Name of the method
- **Returns**: Pointer to vector of Formal nodes, or `nullptr` if not found
- **Note**: Last element represents the return type

---

#### Utility

##### print
```cpp
void print();
```
- **Purpose**: Prints all name maps for debugging purposes
- **Output**: Displays all classes, hierarchy, methods, variables, and formals

---

## AST_Name_Map_Visitor Class

**Purpose**: Implements the visitor pattern to traverse the AST and populate the Name_Maps structure. This visitor collects all name-related information during a single traversal of the AST.

### Private Data Members

#### name_maps
```cpp
Name_Maps *name_maps;
```
- **Purpose**: Pointer to the Name_Maps structure being populated
- **Initialized**: In constructor with `new Name_Maps()`

#### current_visiting_class
```cpp
string current_visiting_class = "";
```
- **Purpose**: Tracks the name of the class currently being visited
- **Usage**: Used to associate declarations with their containing class

#### current_visiting_method
```cpp
string current_visiting_method = "";
```
- **Purpose**: Tracks the name of the method currently being visited
- **Usage**: Used to associate local variables with their containing method

---

### Public Methods

#### Constructor
```cpp
AST_Name_Map_Visitor();
```
- **Purpose**: Initializes the visitor with a new Name_Maps instance
- **Side Effect**: Allocates a new Name_Maps object

#### getNameMaps
```cpp
Name_Maps* getNameMaps();
```
- **Purpose**: Returns the populated Name_Maps structure
- **Returns**: Pointer to the Name_Maps object

#### Visitor Methods
The class implements all visitor methods defined in AST_Visitor:

**Declaration Visitors** (actively populate maps):
- `void visit(Program* node)` - Traverses main method and class declarations
- `void visit(MainMethod* node)` - Sets up __main__ class and main method
- `void visit(ClassDecl* node)` - Registers class and its hierarchy
- `void visit(VarDecl* node)` - Registers class/method variables
- `void visit(MethodDecl* node)` - Registers method and processes body
- `void visit(Formal* node)` - Registers formal parameters

**Statement/Expression Visitors** (traverse without action):
- Control flow: `visit(If*)`, `visit(While*)`, `visit(Nested*)`, `visit(Continue*)`, `visit(Break*)`, `visit(Return*)`
- I/O: `visit(PutInt*)`, `visit(PutCh*)`, `visit(PutArray*)`, `visit(GetInt*)`, `visit(GetCh*)`, `visit(GetArray*)`
- Timing: `visit(Starttime*)`, `visit(Stoptime*)`
- Expressions: `visit(BinaryOp*)`, `visit(UnaryOp*)`, `visit(ArrayExp*)`, `visit(CallExp*)`, `visit(CallStm*)`, `visit(ClassVar*)`, `visit(Assign*)`, `visit(Length*)`, `visit(NewArray*)`, `visit(NewObject*)`, `visit(This*)`, `visit(IdExp*)`, `visit(OpExp*)`, `visit(IntExp*)`
- Types: `visit(Type*)`

---

## Utility Function

### makeNameMaps
```cpp
Name_Maps* makeNameMaps(Program* node);
```
- **Purpose**: Convenience function to create and populate Name_Maps from a Program AST
- **Parameters**: `node` - Pointer to the Program AST root
- **Returns**: Pointer to populated Name_Maps structure
- **Usage**: 
  ```cpp
  Program* prog = parser.parse();
  Name_Maps* maps = makeNameMaps(prog);
  ```

---

## Usage Example

```cpp
// Parse the program
Program* program = parser.parse();

// Build name maps
Name_Maps* name_maps = makeNameMaps(program);

// Use name maps for semantic analysis
if (name_maps->is_class("MyClass")) {
    VarDecl* var = name_maps->get_class_var("MyClass", "field");
    string parent = name_maps->get_parent("MyClass");
}

// Check method existence
if (name_maps->is_method("MyClass", "myMethod")) {
    vector<Formal*>* formals = name_maps->get_method_formal_list("MyClass", "myMethod");
}
```

---

## FDMJ2026 Specific Notes

1. **Single-Level Inheritance**: The `get_parent()` method returns only the direct parent, as FDMJ2026 restricts inheritance to one level.

2. **Main Method Handling**: The main method is treated as belonging to a special `__main__` class for consistency in the name maps.

3. **Return Type Storage**: Method return types are stored as the last element in the formal parameter list for uniformity.

4. **No Validation**: The Name_Maps class only collects data. Semantic validation (circular inheritance, single-level inheritance, etc.) is performed by the semantic analyzer.