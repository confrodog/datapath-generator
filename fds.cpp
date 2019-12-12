//
//  fds.cpp
//  574
//  Force Directed Scheduler
//
//  Created by Kristopher Rockowitz on 11/30/19.
//  Copyright © 2019 r0. All rights reserved.
//

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
    }
};

class Graph{ // will contain the vector of all the edges, where the edges have the children/parent information
    public:
    vector<Node> vertices; //a list of all nodes in this graph
    Node noop;
    Node sink; //I'm thinking these might be helpful
    Graph(){
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

// assume inputs are a graph G(V,E) and lambda.
// assume each vertex has a field that gives it's ASAP, ALAP time.
void FDS(Graph g, int lambda){
    
    // DG(i) variables (probabilities per type at a given time)
    // intialized to 0 and size latency constraint lambda (time)
    vector<float> dg_mult(lambda, 0);
    vector<float> dg_addsub(lambda, 0);
    vector<float> dg_logic(lambda, 0);
    vector<float> dg_divmod(lambda, 0);
    
    // 1) cacl timeframes
    // 2) calc the operation and type probabilites
    // 3) calc self forces, predecessor/successor forces and total forces
    // 4) schedule the operation with the least force and update its timeframe
    
    bool allScheduled = false;
    // main FDS loop, goes until all nodes scheduled
    while(!allScheduled){
        
        // calc the timeframes, this is implicitly done as we go
        
        // calc type probabilities for each time in lambda
        vector<Node>::iterator i;
        // loop over all vertices / nodes, same thing
        for(i = g.vertices.begin(); i != g.vertices.end(); i++){
            // examine timeframe [asap, alap] and calc probability
            // find the operation type so you know which vector to add to
            if(i->operation.name == "MULT"){
                // loop over the time intervals this operation covers
                // start at asap, end at alap
                for(unsigned int j = i->asap; j < i->alap; j++){
                    // accumulate the probability for this component on the total
                    // type probability for this specific time
                    dg_mult.at(j) += 1 / (i->mobility + 1);
                }
            }
            if(i->operation.name == "DIV" || i->operation.name == "MOD"){
                // loop over the time intervals this operation covers
                // start at asap, end at alap
                for(unsigned int j = i->asap; j < i->alap; j++){
                    // accumulate the probability for this component on the total
                    // type probability for this specific time
                    dg_divmod.at(j) += 1 / (i->mobility + 1);
                }
            }
            if(i->operation.name == "ADD" || i->operation.name == "SUB"){
                // loop over the time intervals this operation covers
                // start at asap, end at alap
                for(unsigned int j = i->asap; j < i->alap; j++){
                    // accumulate the probability for this component on the total
                    // type probability for this specific time
                    dg_addsub.at(j) += 1 / (i->mobility + 1);
                }
            }
            if(i->operation.name == "COMP" || i->operation.name == "MUX" || i->operation.name == "SHR" || i->operation.name == "SHL"){
                // loop over the time intervals this operation covers
                // start at asap, end at alap
                for(unsigned int j = i->asap; j < i->alap; j++){
                    // accumulate the probability for this component on the total
                    // type probability for this specific time
                    dg_logic.at(j) += 1 / (i->mobility + 1);
                }
            }
        } // end probability for
        
        // calc the forces
        // loop through each node
        float lowestForce = -numeric_limits<float>::max(); // create the lowest possible float val
        
        for(i = g.vertices.begin(); i != g.vertices.end(); i++){
            // self force = DG(i) * deltaX(i) for alap - asap number of terms
            // for each timeframe for that node, calc the self force, pred, succ forces
            for(unsigned int j = i->asap; j < i->alap; j++){
                float selfForce = 0;
                
            }
        }
    } // end while
}
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

int main() {
    vector<Operation> listOfOps; //create a list of test operations

    Variable w("w", "s", 2);
    Variable x("x", "s", 2);
    Variable x1("x1", "s", 2);
    Variable y("y", "s", 2);
    Variable z("z", "s", 2);
    Variable z2("z2", "s", 2);

    //opp names add/sub, mult, logic, div/mod
    Operation op1; // z = x + y
    op1.result = z;
    op1.equals = "=";
    op1.var1 = x;
    op1.var2 = y;
    op1.op1 = "+";
    op1.bits = 2;
    op1.type = "s";
    op1.name = "ADD";

    listOfOps.push_back(op1);

    Operation op2; //x1 = x-w
    op2.result = x1;
    op2.equals = "=";
    op2.var1 = x;
    op2.var2 = w;
    op2.op1 = "-";
    op2.bits = 2;
    op2.type = "s";
    op2.name = "SUB";
    
    listOfOps.push_back(op2);

    Operation op3; //w = z*x1
    op3.result = w;
    op3.equals = "=";
    op3.var1 = z;
    op3.var2 = x1;
    op3.op1 = "*";
    op3.bits = 4;
    op3.type = "s";
    op3.name = "MULT";

    listOfOps.push_back(op3);

    Operation op4; //z2 = w/z
    op4.result = z2;
    op4.equals = "=";
    op4.var1 = w;
    op4.var2 = z;
    op4.op1 = "/";
    op4.bits = 4;
    op4.type = "s";
    op4.name = "DIV";

    listOfOps.push_back(op4);

    vector<Variable> inputs;
    vector<Variable> outputs;
    vector<Variable> vars;
    vector<string> operations;
    bool validCircuit = true;
    bool* validPointer;
    validPointer = &validCircuit;
    vector<string> parsedFile = ReadFile("C://Users//frees//Documents//School//ECE 474//datapath-generator//test.txt", validPointer);
    
    for(vector<string>::iterator i = parsedFile.begin(); i != parsedFile.end(); ++i){
        ParseLine(inputs, outputs, vars, operations, *i);
    }
    
    PrintOperands(inputs, outputs, vars, operations);
    
    // validate operations
    vector<Operation> ops = processOperation(operations, inputs, outputs, vars, validPointer);

    Graph g;
    //creating all of the nodes for the graph
    g.populateNodes(ops);
    g.cdfg();
    g.schedule(6);
    return 0;
}