## 任务简述
你需要完成一个简单的C++风格的编译器。这个实验分为若干部分。在这个部分中，你需要完成该编译器的词法分析器。
你的编译器不应当对用户输入程序进行假设（例如，假设用户输入的程序不超过若干字节，或每行不超过若干字符，或程序不超过若干符号）。
给定一个C++语言源程序，你需要将其从字符流转换为词语流。具体来说，你需要过滤源程序中的空白符(空格，注释、tab，回车、换行等)，识别关键字、标识符、数字以及运算符。
输入为一段C++语言风格的源程序，输出每个单词所对应的类型。
## 输出格式
若程序没有词法错误，输出为单词流以及每个单词的类型，每个单词以及类型占一行。
若程序有词法错误，直接输出相应的错误。
输出的最后应该有换行。
### 错误类型
错误有以下四种：
| 错误描述                     | 输出内容                                                                             |
| ---------------------------- | ------------------------------------------------------------------------------------ |
| 浮点数中不止一个小数点       | Malformed number: More than one decimal  point in a floating point number.           |
| 小数点在浮点数的开始或者结尾 | Malformed number: Decimal point at the  beginning or end of a floating point number. |
| 整数中有前导零               | Malformed number: Leading zeros in an  integer.                                      |
| 有不能识别的字符             | Unrecognizable characters.                                                           |
### 关键字表
关键字表如下
| 关键字 | 对应类型  |
| ------ | --------- |
| int    | INTSYM    |
| double | DOUBLESYM |
| scanf  | SCANFSYM  |
| printf | PRINTFSYM |
| if     | IFSYM     |
| then   | THENSYM   |
| while  | WHILESYM  |
| do     | DOSYM     |
### 符号表
符号表如下
| 符号 | 对应类型  |
| ---- | --------- |
| =    | AO        |
| ==   | RO        |
| >    | RO        |
| >=   | RO        |
| <    | RO        |
| <=   | RO        |
| \|\| | LO        |
| &&   | LO        |
| !    | LO        |
| !=   | RO        |
| +    | PLUS      |
| -    | MINUS     |
| *    | TIMES     |
| /    | DIVISION  |
| ,    | COMMA     |
| (    | BRACE     |
| )    | BRACE     |
| {    | BRACE     |
| }    | BRACE     |
| ;    | SEMICOLON |
### 标识符以及数字
| 标识符或者数字 | 对应类型 |
| -------------- | -------- |
| 标识符(如x)    | IDENT    |
| 整数（如3）    | INT      |
| 小数(如2.1)    | DOUBLE   |
### 注释
对于注释，与C++类似，有两种方式，如下表所示：
| //    | 单行注释 |
| ----- | -------- |
| /* */ | 多行注释 |
## 输入输出示例
在词法分析中，注释的内容不应输出，即需要将这些内容过滤掉。
示例1
输入：
```
int a, b;
double c=1.2; // This is a comment
scanf(a);
scanf(b);
printf(c);
```
输出：
```
int INTSYM
a IDENT
, COMMA
b IDENT
; SEMICOLON
double DOUBLESYM
c IDENT
= AO
1.2 DOUBLE
; SEMICOLON
scanf SCANFSYM
( BRACE
a IDENT
) BRACE
; SEMICOLON
scanf SCANFSYM
( BRACE
b IDENT
) BRACE
; SEMICOLON
printf PRINTFSYM
( BRACE
c IDENT
) BRACE
; SEMICOLON
```
示例2：
输入：
```
int a = 6037210
int b = 06356417
```
输出：
```
Malformed number: Leading zeros in an integer.
```