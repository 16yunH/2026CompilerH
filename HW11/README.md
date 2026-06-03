# HW 11

这次作业涉及SSA Quad 到 ARM Assembly 指令转化。

输入：`.4-ssa-withflow-xml.quad`文件。
输出：ARMv7-A （`.s`）文件, 除了仍使用（无限量）temp之外，基本就是最后可运行程序。

过程：整个过程经过三步:

* 形成一个 advDFG 图，用于指令选择； 
* 在 advDFG 图上，进行指令选择（使用课堂上说的一个ARMv7-A 指令子集），优化指令的使用，形成 preSchedule。
* 在 preSchedule 的基础上，进行指令线性化，处理好Jump/CJump，Phi，以及 Function entrance，function call，以及 return。最后形成 schedule 结构，并输出至 `.s` 文件。

你需要：填写`buildAdvDFG.cc`、`selectInstr.cc` 以及 `schedule.cc` 中的代码。不要改变任何其他文件！

注意：`buildPreSchedule.cc` 是为了搭好 `preSchedule` structure 的“架子”，以便 `selectInstr.cc` 在每个 Basic Block 里工作。
