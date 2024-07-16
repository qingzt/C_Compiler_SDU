#include<set>
#include<string>
#include<vector>
#include<map>
#include <iostream>
#include<stdio.h>
#include<sstream>
#include<algorithm>
using namespace std;
//#define DEBUG

struct QuardrupleItem{ //每个四元式中的项目
    string val;        //该项目的值
    int use = -1;      //该项目的待用信息
    bool live = false; //该项目的活跃信息
    QuardrupleItem(string val):val(val){}
    QuardrupleItem(){}
};

struct Quardruple {      //四元式
    QuardrupleItem op;   //操作符
    QuardrupleItem opnd1;//左操作数
    QuardrupleItem opnd2;//右操作数
    QuardrupleItem left; //左值
};



struct symbolTableItem {    //符号表中的项目
    string name="";         //变量名
    string type="";         //变量类型
    string value="";        //变量值
    int offset=-1;          //变量的偏移量
    int use = -1;           //变量的待用信息
    bool isTemp = true;     //是否是临时变量
    bool live = false;      //变量的活跃信息
    symbolTableItem(){}
    symbolTableItem(bool isTemp):isTemp(isTemp){}
};

map<string,string> opMap={  //操作符映射，用在genForTheta函数中
    {"+","add"},
    {"-","sub"},
    {"*","mul"},
    {"/","div"},
    {"=","mov"},
    {"<","cmp"},
    {">","cmp"},
    {"<=","cmp"},
    {">=","cmp"},
    {"==","cmp"},
    {"!=","cmp"},
    {"&&","and"},
    {"||","or"},
    {"!","not"},
};

map<string,string> jThetaMap={ //条件跳转映射，用在genForJTheta函数中
    {"j<","jl"},
    {"j>","jg"},
    {"j<=","jle"},
    {"j>=","jge"},
    {"j==","je"},
    {"j!=","jne"},
};

map<string,string> cmpMap={  //比较指令映射，用在genForTheta函数中
    {"<","setl"},
    {">","setg"},
    {"<=","setle"},
    {">=","setge"},
    {"==","sete"},
    {"!=","setne"},
};

bool isTheta(Quardruple &q){  //判断是否是Theta类型的四元式，要求操作符为+、-、*、/、=、<、>、!、&、|，并且右操作数不为"-"
    return (q.op.val[0]=='+'||q.op.val[0]=='-'||q.op.val[0]=='*'||q.op.val[0]=='/'||q.op.val[0]=='='||q.op.val[0]=='<'||q.op.val[0]=='>'||q.op.val[0]=='!'||q.op.val[0]=='&'||q.op.val[0]=='|')&&(q.opnd2.val!="-");
}

bool isJTheta(Quardruple &q){ //判断是否是JTheta类型的四元式，要求操作符的第一个字符为'j'，并且不是"j"和"jnz"
    return q.op.val[0]=='j'&&q.op.val!="j"&&q.op.val!="jnz";
}

bool isJ(Quardruple &q){   //判断是否是J类型的四元式，要求操作符为"j"
    return q.op.val=="j";
}

bool isJnz(Quardruple &q){  //判断是否是Jnz类型的四元式，要求操作符为"jnz"
    return q.op.val=="jnz";
}

bool isROrW(Quardruple &q){  //判断是否是Read或Write类型的四元式，要求操作符为"W"或"R"
    return q.op.val=="W"||q.op.val=="R";
}

bool isOnlyX(Quardruple &q){ //判断是否是只有左操作数的四元式，要求操作符为"="或"!"，并且右操作数为"-"
    return (q.op.val=="="||q.op.val=="!")&&q.opnd2.val=="-";
}

bool isEnd(Quardruple &q){  //判断是否是End类型的四元式，要求操作符为"End"
    return q.op.val=="End";
}

bool isUNum(string &s){    //判断是否是数字，即UINT或UFLOAT
    return s[0]>='0'&&s[0]<='9';
}


struct AvalItem{          //Aval中的项目
    set<string>  reg;
    set<string>  mem;
};

