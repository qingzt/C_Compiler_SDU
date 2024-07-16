# 题目描述

本实验要做的是实现一个可以把四元式翻译成x86目标代码的代码生成器。代码生成器求解待用信息、活跃信息和寄存器描述符地址描述符等，根据它们分配寄存器，并逐条把四元式翻译成汇编代码，注意代码生成器需要在一个基本块范围内考虑如何充分利用寄存器，而全局代码的生成则是简单地将各个基本块代码串联。

# 代码生成器的输入

* 符号表
* 临时变量个数
* 四元式

符号表是通过分析声明语句构建的，我们假定本实验中将所有的形参都视为基本块中的局部变量。在后续中间代码生成中，又加入了临时变量信息，本实验中输入的临时变量信息是临时变量的个数。本实验中没有类型转换，不会出现整型和实型相加这类操作，且整型和实型的运算在目标代码的指令生成中亦不做区分。同时假设代码中不存在任何语义错误。

# 目标程序的形式

本实验我们选择汇编代码作为目标程序。汇编也有CISC和RISC之分，具体来说，我们输出以Intel和AMD为代表的X86架构的CISC指令集汇编。采用CISC指令集有很多特殊限制，如乘除指令需要固定使用EAX和EDX寄存器等，但是本实验我们不考虑寄存器的特殊限制，在设计代码生成算法时，我们设计一个通用算法，此时的寄存器表示为R0, R1,…的形式。且Mul和Div指令不考虑其使用寄存器的特殊性，按普通指令处理。

# 代码书写约定

我们把四元式编号前面加个?符号构成跳转指令的标号。更精确地，由于转移语句只可能会出现在基本块的最后一条语句，因此翻译到基本块的最后一条语句时，如果是转移语句，则在转移的目标语句前生成标号。

实际上，由于变量没有在静态数据区声明，而是在活动记录动态开辟空间，因此不能使用名字进行访问。在非嵌套过程语言的栈式实现中，我们需要通过EBP寄存器获得变量地址进行访问。假设变量在符号表中偏移量为offset，对于形参，应用[EBP+offset+8]来访问该变量，对于局部变量，假设保存寄存器个数为n，则使用[EBP-offset-4(n+1)]访问该变量。由于本实验中将所有的形参都视为了局部变量，所以对访问作出了一定程度的简化处理，即只需要利用[EBP-offset]访问变量即可。假设我们有两个寄存器，z为局部变量，在符号表中offset为8，用pushad保护z寄存器，则对应代码为mov [ebp-8], eax。

需要特殊处理的是临时变量。有的临时变量计算出来后接着被使用，且在后续代码中不活跃，这种临时变量无需存入栈帧，也有的临时变量计算出来后要等后面使用，如果寄存器数量不足，可能就需要存入栈帧，因此需要为其分配一个地址单元。在中间代码生成中，已经在符号表中保存了一个过程的总体偏移量offset，它就是一个过程中所有局部变量占用空间的总和。这个偏移量加上保存的寄存器空间，就是临时变量保存的起始空间，它等于运行时的ESP，但现在生成代码是编译时，需要记录下每个临时变量的地址，供后续访问使用。即当遇到需要保存的局部变量时，先从符号表查询到这个符号，如果该变量还未分配空间，则将临时变量起始空间作为当前临时变量地址并存入符号表，然后生成代码开辟这个空间再将地址偏移量返回。

而本实验对分配或获得一个临时变量的存储地址亦作出了一定程度的简化，在需要存入临时变量时按序依次将临时变量在局部变量之后的栈帧中开辟空间即可，可以像局部变量一样通过[EBP-offset]访问这个地址。

对寄存器的数量和名字我们也作一定程度的简化处理，数量我们约定为三个，名字约定为R0、R1和R2。分配寄存器也作简化，先分配R0，再分配R1，再分配R2，顺序分配。

# 待用信息和活跃信息的生成

对于一个四元式的每个变量，都有一个待用信息，它包括两方面的信息：

* 后续引用点（USE）：在当前基本块中，该变量后续被引用的四元式编号。
* 活跃信息（LIVE）：后续该变量是否活跃。

因此，待用信息用一个二元组（USE, LIVE）去表示，没有引用点的USE用 - 表示。

待用信息求解的结果是为每个四元式的每个变量求一个二元组（USE, LIVE），这些信息被附加在中间代码上，即为中间代码的每个四元式的每个变量增加这样两个属性。在求解活跃信息时，由于所有变量都被登记在符号表中，因此只需在符号表中为每个变量增加两个域，即USE和LIVE。初始时将符号表中个变量的USE设置为非待用，LIVE则根据基本块出口是否活跃设置。本实验中，把基本块中所有临时变量均看作基本块出口之后的非活跃变量，而把所有非临时变量看作基本块出口之后的活跃变量。

