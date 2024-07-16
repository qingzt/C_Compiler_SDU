#ifndef _STRUCT_H
#define _STRUCT_H
#include <iostream>
#include <vector>
#include <functional>
#include <set>
#include <map>
#include <list>
#include <stack>
#include <string>
using namespace std;
enum Type
    {
        START,
        ONE,
        INTSYM,
        DOUBLESYM,
        SCANFSYM,
        PRINTFSYM,
        IFSYM,
        THENSYM,
        WHILESYM,
        DOSYM,
        EQU,
        GREATER,
        GE,
        LESS,
        LE,
        OR,
        AND,
        NOT,
        NOTEQU,
        PLUS,
        MINUS,
        TIMES,
        DIVISION,
        COMMA,
        LBRACE1,
        RBRACE1,
        LBRACE2,
        RBRACE2,
        SEMICOLON,
        ID,
        IDENT,
        UDOUBLE,
        UINT,
        EQUEQU,
        PROG,SUBPROG,
        M,
        N,
        VARIABLES,
        STATEMENT,
        VARIABLE,
        T,
        ASSIGN,
        SCANF,
        PRINTF,
        L,
        B,
        EXPR,
        ORITEM,
        ANDITEM,
        RELITEM,
        NOITEM,
        ITEM,
        FACTOR,
        BORTERM,
        BANDTERM,
        BFACTOR,
        PLUS_MINUS,
        MUL_DIV,
        REL,
        SCANF_BEGIN,
        PRINTF_BEGIN,
        END,
    };
    class Symbol
{
public:
    Symbol()
    {
        rtype = Type::ONE;
    }
    Symbol(Type type) : rtype(type){};
    Type rtype;
    string quad = "";
    string name = "";
    string nextList = "";
    string trueList = "";
    string falseList = "";
    int type = 0;
    int width = 0;
    string op = "";
    string place = "";
};
class Expression
{
public:
    Expression(Type left, vector<Type> right, function<void(void)> action) : left(left), right(right), action(action){};
    Type left;
    vector<Type> right;
    function<void(void)> action;
};
struct ClosureItem
{
    int expressionIndex;
    size_t dot;
    set<Type> lookAhead;
    bool operator<(const ClosureItem &item) const
    {
        if (expressionIndex != item.expressionIndex)
        {
            return expressionIndex < item.expressionIndex;
        }
        if (dot != item.dot)
        {
            return dot < item.dot;
        }
        // for(auto i:lookAhead){
        //     if(item.lookAhead.find(i)==item.lookAhead.end()){
        //         return true;
        //     }
        // }
        return false;
    }
    bool operator==(const ClosureItem &item) const
    {
        return (!(item<*this))&&(!(*this<item));
    }
};
typedef set<ClosureItem> I;

struct StateTrans
{
    int oldState;
    Type go;
    int newState;
    StateTrans(int oldState,Type go,int newState):oldState(oldState),go(go),newState(newState){};
    bool operator<(const StateTrans &trans) const
    {
        if(this->oldState!=trans.oldState){
            return this->oldState<trans.oldState;
        }
        else if(this->go!=trans.go){
            return this->go<trans.go;
        }
        return this->newState<trans.newState;
    }
};

struct CCompare {
    bool operator() (const pair<I,int>& a, const pair<I,int>& b) const {
        return a.first < b.first;
    }
};

#define SYMINT 0
#define SYMDOUBLE 1

struct SymbolTableItem{
    string name;
    int type;
    string value;
    int offset;
    bool hasValue=false;
    SymbolTableItem()=default;
    SymbolTableItem(string name,int type,string value,int offset):name(name),type(type),value(value),offset(offset){};
};

struct Quardruple{
    string op;
    string arg1;
    string arg2;
    string result;
    Quardruple(string op,string arg1,string arg2,string result):op(op),arg1(arg1),arg2(arg2),result(result){};
};

#endif