int InitOffset=0;       //符号表中变量的初始偏移量
int offset=0;           //当前符号表的偏移量
vector<Quardruple> quardruples;   //四元式序列
map<string,symbolTableItem> symbolTable;//符号表
vector<vector<int>> basicBlocks;//基本块，blockBlocks[i]表示第i个基本块中的四元式下标形成的数组
int tempVarCount = 0;      //临时变量的数量
map<string,AvalItem> Aval; //地址描述符
map<string,set<string>> Rval;//寄存器描述符
const vector<string> regs={"R0","R1","R2"};//寄存器集合
vector<set<string>> liveOut;//每个基本块的活跃变量集合
map<string,QuardrupleItem> historyInfo; //历史信息
vector<int> labelFlag;



int findBlockIndex(int index){
    for(int i=0;i<basicBlocks.size();i++){
        if(find(basicBlocks[i].begin(),basicBlocks[i].end(),index)!=basicBlocks[i].end()){
            return i;
            break;
        }
    }
    return -1;
}

void genLable(int quad){ //生成标号的伪代码：
    //int blockIndex=findBlockIndex(quad); //blockIndex=FindBlockIndex（q）；
    if(labelFlag[quad]==0){ //if !labelFlag[quad.block] then
        //cout<<"?"+to_string(blockIndex)<<":"<<endl;
        labelFlag[quad]=1; //labelFlag[quad.block]=true；
    }
}

void getInputForSymbolTable(int lenth)  //输入符号表
{
    string line;
    for(int i = 0; i < lenth; i++)
    {
        getline(cin, line);
        stringstream ss(line);
        symbolTableItem temp(false);
        ss >> temp.name >> temp.type >> temp.value >> temp.offset;
        InitOffset=max(InitOffset,temp.offset);//更新符号表中变量的初始偏移量
        symbolTable["TB"+to_string(i)] = temp;  //将该项目加入符号表
    }
}

void getInputForQuardruples(int lenth)  //输入四元式序列
{
    quardruples.resize(lenth);
    string line;
    for(int i = 0; i < lenth; i++)
    {
        getline(cin, line);
        line=line.substr(line.find(':')+2);
        line=line.substr(1,line.find(')')-1);
        stringstream ss(line);
        vector<string> tokens(4);
        int j=0;
        while(getline(ss, tokens[j], ',')) {
            j++;
        }
        quardruples[i].op = tokens[0];
        quardruples[i].opnd1 = tokens[1];
        quardruples[i].opnd2 = tokens[2];
        quardruples[i].left = tokens[3];
    }
}

void input()  //输入
{
    string line;
    getline(cin, line);
    if(line == "Syntax Error")
    {
        cout << "halt" << endl;
        exit(0);
    }
    int symbolTableSize, quardruplesSize;
    symbolTableSize = stoi(line);
    getInputForSymbolTable(symbolTableSize);
    getline(cin, line);
    tempVarCount = stoi(line);
    getline(cin, line);
    quardruplesSize = stoi(line);
    getInputForQuardruples(quardruplesSize);
    offset=InitOffset; //初始化当前符号表的偏移量
}

