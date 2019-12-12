//
//  fds.cpp
//  574
//  Force Directed Scheduler
//
//  Created by Kristopher Rockowitz on 11/30/19.
//  Copyright © 2019 r0. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <stack>
#include <limits>
#include <cmath>
#include <cstdlib>


//#include <stdio.h>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <vector>
//#include <string>
//#include <algorithm>
//#include <iomanip>
//#include <stack>
//#include <limits>
//#include <cmath>
using namespace std;
// inputs, outputs, registers, wires are of this type
class Variable {
    public:
    string name; // a, b, c, etc.
    string type; // s for signed, u for unsigned
    int bits; // bits 1, 2, 8, 16, 32, 64
    // constructor
    Variable(){
        this->name = "";
        this->type = "";
        this->bits = 0;
    }
    Variable(string name, string type, int bits){
        this->name = name;
        this->type = type;
        this->bits = bits;
    }
};

// class that holds processed operations
// TODO: modify to incorporate 'if' statements
class Operation {
    public:
    Variable result; // the leftmost variable aka result of the op in most cases
    string equals; // the equals operator
    Variable var1; // the first variable after the equals sign
    Variable var2; // the second variable after the equals sign
    Variable var3; // the third variable after the equals sign
    string op1; // the first operator
    string op2; // the second operator
    string name; // name of the operation
    string type; // type (s for signed, u for unsigned)
    int bits; // bits 1, 2, 8, 16, 32, 64
    
    Operation(){
        this->result = Variable();
        this->equals = "";
        this->var1 = Variable();
        this->var2 = Variable();
        this->var3 = Variable();
        this->op1 = "";
        this->op2 = "";
        this->name = "";
        this->type = "";
        this->bits = 0;
    }
};
class Node{
public:
    Operation operation; // add/sub, mult, logic, div/mod
    vector<string> inputs; // max 2 inputs, pretty sure it has to be this way
    vector<Node*> parent;
    vector<Node*> children;
    string output; // max 1 output
    int asap; // this is the first element of timeframe for FDS
    int alap; // this is the second element of timeframe for FDS
    int mobility; // alap - asap. also fyi, width = mobility + 1
    bool asapScheduled;
    bool alapScheduled;
    bool scheduled; // default false, helps to shrink each iteration of FDS
    int selfForce; // SUM(DG(i) * deltaX(i)) where DG(i) is time probability and deltaX(i) is
    int delay;
    int num;
    int fds;
    Node(){
        this->children = {};
        this->inputs = {};
        this->parent = {};
        this->operation = Operation();
        this->output = "";
        this->alap = 0;
        this->asap = 0;
        this->mobility = 0;
        this->scheduled = false;
        this->asapScheduled = false;
        this->alapScheduled = false;
        this->selfForce = 0;
        this->delay = 0;
        this->num = -1;
        this->fds = -1;
    }
};

