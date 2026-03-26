# FDMJ2026 Language Specification

This document defines the **FDMJ2026** programming language, a simplified object-oriented language designed for a college-level compiler course targeting **RPi (ARM) Assembly**.

## 1. Language Behavior & Semantics

### Object Orientation & Inheritance

* **Single-Level Inheritance**: A class can inherit properties and methods from exactly one parent class using the `extends` keyword. In FDMJ2026, a class that extends another cannot itself be a parent to a third class. A parent class doesn't have to be declared before its subclasses (in the source code file). 

* **Method Overriding**: If a child class defines a method with the same name and signature as a method in its parent, the child’s version replaces the parent’s. When that method is called on an instance of the child class, the child’s version executes. For this to work, the methods must have the same signature (i.e., same number and types of parameters), and the return type of the child method must be the same or a subclass (in case of a class type) of the return type of the parent method.

* **Variable Hidding**: When a class variable of the same name appears in the subclass, called `variable hiding`, then the behavior/semantics is as if it's a new class variable in the subclass. 

* **Dynamic Dispatch**: The version of a method that runs is determined by the actual type of the object at runtime, not the type of the variable holding it.

* **The `this` Keyword**: Within a method, `this` refers to the specific object instance currently executing the code.

* Class variables (i.e., variables declared at the class level can only be accessed via  object, i.e., in the form `obj.id`).

### Types and Logic

* **Integer-Based Logic**: There is no `boolean` type; all logic is handled via `int`.

* **Truthiness**: Control flow statements (`if`, `while`) and logic operators treat `0` as **false** and any non-zero value as **true**.

* **Binary Operations**: Supported operators include `+`, `-`, `*`, `/`, `||`, `&&`, `<`, and `==`.

* **Scalar Constraints**: Arithmetic and comparison operators apply only to scalar `int` values, not to entire arrays.

### Arrays and Memory

* **Initialization**: Arrays can be created with fixed values at the time of declaration (e.g., `int[] a = {1, 2, 3};`) or dynamically allocated with a specific size using `new int[Exp]`.

* **Explicit Allocation**: Except for initialization at declaration, arrays must be explicitly created via `new` before use. (Note: Objects variables must also be explictly created via `new` before use.) 

* **Length Property**: Every array has a built-in length property that can be accessed via length() call, which gives the number of elements it contains. The first element of an array is at 0th position (e.g., a[0]).

* **Reference Semantics**: Assigning an object or array to a new variable (e.g., `a = b;`) copies the memory address (reference), meaning both variables point to the same data.

## 2. Formal Grammar

### Program Structure

* `Program` $\rightarrow$ `MainMethod` `ClassDecl`\*

* `MainMethod` $\rightarrow$ `public int main ( ) {` `VarDecl`\* `Statement`\* `}`

* `ClassDecl` $\rightarrow$ `public class id`  [ `extends id` ] `{` `VarDecl`\* `MethodDecl`\* `}`

* `MethodDecl` $\rightarrow$ `public Type id ( FormalList ) {` `VarDecl`\* `Statement`\*  `}`

* `FormalList` $\rightarrow$ `Type id` ( `,` `Type id` )\* | $\epsilon$

### Declarations & Types

* `Type` $\rightarrow$ `int` | `int [ ]` | `class id` 

* `VarDecl` $\rightarrow$ `Type id`  [ `= ArrayInit` ] `;`

* `ArrayInit` $\rightarrow$ `{` `CONST` ( `,` `CONST` )\* `}`

### Statements

* `Statement` $\rightarrow$

  * `{` `Statement`\* `}`

  * `if ( Exp ) Statement else Statement`
 
  * `if ( Exp ) Statement`

  * `while ( Exp ) Statement`
 
  * `while ( Exp ) ;` 

  * `Exp = Exp ;`

  * `Exp [ Exp ] = Exp ;`
 
  * `return Exp ;`
 
  * `continue ;`
 
  * `break ;`
 
  * `putint ( Exp ) ;`
 
  * `putch ( Exp ) ;`
 
  * `putarray ( Exp, Exp ) ;`
 
  * `starttime ( ) ;`
 
  * `stoptime ( ) ;`

### Expressions

* `Exp` $\rightarrow$

  * `( Exp )`
 
  * `Exp op Exp`

  * `Exp [ Exp ]`

  * `length ( Exp )` // to get the length of an array
 
  * `! Exp`
 
  * ` - Exp` 
 
  * `Exp . id` //to get to the class variable

  * `Exp . id ( ExpList )` //to call a method

  * `new int [ Exp ]`

  * `new id ( )`

  * `CONST` | `id` | `this`
 
  * `getint ( )`
 
  * `getch ( )`
 
  * `getarray ( Exp )` 
 
* `ExpList` $\rightarrow$ `Exp ExpRest*`  
* `ExpRest` $\rightarrow$ `, Exp` | $\epsilon$

## 3. Lexical Notes

* **CONST**: An integer consisting of `(+/-)[0-9]+`. When appearing in an expression (`Exp`), it must be a non-negative integer to eliminate grammar ambiguity.

* **id**: Any string consisting of `[a-z]`, `[A-Z]`, `[0-9]`, and `_` (underscore). It must start with a letter or an underscore and cannot be a reserved keyword. Identifiers are case-sensitive.

* **Operator Shortcut**: The boolean operators `||` and `&&` follow "shortcut" semantics; the second operand is only executed if the first does not determine the final result.

* **Execution Order**: All statements and expressions are evaluated from left to right.
## 4. Note on Side Effects
* All expressions are taken to be executed from left to right (need to be considered when there are *side effects*). For example, assume `id` is a class object which has a class variable `x` (with initial value `0`) and method `f` (which increments the value of `x` by `1` and returns `0`), then `id.x+2*id.f()` gives 0, while `2*id.f()+id.x` gives 1.