void getBasicBlock()  //获取基本块
{
    set<vector<int>> blocks; //基本块集合
    int isEnter[quardruples.size()] = {false};//标记每个四元式是否是基本块的入口
    isEnter[0] = true;        //标记qi为入口语句
    for(int i = 0; i < quardruples.size(); i++) //逆序遍历四元式
    {
        if(isJTheta(quardruples[i])||isJnz(quardruples[i])) //if qi 形如 (jTheta,-,-,qj)或(jnz,-,-,qj)
        {
            int index=stoi(quardruples[i].left.val); //取qj
            isEnter[index] = true;   //标记qj为基本块的入口
            if(i<quardruples.size()-1){//if i<n
                isEnter[i+1] = true; //标记qi+1为基本块的入口
            }
            genLable(index);
        }else if(isJ(quardruples[i])){ //else if qi形如(j,-,-,qj)
            int index=stoi(quardruples[i].left.val);//取qj
            isEnter[index] = true; //标记qj为基本块的入口
            genLable(index);
        }
        else if(quardruples[i].op.val == "End") //if qi形如(End,-,-,-)
        {
            isEnter[quardruples.size()-1] = true;//标记qn为基本块的入口
        }
        else if(isROrW(quardruples[i])){
            isEnter[i] = true;
        }
    }
    int i= 0;
    while(i < quardruples.size()) //顺序遍历四元式
    {
        if(isEnter[i]){  //if qi是基本块的入口
            if(i==quardruples.size()-1){ //if i=n
                vector<int> temp;
                temp.push_back(i);
                blocks.insert(temp); //将qi加入基本块
            }
            if(i+1==quardruples.size()){
                break;
            }
            for(int j = i+1; j < quardruples.size(); j++) //遍历qi之后的四元式
            {
                if(isEnter[j]){  //if qj是基本块的入口
                    vector<int> temp;  //qi,qi+1,...,qj-1加入基本块
                    for(int k = i; k < j; k++)
                    {
                        temp.push_back(k);
                    }
                    blocks.insert(temp);
                    i = j; //i=j
                    break;
                }
                else{
                    if(quardruples[j].op.val[0]=='j'||quardruples[j].op.val=="ret"||quardruples[j].op.val=="End"){ //if qj是停机或转移语句
                        vector<int> temp;       //qi,qi+1,...,qj加入基本块
                        for(int k = i; k <= j; k++)
                        {
                            temp.push_back(k);
                        }
                        blocks.insert(temp);
                        i = j+1;           //i=j+1
                        break;
                    }
                }
            }
        }
        else{
            i++; //i++
        }
    }
    basicBlocks.assign(blocks.begin(), blocks.end());//将基本块集合转换为基本块数组
}

set<string> getUseInfo(vector<int> &block)  //求解待用信息
{
    set<string> res;//当前块出口处的活跃变量集合
    for(auto &i :block){//USE在构造时已经是非待用
        string x=quardruples[i].opnd1.val; //对于左右操作数和左值
        string y=quardruples[i].opnd2.val;
        string z=quardruples[i].left.val;
        if(x[0]=='T'){
            symbolTable[x].use = -1;
            if(!symbolTable[x].isTemp){  //如果x不是临时变量
                symbolTable[x].live = true; //x是活跃的
                if(find(res.begin(),res.end(),x)==res.end()){//将x加入当前块出口处的活跃变量集合
                    res.insert(x);
                }
            }
        }
        if(y[0]=='T'){
            symbolTable[y].use = -1;
            if(!symbolTable[y].isTemp){
                symbolTable[y].live = true;
                if(find(res.begin(),res.end(),y)==res.end()){
                    res.insert(y);
                }
            }
        }
        if(z[0]=='T'){
            symbolTable[z].use = -1;
            if(!symbolTable[z].isTemp){
                symbolTable[z].live = true;
                if(find(res.begin(),res.end(),z)==res.end()){
                    res.insert(z);
                }
            }
        }
    }
    for(int i=block.size()-1;i>=0;i--){ //for i=m…1
        int index=block[i];
        if(quardruples[index].left.val[0]=='T'){ 
            quardruples[index].left.use = symbolTable[quardruples[index].left.val].use;//符号表中变量z的use和live赋值给四元式qi的z变量；
            quardruples[index].left.live = symbolTable[quardruples[index].left.val].live;
            symbolTable[quardruples[index].left.val].live = false; //符号表中变量z的use和live分别置为-和N；
            symbolTable[quardruples[index].left.val].use = -1;
        }
        if(quardruples[index].opnd1.val[0]=='T'){
            quardruples[index].opnd1.use = symbolTable[quardruples[index].opnd1.val].use; //符号表中变量x的use和live赋值给四元式qi的x变量
            quardruples[index].opnd1.live = symbolTable[quardruples[index].opnd1.val].live;
            symbolTable[quardruples[index].opnd1.val].live = true;  //符号表中变量x的use和live分别置为qi和Y
            symbolTable[quardruples[index].opnd1.val].use = index;
        }
        if(quardruples[index].opnd2.val[0]=='T'){
            quardruples[index].opnd2.use = symbolTable[quardruples[index].opnd2.val].use; //符号表中变量y的use和live赋值给四元式qi的y变量
            quardruples[index].opnd2.live = symbolTable[quardruples[index].opnd2.val].live;
            symbolTable[quardruples[index].opnd2.val].live = true;  //符号表中变量y的use和live分别置为qi和Y
            symbolTable[quardruples[index].opnd2.val].use = index;
        }
    }
    return res;//返回当前块出口处的活跃变量集合
}