class Graph{ // will contain the vector of all the edges, where the edges have the children/parent information
    public:
    vector<Node> vertices; //a list of all nodes in this graph
    Node noop;
    Node sink; //I'm thinking these might be helpful
    Graph(){
    }
    void printNodes(){
        vector<Node>::iterator z;
        for(z = this->vertices.begin(); z < this->vertices.end(); z++){
            cout << "Node: " << z->num << " Op: " << z->operation.name << " ";
        }
        cout << endl;
    }
        //multiplies have 2 cycle delay, divide and modulo and 3 cycle delay, all others 1 cycle
        void alap(int latency, bool* validCircuit) {
            for(int i = latency; i >= 1; i--){
                for(int j = this->vertices.size() - 1; j >= 0; j--) {//iterate backwards between nodes
                    if(!this->vertices.at(j).alapScheduled) {
                        string newCurrentOp = this->vertices.at(j).operation.name;
                        int delay = 0;
                        if(this->vertices.at(j).children.size() == 0) {//schedule, no children, bottom node
                            this->vertices.at(j).alapScheduled = true;
                            this->vertices.at(j).alap = (i - this->vertices.at(j).delay) + 1;
                        }
                        else {//has children, all of them must be scheduled
                            bool allChildrenScheduled = true;
                            int earliestSchedule = latency + 1;
                            for(int k = 0; k < this->vertices.at(j).children.size(); k++) {
                                if(!this->vertices.at(j).children.at(k)->alapScheduled) {
                                    allChildrenScheduled = false;
                                    break;
                                }
                                else {//if the child is scheduled, then we must record the earliest one of the children are scheduled
                                      //this is because when scheduling the current node, there must be enough time for its operation to be completed
                                    if(this->vertices.at(j).children.at(k)->alap < earliestSchedule) {
                                        earliestSchedule = this->vertices.at(j).children.at(k)->alap;
                                    }
                                }
                            }
                            if(allChildrenScheduled) {//now, we check the current operation to see if there was enough time between it
                                                      //and its earliest child
                                if(i < earliestSchedule) {
                                    this->vertices.at(j).alap = (i - this->vertices.at(j).delay) + 1;
                                    this->vertices.at(j).alapScheduled = true;
                                }
                            }
                        }
                    }
                }
            }
            bool allScheduled = true;
            for(int i = 0; i < this->vertices.size(); i++) {
                if(!this->vertices.at(i).alapScheduled) {
                    allScheduled = false;
                    break;
                }
            }
            if(!allScheduled) {
                //quit program
                *validCircuit = false;
            }
        }
        void asap(int latency, bool* validCircuit) {
            for(int i = 1; i <= latency; i++) {
                for(int j = 0; j < this->vertices.size(); j++) {
                    if(!this->vertices.at(j).asapScheduled) {
                        if(this->vertices.at(j).parent.size() == 0) { //schedule, no parents, top node
                            this->vertices.at(j).asapScheduled = true;
                            this->vertices.at(j).asap = i;
                        }
                        else { //has parents, check if all of them are scheduled
                            bool allParentScheduled = true;
                            int latestSchedule = 0;
                            for(int k = 0; k < this->vertices.at(j).parent.size(); k++) {
                                if(!this->vertices.at(j).parent.at(k)->asapScheduled) {
                                    allParentScheduled = false;
                                    break;
                                }
                                else { //if the parent is scheduled, this records the latest a parent is scheduled
                                    //because the latest one is what determines when the child should be scheduled
                                    //use parent op name to find when the latest cycle is available
                                    // string parentOp = this->vertices.at(j).parent.at(k)->operation.name;
                                    int parentDelay = this->vertices.at(j).parent.at(k)->delay;
                                    int tempLatest = 0;
                                    tempLatest = this->vertices.at(j).parent.at(k)->asap + parentDelay;
                                    if(tempLatest > latestSchedule) {
                                        latestSchedule = tempLatest;
                                    }
                                }
                            }
                            //now that we know all parents are scheduled, we can start checking if these nodes have
                            //the resources to be scheduled
                            if(allParentScheduled){
                                if(i>=latestSchedule) {
                                    this->vertices.at(j).asapScheduled = true;
                                    this->vertices.at(j).asap = i;
                                }
                            }
                        }
                    }
                }
            }
            bool allScheduled = true;
            for(int i = 0; i < this->vertices.size(); i++) {
                if(!this->vertices.at(i).asapScheduled) {
                    allScheduled = false;
                    break;
                }
            }
            if(!allScheduled) {
                //quit program
                *validCircuit = false;
            }
        }
        void populateNodes(vector<Operation> ops) {
            for(int i = 0; i < ops.size(); i++) {
                Node curr;
                Operation currentOp = ops.at(i);
                curr.operation = currentOp;
                if(currentOp.name.compare("MULT") == 0) {//2 cycle delay
                    curr.delay = 2;
                }
                else if(currentOp.name.compare("DIV") == 0) {//3 cycle delay
                    curr.delay = 3;
                }
                else if(currentOp.name.compare("MOD") == 0) {//3 cycle delay
                    curr.delay = 3;
                }
                else {
                    curr.delay = 1;
                }
                //TODO: add in the ternary operator check here, that's when the third variable will be used
                curr.inputs.push_back(currentOp.var1.name);
                //TODO: add the ! thing check
                if (currentOp.op1.compare("!") == 0) {
                    //don't need var1 or var 2
                }
                else {
                    curr.inputs.push_back(currentOp.var2.name);
                }
                if (currentOp.op1.compare("?") == 0) {
                    //need var 3
                    curr.inputs.push_back(currentOp.var3.name);
                }
                curr.output = currentOp.result.name;
                this->vertices.push_back(curr);
            }
        }
        void cdfg() {
            vector<int> visitedNodes = {}; //keep track of what nodes have been seen, recording their indexes
            vector<string> visistedOutputs = {};
            for(int i = 0; i < this->vertices.size(); i++) {
                Node *current = &this->vertices.at(i);
                for(int j = 0; j < current->inputs.size(); j++) { //this is usually 2 iterations
                    string inp = current->inputs.at(j);
                    for(int foundIndex = 0; foundIndex < visistedOutputs.size(); foundIndex++) { //now, go through the visited outputs to see if the input matches anything
                        if(inp == visistedOutputs.at(foundIndex)) {
                            current->parent.push_back(&this->vertices.at(visitedNodes.at(foundIndex)));
                            this->vertices.at(visitedNodes.at(foundIndex)).children.push_back(current);
                            break;
                        }
                    }
                }
                visitedNodes.push_back(i); //add index of where the node visited was in the for loop
                visistedOutputs.push_back(current->output);
            }
        }
        void schedule(int latency, bool* validCircuit) {
            this->alap(latency, validCircuit);
            this->asap(latency, validCircuit);
        }
    };

vector<string> ReadFile(string inputFile, bool *validCircuit){
//    cout << "Reading File...\n";
    vector<string> parsedFile;
    ifstream netlistFile;
    netlistFile.open(inputFile);

    if(netlistFile){
        string str;
        while(getline(netlistFile, str)){
            parsedFile.push_back(str);
        }
    }
    if(netlistFile.peek() == std::ifstream::traits_type::eof()){
        *validCircuit = false;
        return parsedFile;
    }
    else{
//        cout << "Unable to read file" << endl;
        *validCircuit = false;
        return parsedFile;
    }
//    cout << "Done reading file." << endl;
    return parsedFile;
}

