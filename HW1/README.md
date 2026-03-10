**Homework Assignment 1**

**See more on eLearning.**

**Do the following:**

1. **A simple “constant propagation”:** Write a visitor that is similar to MinusIntConvert, but for each binary operation on two constants, reduces the binary operation into a constant node (recursively from bottom up). _Your code must first use the minusIntConvert visitor to reduce the "- constant" to a constant._
2. **"Execute the program":** Write another visitor that runs the program and prints the returned value (from the final return statement, you may assume there are no other return statements in the program). _(You need to remember the current values of the variables in a table. Use a member in the visitor to remember the table.)_ If a **variable is undefined before use**, then:
     (1) _Assume its value is zero,_ and (2) _Report (stderr) where the use is at (which line and which column of the original program)._
5. Use eLearning to **submit** your code package (follow the instruction). 

**You code will be tested with more test programs (fmj files) for grading.**

补充说明：

1. 实现`include/ast/constantPropagation.hh`和`lib/ast/constantPropagation.cc`，其中，提供`Program *constantPropagate(Program *root);`函数供main调用。

2. 实现`include/ast/executor.hh`和`lib/ast/executor.cc`，其中，提供`int execute(Program *root);`函数供main调用。

3. 自行修改`tools/main/main.cc`进行测试。最后测评时我们将使用统一的main（基于上述两个供main调用的函数）。