string getAddress(string var) //获取变量的地址
{
    if(var[0]=='['){
        return var;
    }
    if(symbolTable.find(var)!=symbolTable.end()&&symbolTable[var].offset!=-1){
        return "[ebp-"+to_string(symbolTable[var].offset)+"]"; //如果变量在内存中，返回变量的地址
    }
    if(var.back()=='i'){      //如果变量是整型临时变量
        offset+=4;         //偏移量加4
        symbolTable[var].offset=offset; //更新变量的偏移量
    }
    else if(var.back()=='d'){//如果变量是double型临时变量
        offset+=8;          //偏移量加8
        symbolTable[var].offset=offset; 
    }
    return "[ebp-"+to_string(symbolTable[var].offset)+"]";//返回变量的地址
}

string findR(vector<string> &allR,int index) //用于Ri = （Rj∈RA）argmax（a∈Rval（Rj））min a.use；
{
    // string res;
    // int maxUse=INT32_MIN; //maxUse=-∞
    // for(auto &Ri:allR){
    //     int minUse=INT32_MAX; //minUse=+∞
    //     for(auto &a:Rval[Ri]){ //for each a∈Rval（Ri）
    //         if(historyInfo.find(a)!=historyInfo.end()&&historyInfo[a].use<minUse){ //if a.use<minUse
    //             minUse=historyInfo[a].use; //minUse=a.use
    //         }
    //         if(minUse==-1){
    //             minUse=INT32_MAX; //minUse=+∞
    //         }
    //     }
    //     if(minUse==INT32_MAX){ //如果minUse=+∞
    //         res=Ri; //res=Ri
    //         break;
    //     }
    //     if(minUse>maxUse){ //if minUse>maxUse
    //         maxUse=minUse; //maxUse=minUse
    //         res=Ri; //res=Ri
    //     }
    // }
    // return res;
    string res;
    int maxUse=INT32_MIN; //maxUse=-∞
    int blockIndex=findBlockIndex(index); //blockIndex=FindBlockIndex（q）；
    for(auto &Ri:allR){ //for each Ri∈RA
        bool hasFound=false; // Ri中的变量是否在后续的四元式中使用
        for(int i=index+1;i<=basicBlocks[blockIndex].back();i++){ //遍历四元式
            // if(Rval[Ri].find(quardruples[i].left.val)!=Rval[Ri].end()){ //如果Ri中的变量在四元式qi的左值中
            //     hasFound=true; //hasFound=true
            //     if(i>maxUse){ //更新最大值
            //         maxUse=i;
            //         res=Ri;
            //     }
            //     break;
            // }
            if(Rval[Ri].find(quardruples[i].opnd1.val)!=Rval[Ri].end()){ //同理
                hasFound=true;
                if(i>maxUse){
                    maxUse=i;
                    res=Ri;
                }
                break;
            }
            else if(Rval[Ri].find(quardruples[i].opnd2.val)!=Rval[Ri].end()){
                hasFound=true;
                if(i>maxUse){
                    maxUse=i;
                    res=Ri;
                }
                break;
            }
        }
        if(!hasFound){ //如果Ri中的变量在后续的四元式中没有使用
            res=Ri; //返回Ri
            break;
        }
    }
    return res;
}