void PrintOperands(vector<Variable> &inputs, vector<Variable> &outputs, vector<Variable> &vars, vector<string>& operations){
    cout << "Inputs:\n";
    for(vector<Variable>::iterator i = inputs.begin(); i != inputs.end(); i++){
        cout << distance(inputs.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nOutputs:\n";
    for(vector<Variable>::iterator i = outputs.begin(); i != outputs.end(); i++){
        cout << distance(outputs.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nVariables:\n";
    for(vector<Variable>::iterator i = vars.begin(); i != vars.end(); i++){
        cout << distance(vars.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nOperations:\n";
    for(vector<string>::iterator i = operations.begin(); i != operations.end(); i++){
            cout << (*i) << "\n";
    }
}

void ParseLine(vector<Variable>& inputs, vector<Variable>& outputs, vector<Variable>& vars, vector<string>& operations, string line){
    //parse into inputs, outputs, wires and operations
    
    if(line.find("input")==0){
        //input Int8 a, b, c
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        char U = line.at(line.find("Int")-1);

        
        for(int i = line.find(' ',line.find("Int")); i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] == '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                Variable input;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)
                        i = j;
                    input.name += line[j];
                    j++;
                }
                if(U == 'U'|| BW == 1){
                    input.type = "u";
                }
                else{
                    input.type = "s";
                }
                input.bits = BW;

                inputs.push_back(input);
            }
        }
    }
    else if(line.find("output")==0){
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        char U = line.at(line.find("Int")-1);
        
        for(int i = line.find(' ',line.find("Int"));  i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] == '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                Variable output;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)
                        i = j;
                    output.name += line[j];
                    j++;
                }
                if(U == 'U'|| BW == 1){
                    output.type = "u";
                }
                else{
                    output.type = "s";
                }
                output.bits = BW;
                outputs.push_back(output);
            }
        }
    }
    else if(line.find("variable")==0){
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        char U = line.at(line.find("Int")-1);
        
        for(int i = line.find(' ',line.find("Int"));  i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] == '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                Variable var;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)// check to skip rest of whole words
                        i = j;
                    var.name += line[j];
                    j++;
                }
                if(U == 'U' || BW == 1){
                    var.type = "u";
                }
                else{
                    var.type = "s";
                }
                var.bits = BW;
                vars.push_back(var);
            }
        }
    }
    else if(line.find("=") != std::string::npos || line.find("{") != std::string::npos || line.find("}") != std::string::npos){
        operations.push_back(line);
    }
}

// search all the vectors and return the Variable, else empty Variable
Variable searchVariables(vector<Variable> in, vector<Variable> out, vector<Variable> vari, string find){
    std::vector<Variable>::iterator it;
    for(it = in.begin(); it != in.end(); ++it){
        if(it->name == find){
            return *it;
        }
    }
    for(it = out.begin(); it != out.end(); ++it){
        if(it->name == find){
            return *it;
        }
    }
    for(it = vari.begin(); it != vari.end(); ++it){
        if(it->name == find){
            return *it;
        }
    }
    Variable var;
    return var;
}

int maxInputBits(Operation op){
    unsigned int max = 0;
    if(op.var1.bits > max){
        max = op.var1.bits;
    }
    if(op.var2.bits > max){
        max = op.var2.bits;
    }
    if(op.var3.bits > max){
        max = op.var3.bits;
    }
    return max;
}



