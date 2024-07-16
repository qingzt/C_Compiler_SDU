#include<bits/stdc++.h>
using namespace std;

enum State{
    INIT,
    READING_WORD,
    READING_NUMBER,
};

const map<string,string> keywords = {
    {"int","INTSYM"},
    {"double","DOUBLESYM"},
    {"scanf","SCANFSYM"},
    {"printf","PRINTFSYM"},
    {"if","IFSYM"},
    {"then","THENSYM"},
    {"while","WHILESYM"},
    {"do","DOSYM"}
};

const map<string,string> symbols = {
    {"=","AO"},
    {"==","RO"},
    {">","RO"},
    {">=","RO"},
    {"<","RO"},
    {"<=","RO"},
    {"||","LO"},
    {"&&","LO"},
    {"!","LO"},
    {"!=","RO"},
    {"+","PLUS"},
    {"-","MINUS"},
    {"*","TIMES"},
    {"/","DIVISION"},
    {",","COMMA"},
    {"(","BRACE"},
    {")","BRACE"},
    {"{","BRACE"},
    {"}","BRACE"},
    {";","SEMICOLON"}
};

string input;
string cuttentToken;
State state=INIT;
int currentPos=0;
list<pair<string,string>> output;

bool isDigit(char c)
{
    return c>='0'&&c<='9';
}

bool isLetter(char c)
{
    return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
}

bool canInWord(char c)
{
    return isLetter(c)||isDigit(c);
}

bool isSymbol(char c)
{
    return c=='+'||c=='-'||c=='*'||c=='/'||c=='='||c=='>'||c=='<'||c=='!'||c=='&'||c=='|';
}

bool isCanEquSymbol(char c)
{
    return c=='='||c=='>'||c=='<'||c=='!';
}

bool isMustRepeatSymbol(char c)
{
    return c=='&'||c=='|';
}

bool isSingleSymbol(char c)
{
    return c=='{'||c=='}'||c==';'||c=='('||c==')'||c==','||c=='+'||c=='-'||c=='*'||c=='/';
}

void getInput()
{
    string line;
    while(getline(cin, line))
    {
        input += line + "\n";
    }
}

void preProcess()
{
    string temp;
    for(int i = 0; i < input.size(); i++)
    {
        if(input[i]=='/'&&input[i+1]=='*')
        {
            i+=2;
            while(input[i]!='*'||input[i+1]!='/')
            {
                i++;
            }
            i+=2;
        }
        temp+=input[i];
    }
    input=temp;
    temp="";
    for(int i = 0; i < input.size(); i++)
    {
        if(input[i]=='/'&&input[i+1]=='/')
        {
            while(input[i]!='\n')
            {
                i++;
            }
        }
        temp+=input[i];
    }
    input=temp;
    temp="";
    for(int i = 0; i < input.size(); i++)
    {
        if(input[i]=='\n'||input[i]=='\t'||input[i]=='\r')
        {
            input[i]=' ';
        }
    }
    temp="";
    for(int i = 0; i < input.size(); i++)
    {
        if(input[i]==' '&&input[i+1]==' ')
        {
            continue;
        }
        temp+=input[i];
    }
    input=temp;
}

string getToken()
{
    string token;
    state = INIT;
    while(input[currentPos]==' '&&currentPos<input.size())
    {
        currentPos++;
    }
    while(currentPos<input.size()){
        switch (state)
        {
        case INIT:
            if(isLetter(input[currentPos]))
            {
                state=READING_WORD;
                token+=input[currentPos];
            }
            else if(isDigit(input[currentPos])||input[currentPos]=='.')
            {
                state=READING_NUMBER;
                token+=input[currentPos];
            }
            else if(isCanEquSymbol(input[currentPos]))
            {
                token+=input[currentPos];
                currentPos++;
                if(input[currentPos]=='=')
                {
                    token+=input[currentPos];
                    currentPos++;
                }
                return token;
            }
            else if(isMustRepeatSymbol(input[currentPos]))
            {
                token+=input[currentPos];
                currentPos++;
                if(input[currentPos]==input[currentPos-1])
                {
                    token+=input[currentPos];
                    currentPos++;
                }
                return token;
            }
            else if(isSingleSymbol(input[currentPos]))
            {
                token+=input[currentPos];
                currentPos++;
                return token;
            }
            else{
                cout<<"Unrecognizable characters.";
                exit(0);
            }
            currentPos++;
            break;
        case READING_WORD:
            if(isLetter(input[currentPos])||isDigit(input[currentPos]))
            {
                token+=input[currentPos];
            }
            else
            {
                return token;
            }
            currentPos++;
            break;
        case READING_NUMBER:
            if(isDigit(input[currentPos])||input[currentPos]=='.')
            {
                token+=input[currentPos];
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

void parserToken(string &token)
{
    pair<string,string> temp;
    temp.first=token;
    if(keywords.find(token)!=keywords.end())
    {
        temp.second=keywords.at(token);
    }
    else if(symbols.find(token)!=symbols.end())
    {
        temp.second=symbols.at(token);
    }
    else if(isLetter(token[0]))
    {
        temp.second="IDENT";
    }
    else if(isDigit(token[0])||token[0]=='.')
    {
        if(token[0]=='.'||token[token.size()-1]=='.'){
            cout<<"Malformed number: Decimal point at the beginning or end of a floating point number.";
            exit(0);
        }
        if(token[0]=='0')
        {
            if(token.size()>1)
            {
                if(token[1]!='.')
                {
                    cout<<"Malformed number: Leading zeros in an integer.";
                    exit(0);
                }
            }
        }
        bool hasDot=false;
        for(int i = 0; i < token.size(); i++)
        {
            if(token[i]=='.')
            {
                if(hasDot)
                {
                    cout<<"Malformed number: More than one decimal point in a floating point number.";
                    exit(0);
                }
                hasDot=true;
            }
            else if(!isDigit(token[i]))
            {
                cout<<"Malformed number: Non-digit character in a number.";
                exit(0);
            }
        }
        if(hasDot)
        {
            temp.second="DOUBLE";
        }
        else
        {
            temp.second="INT";
        }
    }
    output.push_back(temp);
}

int main()
{
    getInput();
    preProcess();
    while(true){
        string token=getToken();
        if(token=="")
        {
            break;
        }
        parserToken(token);
    }
    for(auto i:output)
    {
        cout<<i.first<<" "<<i.second<<endl;
    }
    return 0;
}