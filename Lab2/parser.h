#ifndef _PARSER_H
#define _PARSER_H
#include"lexer.h"
#include"struct.h"
using namespace std;
class Parser
{
private:
    int OFFSET = 0;
    int nxq = 0;
    int tempIndex = 0;

    list<Symbol> symbolStack;
    stack<int> stateStack;
    list<Type> input;

    vector<Expression> grammar;
    map<Type, vector<int>> leftToRight;
    map<Type, set<Type>> first;
    map<Type, set<Type>> follow;
    set<pair<I,int>,CCompare> C;
    map<int,I> int2I;
    set<StateTrans> StateTransSet;
    std::map<int, map<Type,function<void(void)>>> analyticalTable;
    vector<SymbolTableItem> symbolTable;
    map<string,int> symbolTableIndex;
    vector<Quardruple> QuardrupleTable;
    vector<Type> allTypes = {
        Type::START,
        Type::ONE,
        Type::INTSYM,
        Type::DOUBLESYM,
        Type::SCANFSYM,
        Type::PRINTFSYM,
        Type::IFSYM,
        Type::THENSYM,
        Type::WHILESYM,
        Type::DOSYM,
        Type::EQU,
        Type::GREATER,
        Type::GE,
        Type::LESS,
        Type::LE,
        Type::OR,
        Type::AND,
        Type::NOT,
        Type::NOTEQU,
        Type::PLUS,
        Type::MINUS,
        Type::TIMES,
        Type::DIVISION,
        Type::COMMA,
        Type::LBRACE1,
        Type::RBRACE1,
        Type::LBRACE2,
        Type::RBRACE2,
        Type::SEMICOLON,
        Type::ID,
        Type::IDENT,
        Type::UDOUBLE,
        Type::UINT,
        Type::EQUEQU,
        Type::PROG,
        Type::SUBPROG,
        Type::M,
        Type::N,
        Type::VARIABLES,
        Type::STATEMENT,
        Type::VARIABLE,
        Type::T,
        Type::ASSIGN,
        Type::SCANF,
        Type::PRINTF,
        Type::L,
        Type::B,
        Type::EXPR,
        Type::ORITEM,
        Type::ANDITEM,
        Type::RELITEM,
        Type::NOITEM,
        Type::ITEM,
        Type::FACTOR,
        Type::BORTERM,
        Type::BANDTERM,
        Type::BFACTOR,
        Type::PLUS_MINUS,
        Type::MUL_DIV,
        Type::REL,
        Type::SCANF_BEGIN,
        Type::PRINTF_BEGIN,
        Type::END,
    };
#ifdef DEBUG
    map<Type,string> type2string={
        {Type::INTSYM,"INTSYM"},
        {Type::DOUBLESYM,"DOUBLESYM"},
        {Type::SCANFSYM,"SCANFSYM"},
        {Type::PRINTFSYM,"PRINTFSYM"},
        {Type::IFSYM,"IFSYM"},
        {Type::THENSYM,"THENSYM"},
        {Type::WHILESYM,"WHILESYM"},
        {Type::DOSYM,"DOSYM"},
        {Type::EQU,"EQU"},
        {Type::GREATER,"GREATER"},
        {Type::GE,"GE"},
        {Type::LESS,"LESS"},
        {Type::LE,"LE"},
        {Type::OR,"OR"},
        {Type::AND,"AND"},
        {Type::NOT,"NOT"},
        {Type::NOTEQU,"NOTEQU"},
        {Type::PLUS,"PLUS"},
        {Type::MINUS,"MINUS"},
        {Type::TIMES,"TIMES"},
        {Type::DIVISION,"DIVISION"},
        {Type::COMMA,"COMMA"},
        {Type::LBRACE1,"LBRACE1"},
        {Type::RBRACE1,"RBRACE1"},
        {Type::LBRACE2,"LBRACE2"},
        {Type::RBRACE2,"RBRACE2"},
        {Type::SEMICOLON,"SEMICOLON"},
        {Type::ID,"ID"},
        {Type::IDENT,"IDENT"},
        {Type::UDOUBLE,"UDOUBLE"},
        {Type::UINT,"UINT"},
        {Type::EQUEQU,"EQUEQU"},
        {Type::PROG,"PROG"},
        {Type::SUBPROG,"SUBPROG"},
        {Type::M,"M"},
        {Type::N,"N"},
        {Type::VARIABLES,"VARIABLES"},
        {Type::STATEMENT,"STATEMENT"},
        {Type::VARIABLE,"VARIABLE"},
        {Type::T,"T"},
        {Type::ASSIGN,"ASSIGN"},
        {Type::SCANF,"SCANF"},
        {Type::PRINTF,"PRINTF"},
        {Type::L,"L"},
        {Type::B,"B"},
        {Type::EXPR,"EXPR"},
        {Type::ORITEM,"ORITEM"},
        {Type::ANDITEM,"ANDITEM"},
        {Type::RELITEM,"RELITEM"},
        {Type::NOITEM,"NOITEM"},
        {Type::ITEM,"ITEM"},
        {Type::FACTOR,"FACTOR"},
        {Type::BORTERM,"BORTERM"},
        {Type::BANDTERM,"BANDTERM"},
        {Type::BFACTOR,"BFACTOR"},
        {Type::PLUS_MINUS,"PLUS_MINUS"},
        {Type::MUL_DIV,"MUL_DIV"},
        {Type::REL,"REL"},
        {Type::SCANF_BEGIN,"SCANF_BEGIN"},
        {Type::PRINTF_BEGIN,"PRINTF_BEGIN"},
        {Type::END,"END"},
    };
#endif
    set<int> endsymbosSet = {
        Type::INTSYM,
        Type::DOUBLESYM,
        Type::SCANFSYM,
        Type::PRINTFSYM,
        Type::IFSYM,
        Type::THENSYM,
        Type::WHILESYM,
        Type::DOSYM,
        Type::EQU,
        Type::EQUEQU,
        Type::GREATER,
        Type::GE,
        Type::LESS,
        Type::LE,
        Type::OR,
        Type::AND,
        Type::NOT,
        Type::NOTEQU,
        Type::PLUS,
        Type::MINUS,
        Type::TIMES,
        Type::DIVISION,
        Type::COMMA,
        Type::LBRACE1,
        Type::RBRACE1,
        Type::LBRACE2,
        Type::RBRACE2,
        Type::SEMICOLON,
        Type::UDOUBLE,
        Type::UINT,
        Type::IDENT,
    };
    bool isEndSymbol(Type type);
    void getFirst();
    void getFollow();
    set<Type> getFirstForCandidate(vector<Type> candidate);  
    I Clousre(ClosureItem sourceItem);
    I Go(I i,Type a);
    void getC();
    void getTable();
    void backpatch(string p, string t);
    void gen(string op, string arg1, string arg2, string result);
    void enter(string name, int type, int offset);
    string newTemp(int type);
    int lookup_type(string name);
#ifdef DEBUG
    void printExp(Expression exp);
    void printSymbolStack();
#endif
    string lookup(string name);
    string merge(string p1, string p2);
    string mklist(int i);
    string mklist();
public:
    Parser();
    void run(list<Type> oinput);
    void output();
    list<string> numAndId;
};
#endif