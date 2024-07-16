#ifndef _LEXER_H
#define _LEXER_H
#include<bits/stdc++.h>
#include"struct.h"
using namespace std;

class Lex
{
public:
    list<Type> getAllToken(string &input,list<string> &numAndId);
private:
    enum State
    {
        INIT,
        READING_WORD,
        READING_NUMBER,
    };
    State state = INIT;
    int currentPos = 0;
    const map<string, Type> endsymbols = {
        {"int", INTSYM},
        {"double", DOUBLESYM},
        {"scanf", SCANFSYM},
        {"printf", PRINTFSYM},
        {"if", IFSYM},
        {"then", THENSYM},
        {"while", WHILESYM},
        {"do", DOSYM},
        {"=", EQU},
        {"==", EQUEQU},
        {">", GREATER},
        {">=", GE},
        {"<", LESS},
        {"<=", LE},
        {"||", OR},
        {"&&", AND},
        {"!", NOT},
        {"!=", NOTEQU},
        {"+", PLUS},
        {"-", MINUS},
        {"*", TIMES},
        {"/", DIVISION},
        {",", COMMA},
        {"(", LBRACE1},
        {")", RBRACE1},
        {"{", LBRACE2},
        {"}", RBRACE2},
        {";", SEMICOLON}
    };
    bool isDigit(char c);
    bool isLetter(char c);
    bool canInWord(char c);
    bool isSymbol(char c);
    bool isCanEquSymbol(char c);
    bool isMustRepeatSymbol(char c);
    bool isSingleSymbol(char c);
    void preProcess(string &input);
    string getToken(string &input);
    pair<string, Type> parserToken(string &token);
};
#endif