// process the string operations and return the Operation vector
vector<Operation> processOperation(vector<string> operations, vector<Variable> inputs, vector<Variable> outputs, vector<Variable> vars, bool* validCircuit){
    vector<Operation> temp;
     // TODO: move this shit to a function
       // create a stringstream to parse the elements of the operation line string
       // iterate through the operation vector
       int numIf = 0;
       bool elseCond = false;
       int elseIndex = -1;
       vector<Variable> ifCond;
       vector<bool> elseCondition;
       for(unsigned int i = 0; i < operations.size(); i++){
           stringstream opstream;
           string result, equals, var1, op1, var2, op2, var3;
           // throw the line into the stream
           opstream << operations.at(i);
           // stream into the variables
           opstream >> result >> equals >> var1 >> op1 >> var2 >> op2 >> var3;
           // create new operation with parsed operators
           if(result == "if"){
               numIf++;
                Variable v = searchVariables(inputs, outputs, vars, var1);
                ifCond.push_back(v);
                elseCondition.push_back(false);
           }
           else if(result == "}" && i != operations.size()-1){
               stringstream eop;
               string e;
               eop << operations.at(i+1);
               eop >> e;
               elseCond = e == "else";
               if(!elseCond){
                    numIf--;
                    ifCond.erase(ifCond.end());
                    elseCondition.erase(elseCondition.end());
               }
               else{
                   elseCondition[elseCondition.size()-1] = true;
               }
           }
           else if (result != "else" && result != "}"){
                Operation oCond;
                if(numIf > 0){
                    int s = ifCond.size();
                    Variable first = ifCond[ifCond.size()-1];
                    int idx = ifCond.size()-1;
                    while(idx > 0){
                        Variable second = ifCond[idx-1];
                        if(elseCondition[idx] == false && elseCondition[idx-1 == true]){
                            Variable e = Variable("e_" + to_string(i), "", 32);
                            vars.push_back(e);
                            Operation eo;
                            eo.result = e;
                            eo.equals = "=";
                            eo.op1 = "!";
                            eo.var1 = second;
                            eo.name = "NOT";
                            temp.push_back(eo);
                            second = e;
                        }
                        else if(elseCondition[idx] == true && idx == ifCond.size() -1){
                            Variable e = Variable("e_" + to_string(i), "", 32);
                            vars.push_back(e);
                            Operation eo;
                            eo.result = e;
                            eo.equals = "=";
                            eo.op1 = "!";
                            eo.var1 = first;
                            eo.name = "NOT";
                            temp.push_back(eo);
                            first = e;
                        }
                        Variable c = Variable("cond" + to_string(s) +"_"+to_string(i), "", 32);
                        vars.push_back(c);
                        Operation nestIf;
                        nestIf.result = c;
                        nestIf.equals = "=";
                        nestIf.var1 = first;
                        nestIf.op1 = "&&";
                        nestIf.name = "COMP";
                        nestIf.var2 = second;
                        
                        temp.push_back(nestIf);
                        first = c;
                        idx--;
                        
                    }
                    Variable v = searchVariables(inputs, outputs, vars, result);
                    if(v.bits == 0){
                        *validCircuit = false;
                    }
                    else{
                        oCond.result = v;
                    }
                    oCond.equals = "=";
                    oCond.var1 = first;
                    oCond.op1 = "?";

                    result = result + to_string(i);
                    Variable r = Variable(result, "", 32);
                    vars.push_back(r);

                    oCond.var2 = r;
                    oCond.op2 = ":";
                    oCond.var3 = v;
                }
                Operation o1;
                o1.equals = equals;
                o1.op1 = op1;
                o1.op2 = op2;
                // discover component and assign to name
                if(op1 == "")
                    o1.name = "REG";
                else if(op1 == "+" && var2 == "1")
                    o1.name = "INC";
                else if(op1 == "+")
                    o1.name = "ADD";
                else if(op1 == "-" && var2 == "1")
                        o1.name = "DEC";
                else if(op1 == "-")
                    o1.name = "SUB";
                else if(op1 == "*")
                    o1.name = "MUL";
                else if(op1 == ">" || op1 == "<" || op1 == "==")
                    o1.name = "COMP";
                else if(op1 == "?" && op2 == ":")
                        o1.name = "MUX2x1";
                else if(op1 == ">>")
                    o1.name = "SHR";
                else if(op1 == "<<")
                    o1.name = "SHL";
                else if(op1 == "/")
                    o1.name = "DIV";
                else if(op1 == "%")
                    o1.name = "MOD";
                else if(op1 == "/")
                    o1.name = "DIV";
                else{
                    *validCircuit = false;
                }
                Variable r1 = searchVariables(inputs, outputs, vars, result);
                if(r1.bits == 0){
                    *validCircuit = false;
                }
                else{
                    o1.result = r1;
                }
                if(equals != "="){
                    *validCircuit = false;
                }

                // validate 'var1'
                Variable v1 = searchVariables(inputs, outputs, vars, var1);
                if(v1.bits == 0){
                    *validCircuit = false;
                }
                else{
                    if(o1.name == "MUX2x1"){
                        o1.var3 = v1;
                    }
                    else{
                        o1.var1 = v1;
                    }

                }
                Variable v2 = searchVariables(inputs, outputs, vars, var2);
                if(v2.bits == 0 && o1.name != "INC" && o1.name != "REG" && o1.name != "DEC"){
                    *validCircuit = false;
                }
                else{
                    if(o1.name == "MUX2x1"){
                        o1.var1 = v2;
                    }
                    else{
                        o1.var2 = v2;
                    }
                }
           
                Variable v3 = searchVariables(inputs, outputs, vars, var3);
                if(v3.bits != 0 && o1.name != "MUX2x1"){
                    *validCircuit = false;
                }
                else{
                    if(o1.name == "MUX2x1"){
                        o1.var2 = v3;
                    }
                    else{
                        o1.var3 = v3;
                    }
                }
                if(o1.name == "COMP"){
                    o1.bits = maxInputBits(o1);
                }
                else{
                    o1.bits = o1.result.bits;
                }
                temp.push_back(o1);
                if(numIf > 0)
                        temp.push_back(oCond);
            } 
        }
    return temp;
}
// Self force function takes in a node, a time, and all the DGs. Iterates throught the timeframes and calculates all the negative probability components.
// Then with the time given, calcs the positive probability component and returns the self force.
// This works off the reduced equation: Self Force = DG(time) - Sum[DG(n) * prob(n)] where n is the interval of asap to alap.
float SelfForce(Node node, int time, vector<float> dg_mult, vector<float> dg_addsub, vector<float> dg_divmod, vector<float> dg_logic){
    // loop through all time frames for a given node and get the negative part of self force. this represents the probability for all times in timeframe
    
    float selfForce = 0.0;
    // if the time given is outside of the asap to alap interval, the force is necessarily 0
    if(time < node.asap || time > node.alap){
        selfForce = 0.0;
    }
    else{
        for(unsigned int n = node.asap; n <= node.alap; n++){
            // calc 1/width for ease of use
            float prob = 1/((float)node.alap - (float)node.asap + 1);
            // operation type matters and so filter by operation type
            if(node.operation.name == "MULT"){
                // self force = DG(j) * deltaX(j) for alap - asap number of terms
                selfForce -= dg_mult.at(n-1) * prob;
            }
            else if(node.operation.name == "DIV" || node.operation.name == "MOD"){
                // self force = DG(j) * deltaX(j) for alap - asap number of terms
                selfForce -= dg_divmod.at(n-1) * prob;
            }
            else if(node.operation.name == "ADD" || node.operation.name == "SUB"){
                // self force = DG(j) * deltaX(j) for alap - asap number of terms
                selfForce -= dg_addsub.at(n-1) * prob;
            }
            else if(node.operation.name == "COMP" || node.operation.name == "MUX2x1" || node.operation.name == "SHR" || node.operation.name == "SHL" || node.operation.name == "NOT"){
                // self force = DG(j) * deltaX(j) for alap - asap number of terms
                selfForce -= dg_logic.at(n-1) * prob;
            }
        }// end negative self force part
        
        // add the positive component to self force. this represents the probability is 1 at the time given
        if(node.operation.name == "MULT"){
            selfForce += dg_mult.at(time-1);
        }
        else if(node.operation.name == "DIV" || node.operation.name == "MOD"){
            selfForce += dg_divmod.at(time-1);
        }
        else if(node.operation.name == "ADD" || node.operation.name == "SUB"){
            selfForce += dg_addsub.at(time-1);
        }
        else if(node.operation.name == "COMP" || node.operation.name == "MUX2x1" || node.operation.name == "SHR" || node.operation.name == "SHL" || node.operation.name == "NOT"){
            selfForce += dg_logic.at(time-1);
        }
    }
    return selfForce;
}