string getReg(int index) //局部寄存器分配的伪代码
{
    Quardruple q=quardruples[index]; //四元式q
    string z=q.left.val;             //左值z
    string x=q.opnd1.val;            //左操作数x
    string y=q.opnd2.val;            //右操作数y
    if(!isUNum(x)&&x!="-"){       //如果x是变量
        for(auto &Ri:Aval[x].reg){ //if 存在Ri∈Aval（x）
            if(Rval[Ri]==set<string>{x}&&(x==z||!q.opnd1.live)){ // 并且Rval（Ri）={x}并且（x=z或者x.live=N）
                return Ri;
            }
        }
    }
    for(auto &Ri:regs){ //if 存在Ri
        if(Rval[Ri].empty()){ //使得Rval（Ri）为空
            return Ri; //return Ri；
        }
    }
    vector<string> RA;
    for(auto &Ri:regs){ //RA={Ri|Ri∈R
        if(!Rval[Ri].empty()){ //并且Rval（Ri）中包含主存单元}；
            RA.push_back(Ri);
        }
    }
    if(RA.empty()){ //if RA为空then RA=R；
        RA=regs;
    }
    string Ri;
    bool hasFound=true;
    for(auto &Rj:RA){ //if 存在Rj∈RA
        hasFound=true;
        for(auto &a:Rval[Rj]){ //并且任意a∈Rval（Rj）都有a∈Aval（a）
            if(Aval[a].mem.find(a)==Aval[a].mem.end()){
                hasFound=false;
                break;
            }
        }
        if(hasFound){ //Ri=Rj
            Ri=Rj;
            break;
        }
    }
    if(!hasFound){
        //abort();
        Ri=findR(RA,index); //Ri = （Rj∈RA）argmax（a∈Rval（Rj））min a.use；
    }
    for(auto &a:Rval[Ri]){ //foreach a∈Rval（Ri）do
        // if(!historyInfo[a].live){
        //     continue;
        // }
        if(Aval[a].mem.find(a)==Aval[a].mem.end()&&a!=z){ //if a不属于Aval（a）并且a≠z
            cout<<"mov "<<getAddress(a)<<", "<<Ri<<endl; //生成代码：mov a，Ri；
        }
        if(a==x||(a==y&&Rval[Ri].find(x)!=Rval[Ri].end())){ //if a = x或者（a=y并且x∈Rval（Ri））
            Aval[a].mem={a}; //Aval（a）={a，Ri}；
            Aval[a].reg={Ri};
        }
        else{ //else Aval（a）= {a}；
            Aval[a].mem={a};
            Aval[a].reg={};
        }
    }
    Rval[Ri].clear();// Rval（Ri）= Rval（Ri）- {a}；
    return Ri;
}

void releaseReg(string var,set<string> &liveOut){//释放寄存器
    if(liveOut.find(var)==liveOut.end()){ //if var 不属于 liveOut（Bi）
        for(auto &reg:Aval[var].reg){ //并且存在reg∈Aval（var）
            Rval[reg].erase(var); //Rval（reg）-={var}；
        }
        Aval[var].reg.clear(); //Aval（var）-={reg}；
    }
}


void genForOnlyX(int index,int blockIndex){
    Quardruple q=quardruples[index];
    auto x=q.opnd1.val;
    auto z=q.left.val;
    auto R=getReg(index);
    string x1;
    if(isUNum(x)){
        x1=x; //x是数字，x1=x；
        cout<<"mov "<<R<<", "<<x1<<endl;
    }
    else{ //x是变量
        if(Rval[R].find(x)==Rval[R].end()){ //if x∉Rval（R）
            if(!Aval[x].reg.empty()){  //if Aval（x）≠∅
                x1=*Aval[x].reg.begin(); //x1=Aval（x）中的寄存器；
            }
            else{
                x1=getAddress(x); //x1=地址（x）；
            }
            cout<<"mov "<<R<<", "<<x1<<endl; //生成代码：mov R，x1；
        }
        if(q.op.val!="="){
            cout<<opMap[q.op.val]<<" "<<R<<endl; //生成代码：θ R；
        }
        if(!isUNum(x)){
            releaseReg(x,liveOut[blockIndex]); //releaseReg（x，Bi）；
        }
    }
    Rval[R].insert(z); //Rval（R）+={z}；
    historyInfo[z]=q.left; //historyInfo（R）=q.left；
    Aval[z].reg.insert(R); //Aval（z）+={R}；
    Aval[z].mem.clear(); //Aval（z）-={z}；
}

