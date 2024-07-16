#include "lexer.h"
using namespace std;

list<Type> Lex::getAllToken(string &input,list<string> &numAndId)
{
    list<Type> output;
    currentPos = 0;
    preProcess(input);
    while (currentPos < (int)input.size())
    {
        string token = getToken(input);
        if (token.empty())
        {
            continue;
        }
        auto res=parserToken(token);
        output.push_back(res.second);
        if(res.second==IDENT)
        {
            numAndId.push_back(res.first);
        }
        else if(res.second==UINT){
            int num=stoi(res.first);
            numAndId.push_back(to_string(num));
        }
        else if(res.second==UDOUBLE){
            double num=stod(res.first);
            numAndId.push_back(to_string(num));
        }
    }
    return output;
}

void Lex::preProcess(string &input)
{
    string temp;
    for (int i = 0; i < (int)input.size(); i++)
    {
        if (input[i] == '/' && input[i + 1] == '*')
        {
            i += 2;
            while (input[i] != '*' || input[i + 1] != '/')
            {
                i++;
            }
            i += 2;
        }
        temp += input[i];
    }
    input = temp;
    temp = "";
    for (int i = 0; i < (int)input.size(); i++)
    {
        if (input[i] == '/' && input[i + 1] == '/')
        {
            while (input[i] != '\n')
            {
                i++;
            }
        }
        temp += input[i];
    }
    input = temp;
    temp = "";
    for (int i = 0; i < (int)input.size(); i++)
    {
        if (input[i] == '\n' || input[i] == '\t' || input[i] == '\r')
        {
            input[i] = ' ';
        }
    }
    temp = "";
    for (int i = 0; i < (int)input.size(); i++)
    {
        if (input[i] == ' ' && input[i + 1] == ' ')
        {
            continue;
        }
        temp += input[i];
    }
    input = temp;
}

string Lex::getToken(string &input)
{
    string token;
    state = INIT;
    while (input[currentPos] == ' ' && currentPos < (int)input.size())
    {
        currentPos++;
    }
    while (currentPos < (int)input.size())
    {
        switch (state)
        {
        case INIT:
            if (isLetter(input[currentPos]))
            {
                state = READING_WORD;
                token += input[currentPos];
            }
            else if (isDigit(input[currentPos]) || input[currentPos] == '.')
            {
                state = READING_NUMBER;
                token += input[currentPos];
            }
            else if (isCanEquSymbol(input[currentPos]))
            {
                token += input[currentPos];
                currentPos++;
                if (input[currentPos] == '=')
                {
                    token += input[currentPos];
                    currentPos++;
                }
                return token;
            }
            else if (isMustRepeatSymbol(input[currentPos]))
            {
                token += input[currentPos];
                currentPos++;
                if (input[currentPos] == input[currentPos - 1])
                {
                    token += input[currentPos];
                    currentPos++;
                }
                return token;
            }
            else if (isSingleSymbol(input[currentPos]))
            {
                token += input[currentPos];
                currentPos++;
                return token;
            }
            else
            {
                cout << "Unrecognizable characters.";
                exit(0);
            }
            currentPos++;
            break;
        case READING_WORD:
            if (isLetter(input[currentPos]) || isDigit(input[currentPos]))
            {
                token += input[currentPos];
            }
            else
            {
                return token;
            }
            currentPos++;
            break;
        case READING_NUMBER:
            if (isDigit(input[currentPos]) || input[currentPos] == '.')
            {
                token += input[currentPos];
            }
            else
            {
                return token;
            }
            currentPos++;
            break;
        default:
            break;
        }
    }
    return token;
}

pair<string,Type> Lex::parserToken(string &token)
{
    pair<string, Type> temp;
    temp.first = token;
    if (endsymbols.find(token) != endsymbols.end())
    {
        temp.second = endsymbols.at(token);
    }
    else if (isLetter(token[0]))
    {
        temp.second = IDENT;
    }
    else if (isDigit(token[0]) || token[0] == '.')
    {
        if (token[0] == '.' || token[token.size() - 1] == '.')
        {
            cout << "Malformed number: Decimal point at the beginning or end of a floating point number.";
            exit(0);
        }
        if (token[0] == '0')
        {
            if (token.size() > 1)
            {
                if (token[1] != '.')
                {
                    cout << "Malformed number: Leading zeros in an integer.";
                    exit(0);
                }
            }
        }
        bool hasDot = false;
        for (int i = 0; i < (int)token.size(); i++)
        {
            if (token[i] == '.')
            {
                if (hasDot)
                {
                    cout << "Malformed number: More than one decimal point in a floating point number.";
                    exit(0);
                }
                hasDot = true;
            }
            else if (!isDigit(token[i]))
            {
                cout << "Malformed number: Non-digit character in a number.";
                exit(0);
            }
        }
        if (hasDot)
        {
            temp.second = UDOUBLE;
        }
        else
        {
            temp.second = UINT;
        }
    }
    return temp;
}

bool Lex::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool Lex::isLetter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool Lex::canInWord(char c)
{
    return isLetter(c) || isDigit(c);
}
bool Lex::isSymbol(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '>' || c == '<' || c == '!' || c == '&' || c == '|';
}

bool Lex::isCanEquSymbol(char c)
{
    return c == '=' || c == '>' || c == '<' || c == '!';
}

bool Lex::isMustRepeatSymbol(char c)
{
    return c == '&' || c == '|';
}

bool Lex::isSingleSymbol(char c)
{
    return c == '{' || c == '}' || c == ';' || c == '(' || c == ')' || c == ',' || c == '+' || c == '-' || c == '*' || c == '/';
}