// calculates the predecessor forces for the given node
tuple<float, int> TotalForce(Node s_in, int size, vector<float> dg_mult, vector<float> dg_addsub, vector<float> dg_divmod, vector<float> dg_logic) {
  // s is the node, time is the current scheduling time, size is the number of total nodes
    
    // TODO: add functionality so that when pred/succ forces are calculated, they set all the timeframes of connected nodes
    Node s = s_in;
    tuple<float, int> out;
    
    float lowestForce = numeric_limits<float>::max(); // create super high float so any min is lower
    for(unsigned int n = s.asap; n <= s.alap; n++){
        
        // create a changes Vector
        vector<Node> changes;
        int time = n;
        // calc self Force
        float selfForce = SelfForce(s, time, dg_mult, dg_addsub, dg_divmod, dg_logic);
        cout << "Self: " << selfForce;

        // Initially mark all verices as not visited
        vector<bool> visited(size, false);
      
        // Create a stack for DFS
        stack<Node> stack;
      
        // Push the current source node.
        stack.push(s);
        // level used for time calculation
        int level = 0;

        //intialize predecessor force
        float predecessorForce = 0.0;
        int x = 0;
        while (!stack.empty())
        {
            // Pop a vertex from stack and print it
            s = stack.top();
            stack.pop();

            if (!visited[s.num]){
//                cout << s.num << " ";
                visited[s.num] = true;
                // if it's not the first node, evaluate whether pred force exists
                if(x > 0){
                    float sf = 0.0;
                    // if the pred node has an alap that is greater than adj_time, calc self force at adj_time
                    int adj_time = time - level + 1;
                    if(s.alap >= adj_time){
                        sf = SelfForce(s, adj_time - 1, dg_mult, dg_addsub, dg_divmod, dg_logic);
                    }
                    else{
                        sf = 0.0;
                    }
                    predecessorForce += sf;
                }
            }
            x++;

            // increment the level every time the number of parents > 0
            if(s.parent.size() > 0){
                level++;
            }
      
            for (unsigned int i = 0; i < s.parent.size(); i++){
                // check if the parent num is in the visisted array
                if (!visited[s.parent.at(i)->num]){
                    stack.push(*s.parent.at(i));
                }
            }
        }
        cout << " Pred: " << predecessorForce;
        
        // Initially mark all verices as not visited
          vector<bool> visited2(size, false);

          // Push the current source node.
          stack.push(s);
          // level used for time calculation
          level = 0;

          //intialize successor force
          float successorForce = 0.0;
          x = 0;
          while (!stack.empty())
          {
              // Pop a vertex from stack and print it
              s = stack.top();
              stack.pop();
        
              if (!visited[s.num]){
//                  cout << s.num << " ";
                  visited[s.num] = true;
                  // if it's not the first node, evaluate whether pred force exists
                  if(x > 0){
                      float sf = 0.0;
                      // if the pred node has an alap that is greater than adj_time, calc self force at adj_time
                      int adj_time = time + level - 1;
                      if(s.asap <= adj_time){
                          sf = SelfForce(s, adj_time + 1, dg_mult, dg_addsub, dg_divmod, dg_logic);
                      }
                      else{
                          sf = 0.0;
                      }
                      successorForce += sf;
                  }
              }
              x++;

              // increment the level every time the number of parents > 0
              if(s.children.size() > 0){
                  level++;
              }
        
              for (unsigned int i = 0; i < s.children.size(); i++){
                  // check if the parent num is in the visisted array
                  if (!visited[s.children.at(i)->num]){
                      stack.push(*s.children.at(i));
                  }
              }
          }
          cout << " Succ: " << successorForce;
        
        float totalForce = selfForce + predecessorForce + successorForce;
        cout << " Total: " << totalForce << endl;
        if (totalForce * 1000 < lowestForce * 1000){
            
            lowestForce = totalForce;
            out = make_tuple(totalForce, n);
        }
        s = s_in;
    }// end for
     return out;
}
void DG(Graph g, vector<float>&dg_mult, vector<float>&dg_addsub, vector<float>&dg_divmod, vector<float>&dg_logic){
    // populate the DG for each component
    vector<Node>::iterator i;
    
    // loop over all vertices / nodes, same thing
    for(i = g.vertices.begin(); i != g.vertices.end(); i++){
        float mobility = i->alap - i->asap;
        // examine timeframe [asap, alap] and calc probability
        // find the operation type so you know which vector to add to
        if(i->operation.name == "MULT"){
            // loop over the time intervals this operation covers
            // start at asap, end at alap
            for(unsigned int j = i->asap; j <= i->alap; j++){
                // accumulate the probability for this component on the total
                // type probability for this specific time
                dg_mult[j-1] += 1 / (mobility + 1);
            }
        }
        if(i->operation.name == "DIV" || i->operation.name == "MOD"){
            // loop over the time intervals this operation covers
            // start at asap, end at alap
            for(unsigned int j = i->asap; j <= i->alap; j++){
                // accumulate the probability for this component on the total
                // type probability for this specific time
                dg_divmod[j-1] += 1 / (mobility + 1);
            }
        }
        if(i->operation.name == "ADD" || i->operation.name == "SUB"){
            // loop over the time intervals this operation covers
            // start at asap, end at alap
            for(unsigned int j = i->asap; j <= i->alap; j++){
                // accumulate the probability for this component on the total
                // type probability for this specific time
                dg_addsub[j-1] += 1 / (mobility + 1);
            }
        }
        if(i->operation.name == "COMP" || i->operation.name == "MUX2x1" || i->operation.name == "SHR" || i->operation.name == "SHL" || i->operation.name == "NOT"){
            // loop over the time intervals this operation covers
            // start at asap, end at alap
            for(unsigned int j = i->asap; j <= i->alap; j++){
                // accumulate the probability for this component on the total
                // type probability for this specific time
                dg_logic[j-1] += 1 / (mobility + 1);
            }
        }
    } // end DG for
}

    
void printVector(vector<float> vec){
    for(int x = 0; x < vec.size(); x++){
        cout << "[" << vec.at(x) << "] ";
    }
    cout << endl;
}