void genForTheta(int index,int blockIndex){ //if qij形如（θ，x，y，z）then
    Quardruple q=quardruples[index];
    auto x=q.opnd1.val;
    auto y=q.opnd2.val;
    auto z=q.left.val;
    auto Rz=getReg(index); //Rz=getReg（qij）；
    string x1;
    if(x!="-"&&!isUNum(x)){ //x' = 存在Rx∈Aval（x）？Rx：x；
        if(!Aval[x].reg.empty()){
            x1=*Aval[x].reg.begin();
        }
        else{
            x1=getAddress(x);
        }
    }
    else{
        x1=x;
    }
    string y1;
    if(y!="-"&&!isUNum(y)){ //y' = 存在Ry∈Aval（y）？Ry：y；
        if(!Aval[y].reg.empty()){
            y1=*Aval[y].reg.begin();
        }
        else{
            y1=getAddress(y);
        }
    }
    else{
        y1=y;
    }
    if(x1==Rz){ //if x' = Rz then
        cout<<opMap[q.op.val]<<" "<<Rz<<", "<<y1<<endl; //生成代码：θ Rz，y'
        if(opMap[q.op.val]=="cmp"){ //if θ = cmp then
            cout<<cmpMap[q.op.val]<<" "<<Rz<<endl; //生成代码：cmp Rz
        }
        Aval[x].reg.erase(Rz); //Aval（x）-={Rz}；
    }
    else{
        cout<<"mov "<<Rz<<", "<<x1<<endl; //生成代码：mov Rz，x'；和θ Rz，y'；
        cout<<opMap[q.op.val]<<" "<<Rz<<", "<<y1<<endl;
        if(opMap[q.op.val]=="cmp"){
            cout<<cmpMap[q.op.val]<<" "<<Rz<<endl;
        }
        // if(!isUNum(x)){
        //     Aval[x].reg.erase(Rz);
        // }
    }
    if(y1==Rz){ //if y' = Rz 
        if(!isUNum(y)){
            Aval[y].reg.erase(Rz); //then Aval（y）-={Rz}；
        }
    }
    Rval[Rz]={z}; //Rval（Rz） = {z}，Aval（z）={Rz}；
    historyInfo[z]=q.left;
    Aval[z].reg={Rz};
    Aval[z].mem.clear();
    if(!isUNum(x)){
        releaseReg(x,liveOut[blockIndex]);//releaseReg（x，Bi）
    }
    if(!isUNum(y)){
        releaseReg(y,liveOut[blockIndex]);//releaseReg（y，Bi）；
    }
}

void genForRorW(Quardruple &q,int blockIndex){ //为R或W生成代码
    // if(Aval[q.left.val].mem.empty()){
    //     cout<<"mov "<<getAddress(q.left.val)<<", "<<getReg(blockIndex)<<endl;
    //     Aval[q.left.val].mem={q.left.val};
    // }
    if(q.op.val=="W"){
        cout<<"jmp ?write";
    }
    else{
        cout<<"jmp ?read";
    }
    cout<<"("<<getAddress(q.left.val)<<")"<<endl;
    if(!isUNum(q.left.val)){
        releaseReg(q.left.val,liveOut[blockIndex]);
    }
}

void genForEnd(Quardruple &q){ //为End生成代码
    cout<<"halt"<<endl;
    //exit(0);
}

void clearSymbolTable(){ //清空符号表，这是后来加的
    offset=InitOffset;
    for(auto &a:symbolTable){
        if(a.second.isTemp){
            a.second.use=-1;
            a.second.live=false;
            a.second.offset=-1;
        }
    }
}

