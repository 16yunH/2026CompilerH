# HW 12

这是本学期编译器构造的最后一步，将HW11得到的“假”汇编程序（只差temp需要改成register）作出最后的register allocation，形成可运行的ARMv7-A汇编代码。

* 输入：`.5-xml.asm`文件，每一个是前一步（HW11输出的）schedule的结构（`.fmj`和`.s`文件供参考）。
* 输出：`.colored.s`）文件，即最后完整的可运行的ARMv7-A汇编程序（假设不同可用register数生成的汇编程序在test folder下不同的子目录里）。

主要步骤如下：

1. 用`xml2asmprog`将输入文件读入`asmprog`结构（代码已给）;
2. 用`predataflowpass`对asmprog进行整理，比如给一些指令加入必要的destination/source等，以便作出正确的interference graph。
3. 用`asmdataflow`为整理好的`asmprog`中的每一个函数做data flow analysis（代码已提供），dataflow结果存在`asmdataflow`结构里。
4. 用`buildIg`，借助`asmdataflow`结果，做好inteference graph，存入`ig`结构中。
5. 用`simcoafrespiesel`并借助`coloring`结构及`ig`中的intereference graph，为`asmprog`每一个函数中的temp选择“color”或决定“spill“。
6. 最后根据`coloring`里的着色结果，用`asmprog2colored`把`asmprog`中所有temp都变成相应的register，并最后整理程序（尤其是处理spilled temps）.
7. 最后将`asmprog`输出最终程序（代码已提供）。

你需要做的就是上面的步骤 2、4、5、6，填写相应的代码。不要改变任何其他文件！

输出的colored.s文件可以使用`make run-assem-k`（k目前设置为2、5、9，与main.cc中对应），或者`make run-one-assem-k`运行。其中使用了cross architecture compiler 及 模拟器（见本学期HW1 lab中相应介绍）。**注意：qemu-arm (user mode) 只能在linux（如ubuntu）上运行。最后验证程序是否正确会看最后的colored.s是否运行正常。**