// assume inputs are a graph G(V,E) and latency.
// assume each vertex has a field that gives it's ASAP, ALAP time.
void FDS(Graph &g, int latency){
    
    // DG(i) variables (probabilities per type at a given time)
    // intialized to 0 and size latency constraint lambda (time)
    vector<Node>::iterator k;
    int num_nodes = g.vertices.size();
    cout << "FDS -> NODE COUNT: " << num_nodes << endl << endl;
    // loop through all the nodes to get the foce for each.
    for(k = g.vertices.begin(); k != g.vertices.end(); k++){
        cout << "beginning node: " << k->num << " time frame: [" << k->asap << ", " << k->alap << "]"<< endl;

    
        // 1) cacl timeframes
        // 2) calc the operation and type probabilites
        // 3) calc self forces, predecessor/successor forces and total forces
        // 4) schedule the operation with the least force and update its timeframe
        
        // create DGs
        vector<float> dg_mult(latency, 0);
        vector<float> dg_addsub(latency, 0);
        vector<float> dg_divmod(latency, 0);
        vector<float> dg_logic(latency, 0);
        DG(g, dg_mult, dg_addsub, dg_divmod, dg_logic);
        cout << "DG_MULT: ";
        printVector(dg_mult);
        cout << "DG_ADD/SUB: ";
        printVector(dg_addsub);
        cout << "DG_DIV/MOD: ";
        printVector(dg_divmod);
        cout << "DG_LOGICAL: ";
        printVector(dg_logic);
    
        // calc the forces

        tuple<float, int> ft = TotalForce(*k, num_nodes, dg_mult, dg_addsub, dg_divmod, dg_logic);
        cout << "lowest total force: " << get<0>(ft) << " at time: " << get<1>(ft) << endl;
        
        // schedule the node
        k->fds = k->asap = k->alap = get<1>(ft);
        k->scheduled = true;
        cout << endl <<"SCHEDULED -> NODE: " << k->num << " FDS: " << k->fds << " ASAP: " << k->asap << " ALAP: " << k->alap << endl << endl;
        
        // change all times for nodes connected to scheduled node
        
        vector<Node>::iterator k2;
        // loop through all nodes
        for(k2 = g.vertices.begin(); k2 != g.vertices.end(); k2++){
            // Nodes that have this node as a parent
            
            // if they have the scheduled node as a parent, and there is an asap overlap, restrict it to time +1
            for(unsigned int p = 0; p < k2->parent.size(); p++){
                // compare the parent nodes to the scheduled node
                if(k2->parent.at(p)->num == k->num){
                    // update the parent's status to scheduled and time frames updated in this node
                    k2->parent.at(p)->alap = k2->parent.at(p)->asap = k2->parent.at(p)->fds = get<1>(ft);
                    // if the asap and scheduled nodes times overlap
                    if(k2->asap <= get<1>(ft)){
                        // restrict it to time + 1
                        k2->asap = get<1>(ft) + 1;
                    }
                }
            }
            
            // Nodes that have this node as a child
            for(unsigned int p = 0; p < k2->children.size(); p++){
                // compare the children nodes to the scheduled node
                if(k2->children.at(p)->num == k->num){
                    // update the parent's status to scheduled and time frames updated in this node
                    k2->children.at(p)->alap = k2->children.at(p)->asap = k2->children.at(p)->fds = get<1>(ft);
                    // if the alap and scheduled nodes times overlap
                    if(k2->alap >= get<1>(ft)){
                        // restrict it to time + 1
                        k2->alap = get<1>(ft) - 1;
                    }
                }
            }
        }
        

    }// end for loop through all nodes
}
// Connor begin
void writeModule(vector<Variable> inputs, vector<Variable> outputs, ofstream &file){
    cout << "module HLSM (Clk, Rst, Start, Done, ";
    file << "module HLSM (Clk, Rst, Start, Done, ";
    std::vector<Variable>::iterator it;
    for ( it = inputs.begin() ; it != inputs.end(); ++it){
        cout << it->name << ", ";
        file << it->name << ", ";
    }
    for (it = outputs.begin() ; it != outputs.end(); ++it){
        if(it == outputs.end()-1){
            cout << it->name;
            file << it->name;
        }
        else{
            cout << it->name << ", ";
            file << it->name << ", ";
        }
    }
    cout << ");" << endl << endl;
    file << ");" << endl << endl;
}

// write inputs
void writeInputs(vector<Variable> var, ofstream &file){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "\tinput ";
        file << "\tinput ";
        if(it->type == "s"){
            cout << "signed ";
            file << "signed ";
        }
        if(it->bits > 1){
            // cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
            file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        }
        else{
            // cout << it->name << ";" << endl;
            file << it->name << ";" << endl;
        }
    }
}

