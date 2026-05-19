# HW 9

这次作业涉及简单的loop优化，主要：1）loop的发现，2）loop invariant的提取(hoisting)。

输入：`.4-ssa-withflow-xml.quad`文件。
输出：在standard out 打印 loop headers 信息，以及优化过的 quad（`.4-ssa-loopopt.quad`）文件。

假设：
*  每个 loop header 总是已经有了一个 preheader了. 
*  每个 call 都有可能有副作用（method call 或 ext call）
*  每个 memory load 都可能获取到不同的值，即使地址一样。

你需要：填写`findloopheader.cc`以及`loophoisfunc.cc`中的代码。不要改变任何其他文件！
