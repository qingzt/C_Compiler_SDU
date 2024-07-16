#include "parser.h"
using namespace std;

// #define DEBUG

#ifdef DEBUG
void Parser::printExp(Expression expression)
{
    cout<<"USE: ";
    cout << type2string[expression.left] << " -> ";
    for (auto type : expression.right)
    {
        cout << type2string[type] << " ";
    }
    cout << endl;
}

void Parser::printSymbolStack()
{
    for (auto &symbol : symbolStack)
    {
        cout << type2string[symbol.rtype] << " ";
    }
    cout << endl;
}
#endif
bool Parser:: isEndSymbol(Type type)
{
    return endsymbosSet.find(type) != endsymbosSet.end();
}

void Parser::getFirst()
{
    for (auto type : allTypes)
    {
        if (isEndSymbol(type))
        {
            first[type].insert(type);
        }
        else{
            first[type]=set<Type>();
        }
    }
    while (true)
    {
        bool isChanged = false;
        for (auto &expression : grammar)
        {
            set<Type> result = getFirstForCandidate(expression.right);
            if (first[expression.left].size() == 0&&result.size()!=0)
            {
                first[expression.left] = result;
                isChanged = true;
            }
            else
            {
                for (auto &type : result)
                {
                    auto res=first[expression.left].insert(type);
                    if(res.second){
                        isChanged = true;
                    }
                }
            }
        }
        if (!isChanged)
        {
            break;
        }
    }
}

void Parser::getFollow()
{
    for(auto type:allTypes){
        if(!isEndSymbol(type)){
            follow[type]=set<Type>();
        }
    }
    follow[START]={END};
    for(auto &expression:grammar){
        int k=expression.right.size()-1;
        for(int i=0;i<k-1;i++){
            if(!isEndSymbol(expression.right[i])){
                set<Type> result = getFirstForCandidate(vector<Type>(expression.right.begin()+i+1,expression.right.end()));
                result.erase(ONE);
                if(result.size()!=0){
                    follow[expression.right[i]].insert(result.begin(),result.end());
                }
            }
        }
    }
    bool isChanged=true;
    while(isChanged){
        isChanged=false;
        for(auto &expression:grammar){
            int k=expression.right.size()-1;
            if(!isEndSymbol(expression.right[k])){
                for(auto &type:follow[expression.left]){
                    auto res=follow[expression.right[k]].insert(type);
                    if(res.second){
                        isChanged=true;
                    }
                }
            }
            else{
                continue;
            }
            for(int i=k-1;i>=0;i--){
                if(!isEndSymbol(expression.right[i])&&first[expression.right[i+1]].find(ONE)!=first[expression.right[i+1]].end()){
                    for(auto &type:follow[expression.left]){
                        auto res=follow[expression.right[i]].insert(type);
                        if(res.second){
                            isChanged=true;
                        }
                    }
                }
                else{
                    break;
                }
            }
        }
    }
}

set<Type> Parser::getFirstForCandidate(vector<Type> candidate)
{
    set<Type> result;
    if ((candidate.size() == 1 )&& (candidate[0]==ONE))
    {
        result.insert(ONE);
        return result;
    }
    result = first[candidate[0]];
    result.erase(ONE);
    bool isEmpty = true;
    for (int i = 1; i < (int)candidate.size(); i++)
    {
        if (first[candidate[i - 1]].find(ONE) != first[candidate[i - 1]].end())
        {
            result.insert(first[candidate[i]].begin(), first[candidate[i]].end());
            result.erase(ONE);
        }
        else
        {
            isEmpty = false;
            break;
        }
    }
    if (isEmpty && first[candidate[candidate.size() - 1]].find(ONE) != first[candidate[candidate.size() - 1]].end())
    {
        result.insert(ONE);
    }
    return result;
}

