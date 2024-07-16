#include"parser.h"
#include"lexer.h"
using namespace std;

string getInput()
{
    string input;
    string line;
    while(getline(cin, line))
    {
        input += line + "\n";
    }
    return input;
}

int main()
{
    // freopen("input.txt", "r", stdin);
    Parser parser;
    string input = getInput();
    Lex lex;
    auto tokens = lex.getAllToken(input,parser.numAndId);
    parser.run(tokens);
    parser.output();
    return 0;
}