// write outputs
void writeOutputs(vector<Variable> var, ofstream &file){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "\toutput reg ";
        file << "\toutput reg ";
        if(it->type == "s"){
            cout << "signed ";
            file << "signed ";
        }
        if(it->bits > 1){
            // cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
            file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        }
        else{
            // cout << it->name << ";" << endl;
            file << it->name << ";" << endl;
        }
    }
}

int writeParameters(Graph g, ofstream &file){
    int numStates = 0;
    for(vector<Node>::iterator it = g.vertices.begin(); it != g.vertices.end(); it++){
        if((*it).fds>numStates){
            numStates = (*it).fds;
        }
    }
    
    for(int i = 1; i< numStates+2;i++){
        if(i == numStates + 1){
            cout << "S" << i << " = " << i <<";"<<endl;
            file << "S" << i << " = " << i <<";"<<endl;
        }
        else{
            cout << "S"<< i << " = " << i << " ,";
            file << "S"<< i << " = " << i << " ,";
        }
    }
    return numStates;
}

// write registers
void writeRegisters(vector<Variable> var, ofstream &file){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "\treg ";
        file << "\treg ";
        if(it->type == "s"){
            cout << "signed ";
            file << "signed ";
        }
        if(it->bits > 1){
            // cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
            file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        }
        else{
            // cout << it->name << ";" << endl;
            file << it->name << ";" << endl;
        }
    }
}

void writeResetReg(vector<Variable> outputs, vector<Variable> variables, ofstream& file){
    cout<<"\t\t\tState <= S0;"<<endl;
    file<<"\t\t\tState <= S0;"<<endl;
    for (std::vector<Variable>::iterator it = outputs.begin() ; it != outputs.end(); ++it){
        cout << "\t\t\t" <<(*it).name << " <= 0;" <<endl;
        file << "\t\t\t" <<(*it).name << " <= 0;" <<endl;
    }
    for (std::vector<Variable>::iterator it = variables.begin() ; it != variables.end(); ++it){
        cout << "\t\t\t" <<(*it).name << " <= 0;" <<endl;
        file << "\t\t\t" <<(*it).name << " <= 0;" <<endl;
    }
}

bool comparator(Node& lhs, Node& rhs){
    return lhs.fds < rhs.fds;
}

void writeStates(Graph g, int numStates, ofstream& file){
    vector<Node> vertices = g.vertices;
    sort(vertices.begin(), vertices.end(), &comparator);
    
    for(int i=1; i< numStates+1;i++){
        cout << "\t\t\t\tS"<<i<<": begin"<<endl;
        file  << "\t\t\t\tS"<<i<<": begin"<<endl;
        
        for(vector<Node>::iterator it = vertices.begin();it!=vertices.end();it++){
            if(i==(*it).fds){
                if((*it).operation.op1 == "?"){
                    cout<<"\t\t\t\t\t" << (*it).operation.result.name<<"= " << (*it).operation.var1.name << (*it).operation.op1 << (*it).operation.var2.name << (*it).operation.op2<<(*it).operation.var3.name << ";"<<endl;
                    file<<"\t\t\t\t\t" << (*it).operation.result.name<<"= " << (*it).operation.var1.name << (*it).operation.op1 << (*it).operation.var2.name << (*it).operation.op2<<(*it).operation.var3.name << ";"<<endl;
                }
                else{
                    cout<<"\t\t\t\t\t"<<(*it).operation.result.name<<"= "<<(*it).operation.var1.name<<(*it).operation.op1<<(*it).operation.var2.name<<";"<<endl;
                    file<<"\t\t\t\t\t"<<(*it).operation.result.name<<"= "<<(*it).operation.var1.name<<(*it).operation.op1<<(*it).operation.var2.name<<";"<<endl;
                }
            }
        }
        cout << "\t\t\t\tend"<<endl;
        file << "\t\t\t\tend"<<endl;
    }
}