I Parser::Clousre(ClosureItem sourceItem)
{
    I result;
    queue<ClosureItem> q;
    q.push(sourceItem);
    result.insert(sourceItem);
    while(!q.empty()){
        ClosureItem item=q.front();
        q.pop();
        // result.insert(item);
        if(item.dot<grammar[item.expressionIndex].right.size()){
            Type nextType=grammar[item.expressionIndex].right[item.dot];
            if(!isEndSymbol(nextType)){
                for(auto &exprIt:leftToRight[nextType]){
                    auto expressionIndex=exprIt;
                    ClosureItem newItem={expressionIndex,0,{}};
                    if(item.dot==grammar[item.expressionIndex].right.size()-1){
                        newItem.lookAhead.insert(item.lookAhead.begin(),item.lookAhead.end());
                    }
                    else{
                        set<Type> firstSet=first[grammar[item.expressionIndex].right[item.dot+1]];
                        newItem.lookAhead.insert(firstSet.begin(),firstSet.end());
                    }
                    auto res=result.insert(newItem);
                    if(res.second){
                        q.push(newItem);
                    }
                    else if(!res.second){
                        if(res.first->lookAhead!=newItem.lookAhead){
                            auto temp=*res.first;
                            result.erase(res.first);
                            bool changed=false;
                            for(auto &type:newItem.lookAhead){
                                auto res2=temp.lookAhead.insert(type);
                                if(res2.second){
                                    changed=true;
                                }
                            }
                            if(changed){
                                result.insert(temp);
                                q.push(temp);
                            }
                            else{
                                result.insert(temp);
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

I Parser::Go(I i,Type a)
{
    I result;
    for(auto &item:i){
        if(item.dot<grammar[item.expressionIndex].right.size()&&grammar[item.expressionIndex].right[item.dot]==a){
            ClosureItem newItem={item.expressionIndex,item.dot+1,item.lookAhead};
            I closure=Clousre(newItem);
            result.insert(closure.begin(),closure.end());
        }
    }
    return result;
}

void Parser::getC()
{
    int state=0;
    ClosureItem startItem={(leftToRight[START].begin())[0],0,{END}};
    I I0=Clousre(startItem);
    C.insert({I0,state});
    int2I[state]=C.begin()->first;
    state++;
    stateStack.push(C.begin()->second);
    list<set<pair<I,int>>::iterator> closureQueue;
    closureQueue.push_back(C.begin());
    while(!closureQueue.empty()){
        auto cur=closureQueue.front();
        auto i=cur->first;
        auto curState=cur->second;
        closureQueue.pop_front();
        for(auto &item:i){
            if(item.dot<grammar[item.expressionIndex].right.size()){
                Type a=grammar[item.expressionIndex].right[item.dot];
                I newI=Go(i,a);
                auto res=C.insert({newI,state});
                if(res.second){
                    StateTransSet.insert({curState,a,state});
                    //cout<<"StateTrans "<<curState<<" "<<a<<" "<<state<<endl;
                    int2I[state]=res.first->first;
                    state++;
                    closureQueue.push_back(res.first);
                }
                else{
                    auto temp=*res.first;
                    auto tempI=temp.first;
                    bool outChanged=false;
                    for(auto &closureIt:newI){//对于新的I中的每一个项目
                        auto res2=tempI.find(closureIt);//在已有的I中查找是否有相同的项目
                        if(res2!=tempI.end()){//如果有
                            bool changed=false;
                            auto tempItem=*res2;
                            for(auto &type:closureIt.lookAhead){
                                auto res3=tempItem.lookAhead.insert(type);
                                if(res3.second){
                                    changed=true;
                                    outChanged=true;
                                }
                            }
                            if(changed){
                                tempI.erase(res2);
                                tempI.insert(tempItem);
                            }
                        }
                    }
                    temp.first=tempI;
                    if(outChanged){
                        C.erase(res.first);
                        auto delIt=find(closureQueue.begin(),closureQueue.end(),res.first);
                        if(delIt!=closureQueue.end()){
                            closureQueue.erase(delIt);
                        }
                        C.insert(temp);
                        closureQueue.push_back(C.find(temp));
                    }
                    StateTransSet.insert({curState,a,temp.second});
                    //cout<<"StateTrans "<<curState<<" "<<a<<" "<<res.first->second<<endl;
                }
            }
        }
    }
}

void Parser::getTable()
{
    for(auto &trans:StateTransSet){
        if(isEndSymbol(trans.go)){
            analyticalTable[trans.oldState][trans.go]=[&](){
                Symbol symbol(input.front());
                input.pop_front();
                symbolStack.push_back(symbol);
                stateStack.push(trans.newState);
            };
        }
        else{
            analyticalTable[trans.oldState][trans.go]=[&](){
                stateStack.push(trans.newState);
            };
        }
    }
    for(auto i=C.begin();i!=C.end();i++){
        for(auto &item:i->first){
            if(item.dot==grammar[item.expressionIndex].right.size()){
                if(grammar[item.expressionIndex].left==START){
                    analyticalTable[i->second][END]=[&](){
                        input.pop_front();
                    };
                }
                else{
                    for(auto &type:item.lookAhead){
                        analyticalTable[i->second][type]=[&](){
                        #ifdef DEBUG
                            printExp(item.expression);
                            cout<<"BEFORE: ";
                            printSymbolStack();
                        #endif
                            grammar[item.expressionIndex].action();
                        #ifdef DEBUG
                            cout<<" AFTER: ";
                            printSymbolStack();
                        #endif
                            auto action=analyticalTable[stateStack.top()][grammar[item.expressionIndex].left];
                            if(action){
                                action();
                            }
                            else{
                                abort();
                            }
                        };
                    }
                }
            }
        }
    }
    
}

void Parser::backpatch(string p,string t)
{
    string q;
    while(p!="null"&&p!="0"){
        q=QuardrupleTable[stoi(p)].result;
        QuardrupleTable[stoi(p)].result=t;
        p=q;
    }
}

string Parser::merge(string p1,string p2)
{
    if(p1=="null"){
        return p2;
    }
    if(p2=="null"){
        return p1;
    }
    string p=p2;
    while(QuardrupleTable[stoi(p)].result!="null"&&QuardrupleTable[stoi(p)].result!="0"){
        p=QuardrupleTable[stoi(p)].result;
    }
    QuardrupleTable[stoi(p)].result=p1;
    return p2;
}

void Parser::gen(string op,string arg1,string arg2,string result)
{
#ifdef DEBUG
    cout<<nxq<<":("<<op<<","<<arg1<<","<<arg2<<","<<result<<")"<<endl;
#endif
    // if(op==""||arg1==""||arg2==""||result==""){
    //     abort();
    // }
    // if(op=="="&&result.find("TB")!=string::npos){
    //     auto symbolIndex=stoi(result.substr(2));
    //     symbolTable[symbolIndex].hasValue=true;
    // }
    // if(arg1.find("TB")!=string::npos){
    //     auto symbolIndex=stoi(arg1.substr(2));
    //     if(!symbolTable[symbolIndex].hasValue){
    //         cout<<"Syntax Error"<<endl;
    //         exit(0);
    //     }
    // }
    // if(arg2.find("TB")!=string::npos){
    //     auto symbolIndex=stoi(arg2.substr(2));
    //     if(!symbolTable[symbolIndex].hasValue){
    //         cout<<"Syntax Error"<<endl;
    //         exit(0);
    //     }
    // }
    if((int)QuardrupleTable.size()<nxq+1){
        QuardrupleTable.push_back(Quardruple(op,arg1,arg2,result));
    }
    else{
        QuardrupleTable[nxq]=Quardruple(op,arg1,arg2,result);
    }
    nxq++;
}

void Parser::enter(string name,int type,int offset)
{
    if(symbolTableIndex.find(name)!=symbolTableIndex.end()){
        cout<<"Syntax Error"<<endl;
        exit(0);
    }
    SymbolTableItem item(name,type,"null",offset);
    symbolTable.push_back(item);
    symbolTableIndex[name]=symbolTable.size()-1;
}

string Parser::newTemp(int type)
{
    string name="T"+to_string(tempIndex++);
    if(type==SYMINT){
        name+="_i";
    }
    else if(type==SYMDOUBLE){
        name+="_d";
    }
    return name;
}

string Parser::mklist(int i)
{
    return to_string(i);
}

string Parser::mklist()
{
    return "null";
}

int Parser::lookup_type(string name)
{
    if(symbolTableIndex.find(name)!=symbolTableIndex.end()){
        return symbolTable[symbolTableIndex[name]].type;
    }
    return -1;
}

string Parser::lookup(string name)
{
    if(symbolTableIndex.find(name)!=symbolTableIndex.end()){
        return "TB"+to_string(symbolTableIndex[name]);
    }
    cout<<"Syntax Error"<<endl;
    exit(0);
    return "null";
}

void Parser::output()
{
    cout<<symbolTable.size()<<endl;
    for(auto &item:symbolTable){
        cout<<item.name<<" "<<item.type<<" "<<item.value<<" "<<item.offset<<endl;
    }
    cout<<tempIndex<<endl;
    cout<<QuardrupleTable.size()<<endl;
    for(size_t i=0;i<QuardrupleTable.size();i++){
        cout<<i<<": ("<<QuardrupleTable[i].op<<","<<QuardrupleTable[i].arg1<<","<<QuardrupleTable[i].arg2<<","<<QuardrupleTable[i].result<<")"<<endl;
    }
}

void Parser::run(list<Type> oinput)
{
    this->input = oinput;
    this->input.push_back(END);
    while (!input.empty())
    {
        auto state = stateStack.top();
        auto symbol = input.front();
        auto action = analyticalTable[state][symbol];
        if (action)
        {
            action();
        }
        else if(symbol!=ONE){
            symbol=ONE;
            action=analyticalTable[state][symbol];
            if(action){
                action();
            }
            else{
                cout<<"Syntax Error"<<endl;
                exit(0);
                break;
            }
        }
        else
        {
            cout << "Syntax Error" << endl;
            exit(0);
            break;
        }
    }
}