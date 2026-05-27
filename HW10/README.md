# HW 10

这次作业基于 inductino variables 和 derived induction variables 概念进行loop优化，主要：

* (Basic) induction variable发现 (`loopiductionbasic.cc`)
* Derived induction variable 发现 (`loopinductionderived.cc`). 
* Strength reduction plan 生成并执行(`loopstrengthreduction.cc`)。
* 最后是 unused induction variable removal（`loopinductionelimination.cc`)

`loopinductionopt.cc`中代码将整个优化过程串联起来。另外，`loopoptanalysis`和`loopoptreport`可以用来分析和报告中间结果（参见`main.cc`）。

输入：`.4-ssa-withflow-xml.quad`文件。
输出：在standard out 打印 loop induction variables 及 strength reduction plan 信息，以及优化过的 quad（`.4-ssa-loopivopt.quad`）文件。

在 .fmj 文件中，加上了相应 4-ssa.quad 在进行优化时，其 basic induction var，derived induction var 以及 strength reduction plan 的 printout，供参考。

假设：
*  每个 loop header 总是已经有了一个 preheader了. 
*  我们只处理“一阶”derived induction variable，并且一定是这样形状的：`j = a * i + b` （或由交换率变化的，比如` j = b + a * i`），其中 `a` 和 `b` 是constants, `i` 是（当前loop的）basic induction。 variable。 (注意：basic induction var我们仍假设 i = i + k，其中 k 可以是constant或者(当前) loop invariant。)

对于`loopheaderwithflow.cc`中需要的代码，你可以从HW9直接抄（copy）过来。

注意，除了上面提到你需要修改的 `.cc` 文件（即：`loopiductionbasic.cc`，`loopinductionderived.cc`，`loopstrengthreduction.cc`，`loopinductionelimination.cc`， 以及copy的`loopheaderwithflow.cc`)，其他文件都不要进行改变。

最后，`vendor/genAndrunQuad` 提供可以自行从 fmj 文件产生 quad 程序，并直接运行 quad 程序的能力，供测试你的代码优化程序。具体运行方式见那边的 README 文件。