void VerilogHLSM(Graph g, vector<Variable> inputs, vector<Variable> outputs, vector<Variable> variables, ofstream& file){
    int numStates = 0;
    //STATIC timescale
    cout << "`timescale 1ns / 1ps \n" << endl;
    file << "`timescale 1ns / 1ps \n" << endl;
    
    writeModule(inputs,outputs,file);
    
    //inputs
    //STATIC inputs
    cout << "\tinput Clk, Rst, Start;\n";
    file << "\tinput Clk, Rst, Start;\n";
    //DYNAMIC inputs (TODO)
    
    writeInputs(inputs, file);
    
    //outputs
    //STATIC outputs
    cout << "\toutput reg Done;\n";
    file << "\toutput reg Done;\n";
    
    //DYNAMIC outputs (TODO)
    
    writeOutputs(outputs, file);
    
    //parameters (states)
    
    cout << "\tparameter S0 = 0, ";
    file << "\tparameter S0 = 0, ";
    
    numStates = writeParameters(g, file);
    
    //registers (interim states and states)
    
    writeRegisters(variables, file);

    int numBits = 1;
    
    while(pow(2,numBits) < numStates + 2){
        numBits++;
    }
    
    cout << "\treg ["<< numBits-1 << ":0] State;\n"; //replace 2 with number of bits needed for states
    file << "\treg ["<< numBits-1 << ":0] State;\n";
    
    //STATIC always block
    cout << "\talways @(posedge Clk) begin\n";
    file << "\talways @(posedge Clk) begin\n";
    
    //STATIC if (Rst) begin
    cout << "\t\tif(Rst) begin\n";
    file << "\t\tif(Rst) begin\n";
    
    writeResetReg(outputs, variables, file);
    
    //STATIC end/else for reset condition and case statement
    cout << "\t\tend\n\t\telse begin\n\t\t\tcase(State)\n";
    file << "\t\tend\n\t\telse begin\n\t\t\tcase(State)\n";
    
    //STATIC wait state
    cout << "\t\t\t\tS0: begin\n\t\t\t\t\tif (Start) begin\n\t\t\t\t\t\tState <= S1;\n\t\t\t\t\tend\n"
    << "\t\t\t\t\telse begin\n\t\t\t\t\t\tState <= S0;\n\t\t\t\t\tend\n\t\t\t\tend\n";
    file << "\t\t\t\tS0: begin\n\t\t\t\t\tif (Start) begin\n\t\t\t\t\t\tState <= S1;\n\t\t\t\t\tend\n"
    << "\t\t\t\t\telse begin\n\t\t\t\t\t\tState <= S0;\n\t\t\t\t\tend\n\t\t\t\tend\n";
    
    writeStates(g, numStates, file);
    
    //STATIC done state
    cout << "\t\t\t\tS"<< numStates + 1 <<": begin\n\t\t\t\t\tState <= S0;\n\t\t\t\t\tDone <= 1;\n\t\t\t\tend\n";
    file << "\t\t\t\tS"<< numStates + 1 <<": begin\n\t\t\t\t\tState <= S0;\n\t\t\t\t\tDone <= 1;\n\t\t\t\tend\n";
    
    //STATIC end for else and endcase
    cout << "\t\t\tendcase\n\t\tend\n";
    file << "\t\t\tendcase\n\t\tend\n";
    
    //STATIC end for always
    cout << "\tend\nendmodule\n";
    file << "\tend\nendmodule\n";
}
// Connor end

int main(int argc, const char * argv[]) {
// validate command line args
    
    // validate command line args
    //    hlsyn cFile latency verilogFile
    if (argc != 4) {
        // show usage
        cerr << "Usage: arg[0] should be hlsyn, arg[1] should be a cFile, arg[2] should be a latency, and arg[3] should be a verilog .v file.\nExample: ./src/hlsyn cFile.c latency verilogFile.v\n";

        return 1;
    }

    
    
    
//    vector<Operation> listOfOps; //create a list of test operations
//
//    Variable w("w", "s", 2);
//    Variable x("x", "s", 2);
//    Variable x1("x1", "s", 2);
//    Variable y("y", "s", 2);
//    Variable z("z", "s", 2);
//    Variable z2("z2", "s", 2);
//
//    //opp names add/sub, mult, logic, div/mod
//    Operation op1; // z = x + y
//    op1.result = z;
//    op1.equals = "=";
//    op1.var1 = x;
//    op1.var2 = y;
//    op1.op1 = "+";
//    op1.bits = 2;
//    op1.type = "s";
//    op1.name = "ADD";
//
//    listOfOps.push_back(op1);
//
//    Operation op2; //x1 = x-w
//    op2.result = x1;
//    op2.equals = "=";
//    op2.var1 = x;
//    op2.var2 = w;
//    op2.op1 = "-";
//    op2.bits = 2;
//    op2.type = "s";
//    op2.name = "SUB";
//
//    listOfOps.push_back(op2);
//
//    Operation op3; //w = z*x1
//    op3.result = w;
//    op3.equals = "=";
//    op3.var1 = z;
//    op3.var2 = x1;
//    op3.op1 = "*";
//    op3.bits = 4;
//    op3.type = "s";
//    op3.name = "MULT";
//
//    listOfOps.push_back(op3);
//
//    Operation op4; //z2 = w/z
//    op4.result = z2;
//    op4.equals = "=";
//    op4.var1 = w;
//    op4.var2 = z;
//    op4.op1 = "/";
//    op4.bits = 4;
//    op4.type = "s";
//    op4.name = "DIV";
//
//    listOfOps.push_back(op4);

    vector<Variable> inputs;
    vector<Variable> outputs;
    vector<Variable> vars;
    vector<string> operations;
    bool validCircuit = true;
    bool* validPointer;
    validPointer = &validCircuit;
    
    vector<string> parsedFile = ReadFile(argv[1], validPointer);
     if(!validCircuit){
        cout << "ERROR : Failure reading file."<< endl;
        return 1;
    }
    for(vector<string>::iterator i = parsedFile.begin(); i != parsedFile.end(); ++i){
        ParseLine(inputs, outputs, vars, operations, *i);
    }
    
    PrintOperands(inputs, outputs, vars, operations);
    
    // validate operations
    vector<Operation> ops = processOperation(operations, inputs, outputs, vars, validPointer);

    Graph g;
    //creating all of the nodes for the graph
    g.populateNodes(ops);
    cout << "####### PRINTING NODES #######" << endl;
    g.printNodes();
    g.cdfg();
 

    int latency = atoi(argv[2]);
    g.schedule(latency, validPointer);
    
    cout << "####### RUNNING FDS #######" << endl;
    // one last check before printing the circuit
     if(!validCircuit){
        cout << "ERROR : Latency Error."<< endl;
        return 1;
    }
    FDS(g, latency);
    

    //where the file writing starts
    ofstream myfile (argv[3]);
    if(myfile.is_open()){
        VerilogHLSM(g,inputs,outputs,vars, myfile);
        myfile.close();
    }
    else cout << "Unable to open output file";
    
    return 0;
}
