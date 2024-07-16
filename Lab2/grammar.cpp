#include "parser.h"
using namespace std;

Parser::Parser()
{   
    grammar={
        //总程序部分
        {START,{PROG},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(START);
            symbolStack.push_back(symbol);
        }},
        {PROG,{SUBPROG},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(PROG);
            symbolStack.push_back(symbol);
        }},
        {SUBPROG,{M,VARIABLES,STATEMENT},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto statement=symbolStack.back();
            symbolStack.pop_back();
            auto variables=symbolStack.back();
            symbolStack.pop_back();
            auto m=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(SUBPROG);
            backpatch(statement.nextList,to_string(nxq));
            gen("End","-","-","-");
            symbolStack.push_back(symbol);
        }},
        {M,{ONE},[&](){
            OFFSET=0;
            stateStack.pop();
            Symbol symbol(M);
            symbolStack.push_back(symbol);
        }},
        {N,{ONE},[&](){
            stateStack.pop();
            Symbol symbol(N);
            symbol.quad=to_string(nxq);
            symbolStack.push_back(symbol);
        }},
        //变量声明部分
        {VARIABLES,{VARIABLES,VARIABLE,SEMICOLON},[&](){
            int repeat=3;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(VARIABLES);
            symbolStack.push_back(symbol);
        }},
        {VARIABLES,{VARIABLE,SEMICOLON},[&](){
            int repeat=2;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(VARIABLES);
            symbolStack.push_back(symbol);
        }},
        {T,{INTSYM},[&](){
            symbolStack.pop_back();
            stateStack.pop();
            Symbol symbol(T);
            symbol.type=SYMINT;
            symbol.width=4;
            symbolStack.push_back(symbol);
        }},
        {T,{DOUBLESYM},[&](){
            symbolStack.pop_back();
            stateStack.pop();
            Symbol symbol(T);
            symbol.type=SYMDOUBLE;
            symbol.width=8;
            symbolStack.push_back(symbol);
        }},
        {ID,{IDENT},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(ID);
            symbol.name=numAndId.front();
            numAndId.pop_front();
            symbolStack.push_back(symbol);
        }},
        {VARIABLE,{T,ID},[&](){
            int repeat=2;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            auto t=symbolStack.back();
            symbolStack.pop_back();
            enter(id.name,t.type,OFFSET);
            OFFSET+=t.width;
            Symbol symbol(VARIABLE);
            symbol.type=t.type;
            symbol.width=t.width;
            symbolStack.push_back(symbol);
        }},
        {VARIABLE,{VARIABLE,COMMA,ID},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto varible=symbolStack.back();
            symbolStack.pop_back();
            enter(id.name,varible.type,OFFSET);
            OFFSET+=varible.width;
            Symbol symbol(VARIABLE);
            symbol.type=varible.type;
            symbol.width=varible.width;
            symbolStack.push_back(symbol);
        }},
        //语句部分
        {STATEMENT,{ASSIGN},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(STATEMENT);
            symbol.nextList=mklist();
            symbolStack.push_back(symbol);
        }},
        {STATEMENT,{SCANF},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(STATEMENT);
            symbol.nextList=mklist();
            symbolStack.push_back(symbol);
        }},
        {STATEMENT,{PRINTF},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(STATEMENT);
            symbol.nextList=mklist();
            symbolStack.push_back(symbol);
        }},
        {STATEMENT,{ONE},[&](){
            stateStack.pop();
            Symbol symbol(STATEMENT);
            symbol.nextList=mklist();
            symbolStack.push_back(symbol);
        }},
        {STATEMENT,{LBRACE2,L,SEMICOLON,RBRACE2},[&](){
            int repeat=4;
            while(repeat--){
                stateStack.pop();
            }
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto l=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(STATEMENT);
            symbol.nextList=l.nextList;
            symbolStack.push_back(symbol);
        }},
        {STATEMENT,{WHILESYM,N,B,DOSYM,N,STATEMENT},[&](){
            int repeat=6;
            while(repeat--){
                stateStack.pop();
            }
            auto statement=symbolStack.back();
            symbolStack.pop_back();
            auto n2=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto b=symbolStack.back();
            symbolStack.pop_back();
            auto n1=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(STATEMENT);
            backpatch(statement.nextList,n1.quad);
            backpatch(b.trueList,n2.quad);
            symbol.nextList=b.falseList;
            gen("j","-","-",n1.quad);
            symbolStack.push_back(symbol);
        }},
        {STATEMENT,{IFSYM,B,THENSYM,N,STATEMENT},[&](){
            int repeat=5;
            while(repeat--){
                stateStack.pop();
            }
            auto statement=symbolStack.back();
            symbolStack.pop_back();
            auto n=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto b=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(STATEMENT);
            backpatch(b.trueList,n.quad);
            symbol.nextList=merge(b.falseList,statement.nextList);
            symbolStack.push_back(symbol);
        }},
        {ASSIGN,{ID,EQU,EXPR},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto expr=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto id=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(ASSIGN);
            auto p=lookup(id.name);
            gen("=",expr.place,"-",p);
            symbolStack.push_back(symbol);
        }},
        {L,{L,SEMICOLON,N,STATEMENT},[&](){
            int repeat=4;
            while(repeat--){
                stateStack.pop();
            }
            auto statement=symbolStack.back();
            symbolStack.pop_back();
            auto n=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto l=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(L);
            backpatch(l.nextList,n.quad);
            symbol.nextList=statement.nextList;
            symbolStack.push_back(symbol);
        }},
        {L,{STATEMENT},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto statement=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(L);
            symbol.nextList=statement.nextList;
            symbolStack.push_back(symbol);
        }},
        //数值表达式部分
        {EXPR,{EXPR,OR,ORITEM},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto orItem=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto expr=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(EXPR);
            symbol.place=newTemp(SYMINT);
            symbol.type=SYMINT;
            gen("||",expr.place,orItem.place,symbol.place);
            symbolStack.push_back(symbol);
        }},
        {EXPR,{ORITEM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto orItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(EXPR);
            symbol.place=orItem.place;
            symbol.type=orItem.type;
            symbolStack.push_back(symbol);
        }},
        {ORITEM,{ORITEM,AND,ANDITEM},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto andItem=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto orItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(ORITEM);
            symbol.place=newTemp(SYMINT);
            symbol.type=SYMINT;
            gen("&&",orItem.place,andItem.place,symbol.place);
            symbolStack.push_back(symbol);
        }},
        {ORITEM,{ANDITEM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto andItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(ORITEM);
            symbol.place=andItem.place;
            symbol.type=andItem.type;
            symbolStack.push_back(symbol);
        }},
        {ANDITEM,{NOITEM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto noItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(ANDITEM);
            symbol.place=noItem.place;
            symbol.type=noItem.type;
            symbolStack.push_back(symbol);
        }},
        {ANDITEM,{NOT,NOITEM},[&](){
            int repeat=2;
            while(repeat--){
                stateStack.pop();
            }
            auto noItem=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(ANDITEM);
            symbol.place=newTemp(SYMINT);
            symbol.type=SYMINT;
            gen("!",noItem.place,"-",symbol.place);
            symbolStack.push_back(symbol);
        }},
        {NOITEM,{NOITEM,REL,RELITEM},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto relItem=symbolStack.back();
            symbolStack.pop_back();
            auto rel=symbolStack.back();
            symbolStack.pop_back();
            auto noItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(NOITEM);
            symbol.place=newTemp(SYMINT);
            symbol.type=SYMINT;
            gen(rel.op,noItem.place,relItem.place,symbol.place);
            symbolStack.push_back(symbol);
        }},
        {NOITEM,{RELITEM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto relItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(NOITEM);
            symbol.place=relItem.place;
            symbol.type=relItem.type;
            symbolStack.push_back(symbol);
        }},
        {RELITEM,{RELITEM,PLUS_MINUS,ITEM},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto item=symbolStack.back();
            symbolStack.pop_back();
            auto plusMinus=symbolStack.back();
            symbolStack.pop_back();
            auto relItem=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(RELITEM);
            symbol.place=newTemp(relItem.type);
            symbol.type=relItem.type;
            gen(plusMinus.op,relItem.place,item.place,symbol.place);
            symbolStack.push_back(symbol);
        }},
        {RELITEM,{ITEM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto item=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(RELITEM);
            symbol.place=item.place;
            symbol.type=item.type;
            symbolStack.push_back(symbol);
        }},
        {ITEM,{FACTOR},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto factor=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(ITEM);
            symbol.place=factor.place;
            symbol.type=factor.type;
            symbolStack.push_back(symbol);
        }},
        {ITEM,{ITEM,MUL_DIV,FACTOR},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto factor=symbolStack.back();
            symbolStack.pop_back();
            auto mulDiv=symbolStack.back();
            symbolStack.pop_back();
            auto item=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(ITEM);
            symbol.place=newTemp(factor.type);
            symbol.type=factor.type;
            gen(mulDiv.op,item.place,factor.place,symbol.place);
            symbolStack.push_back(symbol);
        }},
        {FACTOR,{ID},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(FACTOR);
            symbol.place=lookup(id.name);
            symbol.type=lookup_type(id.name);
            symbolStack.push_back(symbol);
        }},
        {FACTOR,{UINT},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(FACTOR);
            symbol.place=newTemp(SYMINT);
            symbol.type=SYMINT;
            gen("=",numAndId.front(),"-",symbol.place);
            numAndId.pop_front();
            symbolStack.push_back(symbol);
        }},
        {FACTOR,{UDOUBLE},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(FACTOR);
            symbol.place=newTemp(SYMDOUBLE);
            symbol.type=SYMDOUBLE;
            gen("=",numAndId.front(),"-",symbol.place);
            numAndId.pop_front();
            symbolStack.push_back(symbol);
        }},
        {FACTOR,{LBRACE1,EXPR,RBRACE1},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            symbolStack.pop_back();
            auto expr=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(FACTOR);
            symbol.place=expr.place;
            symbol.type=expr.type;
            symbolStack.push_back(symbol);
        }},
        {FACTOR,{PLUS_MINUS,FACTOR},[&](){
            int repeat=2;
            while(repeat--){
                stateStack.pop();
            }
            auto factor=symbolStack.back();
            symbolStack.pop_back();
            auto plusMinus=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(FACTOR);
            symbol.place=newTemp(factor.type);
            symbol.type=factor.type;
            gen(plusMinus.op,"0",factor.place,symbol.place);
            symbolStack.push_back(symbol);
        }},
        //条件控制表达式
        {B,{B,OR,N,BORTERM},[&](){
            int repeat=4;
            while(repeat--){
                stateStack.pop();
            }
            auto bOrTerm=symbolStack.back();
            symbolStack.pop_back();
            auto n=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto b=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(B);
            backpatch(b.falseList,n.quad);
            symbol.trueList=merge(b.trueList,bOrTerm.trueList);
            symbol.falseList=bOrTerm.falseList;
            symbolStack.push_back(symbol);
        }},
        {B,{BORTERM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto bOrTerm=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(B);
            symbol.trueList=bOrTerm.trueList;
            symbol.falseList=bOrTerm.falseList;
            symbolStack.push_back(symbol);
        }},
        {BORTERM,{BORTERM,AND,N,BANDTERM},[&](){
            int repeat=4;
            while(repeat--){
                stateStack.pop();
            }
            auto bAndTerm=symbolStack.back();
            symbolStack.pop_back();
            auto n=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto bOrTerm=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(BORTERM);
            backpatch(bOrTerm.trueList,n.quad);
            symbol.falseList=merge(bOrTerm.falseList,bAndTerm.falseList);
            symbol.trueList=bAndTerm.trueList;
            symbolStack.push_back(symbol);
        }},
        {BORTERM,{BANDTERM},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            Symbol symbol(BORTERM);
            auto bAndTerm=symbolStack.back();
            symbolStack.pop_back();
            symbol.trueList=bAndTerm.trueList;
            symbol.falseList=bAndTerm.falseList;
            symbolStack.push_back(symbol);
        }},
        {BANDTERM,{LBRACE1,B,RBRACE1},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            symbolStack.pop_back();
            auto b=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(BANDTERM);
            symbol.trueList=b.trueList;
            symbol.falseList=b.falseList;
            symbolStack.push_back(symbol);
        }},
        {BANDTERM,{NOT,BANDTERM},[&](){
            int repeat=2;
            while(repeat--){
                stateStack.pop();
            }
            auto bAndTerm=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            Symbol symbol(BANDTERM);
            symbol.trueList=bAndTerm.falseList;
            symbol.falseList=bAndTerm.trueList;
            symbolStack.push_back(symbol);
        }},
        {BANDTERM,{BFACTOR,REL,BFACTOR},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto bFactor2=symbolStack.back();
            symbolStack.pop_back();
            auto rel=symbolStack.back();
            symbolStack.pop_back();
            auto bFactor1=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(BANDTERM);
            symbol.trueList=mklist(nxq);
            symbol.falseList=mklist(nxq+1);
            gen("j"+rel.op,bFactor1.place,bFactor2.place,"0");
            gen("j","-","-","0");
            symbolStack.push_back(symbol);
        }},
        {BANDTERM,{BFACTOR},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto bFactor=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(BANDTERM);
            symbol.trueList=mklist(nxq);
            symbol.falseList=mklist(nxq+1);
            gen("jnz",bFactor.place,"-","0");
            gen("j","-","-","0");
            symbolStack.push_back(symbol);
        }},
        {BFACTOR,{UINT},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(BFACTOR);
            symbol.place=newTemp(SYMINT);
            symbol.type=SYMINT;
            gen("=",numAndId.front(),"-",symbol.place);
            numAndId.pop_front();
            symbolStack.push_back(symbol);
        }},
        {BFACTOR,{UDOUBLE},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(BFACTOR);
            symbol.place=newTemp(SYMDOUBLE);
            symbol.type=SYMDOUBLE;
            gen("=",numAndId.front(),"-",symbol.place);
            numAndId.pop_front();
            symbolStack.push_back(symbol);
        }},
        {BFACTOR,{ID},[&](){
            int repeat=1;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            Symbol symbol(BFACTOR);
            symbol.place=lookup(id.name);
            symbol.type=lookup_type(id.name);
            symbolStack.push_back(symbol);
        }},
        //运算符
        {PLUS_MINUS,{PLUS},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(PLUS_MINUS);
            symbol.op="+";
            symbolStack.push_back(symbol);
        }},
        {PLUS_MINUS,{MINUS},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(PLUS_MINUS);
            symbol.op="-";
            symbolStack.push_back(symbol);
        }},
        {MUL_DIV,{TIMES},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(MUL_DIV);
            symbol.op="*";
            symbolStack.push_back(symbol);
        }},
        {MUL_DIV,{DIVISION},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(MUL_DIV);
            symbol.op="/";
            symbolStack.push_back(symbol);
        }},
        {REL,{EQUEQU},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(REL);
            symbol.op="==";
            symbolStack.push_back(symbol);
        }},
        {REL,{NOTEQU},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(REL);
            symbol.op="!=";
            symbolStack.push_back(symbol);
        }},
        {REL,{LESS},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(REL);
            symbol.op="<";
            symbolStack.push_back(symbol);
        }},
        {REL,{LE},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(REL);
            symbol.op="<=";
            symbolStack.push_back(symbol);
        }},
        {REL,{GREATER},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(REL);
            symbol.op=">";
            symbolStack.push_back(symbol);
        }},
        {REL,{GE},[&](){
            int repeat=1;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(REL);
            symbol.op=">=";
            symbolStack.push_back(symbol);
        }},
        //读写语句
        {SCANF,{SCANF_BEGIN,RBRACE1},[&](){
            int repeat=2;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(SCANF);
            symbolStack.push_back(symbol);
        }},
        {SCANF_BEGIN,{SCANF_BEGIN,COMMA,ID},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto p=lookup(id.name);
            gen("R","-","-",p);
            Symbol symbol(SCANF_BEGIN);
            symbolStack.push_back(symbol);
        }},
        {SCANF_BEGIN,{SCANFSYM,LBRACE1,ID},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto p=lookup(id.name);
            gen("R","-","-",p);
            Symbol symbol(SCANF_BEGIN);
            symbolStack.push_back(symbol);
        }},
        {PRINTF,{PRINTF_BEGIN,RBRACE1},[&](){
            int repeat=2;
            while(repeat--){
                symbolStack.pop_back();
                stateStack.pop();
            }
            Symbol symbol(PRINTF);
            symbolStack.push_back(symbol);
        }},
        {PRINTF_BEGIN,{PRINTFSYM,LBRACE1,ID},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto p=lookup(id.name);
            gen("W","-","-",p);
            Symbol symbol(PRINTF_BEGIN);
            symbolStack.push_back(symbol);
        }},
        {PRINTF_BEGIN,{PRINTF_BEGIN,COMMA,ID},[&](){
            int repeat=3;
            while(repeat--){
                stateStack.pop();
            }
            auto id=symbolStack.back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            symbolStack.pop_back();
            auto p=lookup(id.name);
            gen("W","-","-",p);
            Symbol symbol(PRINTF_BEGIN);
            symbolStack.push_back(symbol);
        }},
        
    };

    for(auto i=0;i<grammar.size();i++){
        leftToRight[grammar[i].left].push_back(i);
    }
    getFirst();
    // getFollow();
    getC();
    getTable();
}