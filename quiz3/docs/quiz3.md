姓名_______________ 学号________________ 签名______________________
Quiz #3, Quad SSA过程信息收集及信息解释
问题背景：我们在将Quad转换为SSA形式过程中，有几个步骤：（1）每个变量（temp）需要考虑在哪些Basic block（Candidate Blocks）里插入Phi函数；（2）然后对每一个Phi是否放置进行过滤并形成Phi的“骨架”（即变量命名前）；（3）变量改名（增加version）；（4）最后考虑是否需要删除最后形成的Phi函数。我们希望记录在整个转换过程中，每个变量（1）Phi的candidate blocks，以及（2）产生了哪些新的versions（注意：原变量第一次赋值改为00 version不算是新version）。有了这些记录以及最后形成的SSA程序，我们可以对整个过程进行分析。
Coding要求：请见给定的quadssa_diag.hh/cc文件，里面有一个收集信息的SsaDiagState类，以及打印信息的代码（注意打印代码需要将最终转化好的SSA QuadFuncDecl一并输入进行打印）。你的任务是调整你的quadssa.cc代码，以便在适当的地方，加上对相关信息的收集。注意你只能修改quadssa.cc文件！你在编写调试你的代码时，可以将给定的代码（包括quadssa_diag.hh/cc以及一些test文件）copy到相应的代码树结构中去。
1.	在你的HW7的基础上，进行上述coding任务。main.cc不需要任何变化：输入仍是带数据流控制流的Quad XML文件（*.4-quadwithflow-xml.quad文件），输出是对应的 *.4-ssa.quad文件。另外给的*.fmj 和 *.4-block.quad文件是供参考的版本。*_diag.txt是相应打印SsaDiagState的结果。
2.	在quadssa.cc中对每个函数QuadFuncDecl转换完成并返回之前，以以下方式调用信息打印函数（假设funcdecl是已经转换完成的函数指针QuadFuncDecl *，diag是收集信息的SsaDiagState对象）：printSsaDiagSummary(funcdecl, diag)；这个diag可以是个全局变量，但注意初始化。
3.	你必须在断网并不使用大模型编程辅助的情况下完成coding。
4.	你需要仔细考虑什么情况下，需要插入信息收集的代码。
报告要求（很重要，留出时间来做！）	
请提交一个PDF（或markdown或txt）文件报告与代码，与提交作业一样的方式提交。在报告中，（1）使用一个给定的test为例子，从里面能找到一个变量，它的candidate phi block非空，但最后没有放置 phi。说明在哪个例子里，哪个变量有这个情况；（2）说明在这个test case下，为什么这个变量会出现这个情况，即：为什么有这个candidate phi，又为什么最后没放phi。
打分方法
（1）你的代码将在一些测试案例上运行，根据检查信息打印结果判断正确性。（2）你的报告也构成打分标准很重要的一部分。
提交方法
与HW的提交方法一致（打包、提交）。并在本页上签字，在课堂上将本页交给老师/助教。
务必在监考教师/助教指示及监督下开启网络进行elearning提交。
代码及Test cases文件
你可以用给定的test文件测试你的代码。并最后将代码以及报告打包提交至elearning系统（与HW提交一致的方式）。注意： 给定的test文件并没有把所有可能的SSA转换过程不同的情况都列出，需要你仔细考虑信息收集过程。
