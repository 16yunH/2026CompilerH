**Homework Assignment 2**

**See more on eLearning.**

This assignment is to

1. write a visitor of the AST to produce semantic map for names (namemap), and
2. write another visitor of the AST to add semantic information to each AST node (if applicable).

**Semantic check** not only finds out the semantic of each AST node, and also check if the type of a node is correct. Various type checking needs to be done, including for example, the left-hand-side of an assignment is an lvalue (or not), the method call parameters match up with the declaration of the method in terms of type type. **Your report must give all the type checking your program does. Missing apparant type checking is a cause for deducting points.** 

Your program may report the first type-error (if any) and quit. Or if you are brave, try to continue the type-checking to find more errors.

Some more notes below:

* The parser binary executable is given under directory `tools/parser` (choose your platform, the default one is for Linux on x86_64 (Windows machines)). You may use `make parse` to generarte the AST for all the fmj files under the test directory (into *.2.ast files).

* The full FDMJ AST definition is given in the FDMJAST.hh file (with accompanying FDMJAST.cc code). The semantic map definition is given in the namemap.hh and semant.hh files. See description of these three hh files in the docs directory.

* Conversion code between FDMJAST (with/without semantic information) and XML is given. You should use it to print the parsing result using the given code (as exemplified in main.cc). Example AST output may be found under test/output_example directory.

* Use eLearning to **submit** your code package (follow the instructions). 

**Your code will be tested with more test programs (fmj files) for grading.**

补充说明：

1. 自行修改`tools/main/main.cc`进行测试。最后测评时我们将使用统一的main（即repo里给的main.cc）。
2. 测试时，建议自己编写一些fmj code文件（放在test目录下），使用`make parse`先生成AST，然后再进行测试。