void genCode()
{
    for(int blockIndex=0;blockIndex<basicBlocks.size();blockIndex++){
        auto &block=basicBlocks[blockIndex];
        if(labelFlag[basicBlocks[blockIndex].front()]==1){
            cout<<"?"+to_string(block.front())+":"<<endl;
        }
        for(auto j:block){//为每个四元式生成代码
            
            if(isTheta(quardruples[j])){
                genForTheta(j,blockIndex);
            }
            else if(isOnlyX(quardruples[j])){
                genForOnlyX(j,blockIndex);
            }
            else if(isROrW(quardruples[j])){
                genForRorW(quardruples[j],blockIndex);
            }
        #ifdef DEBUG
            for(auto reg:Rval){
                cout<<";;;"<<j<<"-"<<reg.first<<":";
                for(auto a:reg.second){
                    cout<<a<<" ";
                }
                cout<<endl;
            }
        #endif
        }
        for(auto a:liveOut[blockIndex]){ //foreach a∈liveOut（Bi）
            if(Aval[a].mem.find(a)==Aval[a].mem.end()){ //并且a不属于Aval（a）do
                if(!Aval[a].reg.empty()){
                    for(auto reg:Aval[a].reg){ //Ra=Aval（a）中的寄存器；
                        if(historyInfo[a].live){
                            cout<<"mov "<<getAddress(a)<<", "<<reg<<endl; //生成代码：mov a，Ra；
                        }
                    }
                }
            }
        }
        auto qini=quardruples[block.back()]; //qini
        if(isJ(qini)){ //if qini形如（j，-，-，q）then
            cout<<"jmp ?"<<qini.left.val<<endl; //生成代码：jmp ?q；
            //genLable(stoi(qini.left.val),labelFlag); //genLabel（q，labelFlag）；
        }
        else if(isJTheta(qini)){ //else if qini形如（jθ，x，y，q）then
            auto x=qini.opnd1.val;
            auto y=qini.opnd2.val;
            auto q=qini.left.val;
            string x1; //x' = 存在Rx∈Aval（x）？Rx：x；
            if(!Aval[x].reg.empty()){
                x1=*Aval[x].reg.begin();
            }
            else{
                x1=x;
            }
            string y1; //y' = 存在Ry∈Aval（y）？Ry：y；
            if(!Aval[y].reg.empty()){
                y1=*Aval[y].reg.begin();
            }
            else{
                y1=getAddress(y);
            }
            if(x1==x){ //if x' = x then
                x1=getReg(block.back()); //x' = getReg（qini）；
                cout<<"mov "<<x1<<", "<<getAddress(x)<<endl; //生成代码：mov x'，x；
            }
            cout<<"cmp "<<x1<<", "<<y1<<endl; //生成代码：cmp x'，y'；
            cout<<jThetaMap[qini.op.val]<<" ?"<<q<<endl; //生成代码：jθ ?q；
            //genLable(stoi(q),labelFlag); //genLabel（q，labelFlag）；
        }
        else if(isJnz(qini)){ //else if qini形如（jnz，x，-，q）then
            auto x=qini.opnd1.val;
            auto q=qini.left.val;
            string x1; //x' = 存在Rx∈Aval（x）？Rx：x；
            if(!Aval[x].reg.empty()){
                x1=*Aval[x].reg.begin();
            }
            else{
                x1=x;
            }
            if(x1==x){
                x1=getReg(block.back());
                cout<<"mov "<<x1<<", "<<getAddress(x)<<endl;
            }
            cout<<"cmp "<<x1<<", 0"<<endl; //生成代码：cmp x'，0；
            cout<<"jne"<<" ?"<<q<<endl; //生成代码：jne ?q；
            //genLable(stoi(q),labelFlag); //genLabel（q，labelFlag）；
        }
        else if(isEnd(qini)){//else if qini形如（End，-，-，-）then
            genForEnd(qini); //生成代码：halt
        }
        Rval.clear(); //所有寄存器描述符置空
        Aval.clear(); //所有变量的地址描述符置空
        //clearSymbolTable();
    }
}

void run(){
    labelFlag.resize(quardruples.size(),0);
    getBasicBlock();
    liveOut.resize(basicBlocks.size());
    for(int i=0;i<basicBlocks.size();i++){
        liveOut[i]=getUseInfo(basicBlocks[i]);
    }
    genCode();
}

int main()
{
    //freopen("input.txt", "r", stdin);
    input();
    run();
    //cout<<"halt"<<endl;
    return 0;
}