在基本块的出口处，活跃变量存入主存中时依照字典序存入。

# 寄存器和地址描述符

每个可用的寄存器都有一个寄存器描述符Rval，它是一个集合，用来记录哪些变量的值存放在此寄存器中。你可以用Rval（R0）表示寄存器R0中保存的变量，基本块级别的代码生成，初始可以将所有寄存器描述符置空。

而每个变量都有一个地址描述符Aval，它可以被记录在符号表中，也是一个集合，用来记录标记变量的值是存放在寄存器还是内存中。

# 目标代码的映射

各四元式对应的目标代码（注意在本实验中整型和浮点的运算不作区分）：


|          中间代码          |                 目标代码                 |                                              说明                                              |
| :------------------------: | :---------------------------------------: | :--------------------------------------------------------------------------------------------: |
| (=,lhs/UINT/UFLOAT,-,dest) |          mov R, lhs/UINT/UFLOAT          |            (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成目标代码            |
|        (j,-,-,dest)        |                 jmp ?dest                 |                                         无条件转移指令                                         |
|      (jnz,lhs,-,dest)      |  mov R, lhs<br />cmp R, 0<br />jne ?dest  |                              如果lhs∈Rval(R)，则不生成第1条代码                              |
|     (jθ,lhs,rhs,dest)     | mov R, lhs<br />cmp R, rhs<br />jθ ?dest |                              如果lhs∈Rval(R)，则不生成第1条代码                              |
|      (+,lhs,rhs,dest)      |        mov R, lhs<br />add R, rhs        |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|      (-,lhs,rhs,dest)      |        mov R, lhs<br />sub R, rhs        |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|      (*,lhs,rhs,dest)      |        mov R, lhs<br />mul R, rhs        | (1) R是新分配给dest的寄存器，不考虑特殊寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码 |
|      (/,lhs,rhs,dest)      |        mov R, lhs<br />div R, rhs        | (1) R是新分配给dest的寄存器，不考虑特殊寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码 |
|     (==,lhs,rhs,dest)     |  mov R, lhs<br />cmp R, rhs<br />sete R  |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|     (!=,lhs,rhs,dest)     |  mov R, lhs<br />cmp R, rhs<br />setne R  |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|      (<,lhs,rhs,dest)      |  mov R, lhs<br />cmp R, rhs<br />setl R  |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|     (<=,lhs,rhs,dest)     |  mov R, lhs<br />cmp R, rhs<br />setle R  |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|     (\>,lhs,rhs,dest)     |  mov R, lhs<br />cmp R, rhs<br />setg R  |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|     (>=,lhs,rhs,dest)     |  mov R, lhs<br />cmp R, rhs<br />setge R  |          (1) R是新分配给dest的寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码          |
|     (&&,lhs,rhs,dest)     |        mov R, lhs<br />and R, rhs        | (1) R是新分配给dest的寄存器，不考虑特殊寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码 |
|    (\|\|,lhs,rhs,dest)    |         mov R, lhs<br />or R, rhs         | (1) R是新分配给dest的寄存器，不考虑特殊寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码 |
|       (!,lhs,-,dest)       |           mov R, lhs<br />not R           | (1) R是新分配给dest的寄存器，不考虑特殊寄存器<br />(2) 如果lhs∈Rval(R)，则不生成第1条目标代码 |
|        (R,-,-,dest)        |              jmp ?read(dest)              |                                       视作无条件转移指令                                       |
|        (W,-,-,dest)        |             jmp ?write(dest)             |                                       视作无条件转移指令                                       |
|        (End,-,-,-)        |                   halt                   |                                            程序终止                                            |
|        Syntax Error        |                   halt                   |                                          程序异常终止                                          |

# 样例输入

## 样例1

```
4
a 0 null 0
b 0 null 4
c 0 null 8
d 0 null 12
4
12
0: (=,0,-,T0_i)
1: (=,T0_i,-,TB0)
2: (=,1,-,T1_i)
3: (=,T1_i,-,TB1)
4: (=,2,-,T2_i)
5: (=,T2_i,-,TB2)
6: (jnz,TB0,-,8)
7: (j,-,-,10)
8: (+,TB1,TB2,T3_i)
9: (=,T3_i,-,TB3)
10: (W,-,-,TB3)
11: (End,-,-,-)

```

## 样例2

```
4
a 0 null 0
b 0 null 4
x 1 null 8
y 1 null 16
6
13
0: (=,3,-,T0_i)
1: (=,T0_i,-,TB0)
2: (=,2,-,T1_i)
3: (=,T1_i,-,TB1)
4: (+,TB0,TB1,T2_i)
5: (=,T2_i,-,TB0)
6: (=,5.200000,-,T3_d)
7: (=,T3_d,-,TB2)
8: (=,2.400000,-,T4_d)
9: (=,T4_d,-,TB3)
10: (-,TB2,TB3,T5_d)
11: (=,T5_d,-,TB2)
12: (End,-,-,-)

```

## 样例3

```
3
a 0 null 0
b 0 null 4
c 0 null 8
11
23
0: (R,-,-,TB0)
1: (=,0,-,T0_i)
2: (=,T0_i,-,TB1)
3: (=,0,-,T1_i)
4: (j>=,TB0,T1_i,6)
5: (j,-,-,21)
6: (=,1,-,T2_i)
7: (-,TB0,T2_i,T3_i)
8: (=,T3_i,-,TB0)
9: (=,2,-,T4_i)
10: (/,TB0,T4_i,T5_i)
11: (=,2,-,T6_i)
12: (*,T5_i,T6_i,T7_i)
13: (-,TB0,T7_i,T8_i)
14: (=,T8_i,-,TB2)
15: (=,0,-,T9_i)
16: (j!=,TB2,T9_i,18)
17: (j,-,-,3)
18: (+,TB1,TB0,T10_i)
19: (=,T10_i,-,TB1)
20: (j,-,-,3)
21: (W,-,-,TB1)
22: (End,-,-,-)
```

# 样例输出

## 样例1

```
mov R0, 0
mov R1, 1
mov R2, 2
mov [ebp-0], R0
mov [ebp-4], R1
mov [ebp-8], R2
cmp R0, 0
jne ?8
jmp ?10
?8:
mov R0, [ebp-4]
add R0, [ebp-8]
mov [ebp-12], R0
?10:
jmp ?write([ebp-12])
halt

```

## 样例2

```
mov R0, 3
mov R1, 2
add R0, R1
mov R2, 5.200000
mov [ebp-0], R0
mov R0, 2.400000
sub R2, R0
mov [ebp-4], R1
mov [ebp-8], R2
mov [ebp-16], R0
halt

```

## 样例3

```
jmp ?read([ebp-0])
mov R0, 0
mov [ebp-4], R0
?3:
mov R0, 0
mov R1, [ebp-0]
cmp R1, R0
jge ?6
jmp ?21
?6:
mov R0, 1
mov R1, [ebp-0]
sub R1, R0
mov R0, 2
mov R2, R1
div R2, R0
mov R0, 2
mul R2, R0
mov R0, R1
sub R0, R2
mov R2, 0
mov [ebp-0], R1
mov [ebp-8], R0
cmp R0, R2
jne ?18
jmp ?3
?18:
mov R0, [ebp-4]
add R0, [ebp-0]
mov [ebp-4], R0
jmp ?3
?21:
jmp ?write([ebp-4])
halt

```

# 附

在X86汇编中，条件跳转的指令通常使用基于比较结果的条件码来执行跳转。根据不同的比较操作符，条件跳转的指令会有所不同。以下是一些常见的条件跳转指令和它们对应的比较操作符：

> ==：等于

X86 汇编中对应的条件跳转指令是
je（Jump if Equal）。

> !=：不等于

X86 汇编中对应的条件跳转指令是
jne（Jump if Not Equal）。

> <：小于

X86 汇编中对应的条件跳转指令是
jl（Jump if Less）。

> <=：小于等于

X86 汇编中对应的条件跳转指令是
jle（Jump if Less or Equal）。

> \>：大于

X86 汇编中对应的条件跳转指令是
jg（Jump if Greater）。

> \>=：大于等于

X86 汇编中对应的条件跳转指令是
jge（Jump if Greater or Equal）。

这些条件跳转指令的用法是：如果前一次比较的结果满足条件，则跳转到指定的标签处执行。例如，如果要实现 (j==, lhs, rhs, dest) 的条件跳转，则可以使用 je 指令。

## 各个功能的伪代码逻辑

```
基本块划分的伪代码：

输入：四元式q1, q2, ..., qn
输出：划分出的基本块

q1标记为基本块入口语句;
for i=1:n do
    if qi 形如 (jθ,-,-,qj) then
        qj标记为基本块入口语句;
        if i<n then qi+1 标记为基本块入口语句;
    end
    if qi 形如 (j,-,-,qj) then qj标记为基本块入口语句;
    if qi 形如 (R,-,-,v) then qi标记为基本块入口语句;
    if qi 形如 (W,-,-,v) then qi标记为基本块入口语句;
end
i=1;
while i <= n do
    if qi 是基本块入口语句 then
        if i=n then
            qi 是一个基本块;
            break;
        for j = i+1 : n do
            if qj 是基本块入口语句 then
                qi, qi+1, ..., qj-1 是一个基本块;
                i=j;
                break;
            else if qj 是转移或停机语句 then
                qi, qi+1, ..., qj 是一个基本块;
                i=j+1;
                break;
            end
        end
    else
        i++
    end
end

```

```
求解待用信息的伪代码：

输入：基本块四元式序列q1,q2,…,qm
输出：附加在四元式上的待用信息

把基本块中各变量符号表中的use设置为“非待用（-）”；
根据变量在出口处是否活跃，设置符号表中变量的live为“活跃（Y）”或“非活跃（N）”；
for i=m…1，设qi为（θ，x，y，z）do
	符号表中变量z的use和live赋值给四元式qi的z变量；
	符号表中变量z的use和live分别置为-和N；
	符号表中变量x和y的use和live赋值给四元式qi的x和y变量；
	符号表中变量x和y的use和live分别置为qi和Y；
end

```

```
局部寄存器分配的伪代码：

输入：四元式q，所有寄存器的集合R
输出：返回分配的寄存器

function getReg（q）：
	if q形如（θ，x，y，z）then
		if 存在Ri∈Aval（x）并且Rval（Ri）={x}并且（x=z或者x.live=N）then
			return Ri；
		end
	end
	if 存在Ri使得Rval（Ri）为空then return Ri；
	RA={Ri|Ri∈R并且Rval（Ri）中包含主存单元}；
	if RA为空then RA=R；
	if 存在Rj∈RA并且任意a∈Rval（Rj）都有a∈Aval（a）then
		Ri=Rj；
	else
		Ri = （Rj∈RA）argmax（a∈Rval（Rj））min a.use；
	end
	foreach a∈Rval（Ri）do
		if a不属于Aval（a）并且a≠z then 生成代码：mov a，Ri；
		if a = x或者（a=y并且x∈Rval（Ri））then Aval（a）={a，Ri}；
		else Aval（a）= {a}；
		Rval（Ri）= Rval（Ri）- {a}；
	end
	return Ri；
end getReg

```

```
释放寄存器的伪代码：

输入：占用寄存器的变量var，变量所在四元式所属基本块Bi
输出：修改后的寄存器和地址描述符

function releaseReg（var，Bi）：
	if var 不属于 liveOut（Bi）并且存在reg∈Aval（var）then
		Rval（reg）-={var}；
		Aval（var）-={reg}；
	end
end releaseReg

```

```
生成标号的伪代码：

输入：生成标号位置四元式quad。是否生成标号的标志数组labelFlag
输出：生成标号后的目标代码

function genLabel（quad，labelFlag）：
	if !labelFlag[quad.block] then
		在quad前生成标号?quad:；
		labelFlag[quad.block]=true；
	end
end genLabel

```

```
基于基本块的简单代码生成伪代码：

输入：基本块集合N = {B1，B2，…，Bm}，基本块Bi的四元式序列qi1，qi2，…，qini，基本块Bi出口处的活跃变量集合liveOut（Bi）
输出：目标代码

构造布尔型数组labelFlag[m]记录基本块是否建立标号标志，所有元素初始化为false；
foreach Bi∈N do
	for j=1:ni do
		if qij形如（θ，x，y，z）then
			Rz=getReg（qij）；
			x' = 存在Rx∈Aval（x）？Rx：x；
			y' = 存在Ry∈Aval（y）？Ry：y；
			if x' = Rz then
				生成代码：θ Rz，y'
				Aval（x）-={Rz}；
			else
				生成代码：mov Rz，x'；和θ Rz，y'；
			end
			if y' = Rz then Aval（y）-={Rz}；
			Rval（Rz） = {z}，Aval（z）={Rz}；
			releaseReg（x，Bi），releaseReg（y，Bi）；
		else if qij形如（R，-，-，v）then
		    生成代码：jmp ?read(v)；
		else if qij形如（W，-，-，v）then
		    生成代码：jmp ?write(v)；
		end
	end
	foreach a∈liveOut（Bi）并且a不属于Aval（a）do
		Ra=Aval（a）中的寄存器；
		生成代码：mov a，Ra；
	end
	if qini形如（j，-，-，q）then
		生成代码：jmp ?q；
		genLabel（q，labelFlag）；
	else if qini形如（jθ，x，y，q）then
		x' = 存在Rx∈Aval（x）？Rx：x；
		y' = 存在Ry∈Aval（y）？Ry：y；
		if x' = x then
			x' = getReg（qini）；
			生成代码：mov x'，x；
		end
		生成代码：cmp x'，y'和jθ ?q；
		genLabel（q，labelFlag）；
	else if qini=（jnz，x，-，q）then
		x' = 存在Rx∈Aval（x）？Rx：x； 
		if x' = x then
			x' = getReg（qini）；
			生成代码：mov x'，x；
		end
		生成代码：cmp x'，0和jne ?q；
		genLabel（q，labelFlag）；
	else if qini=（End，-，-，-）then
		生成代码：halt；
	end
	所有寄存器描述符置空，所有变量的地址描述符置空；
end

```
