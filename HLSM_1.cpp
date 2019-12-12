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
    int fds; //time frame that the node is scheduled to (post-fds)
    int num;
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
    void alap(int latency) {
        for(int i = latency; i >= 1; i--){
            for(int j = this->vertices.size() - 1; j >= 0; j--) {//iterate backwards between nodes
                if(!this->vertices.at(j).alapScheduled) {
                    if(this->vertices.at(j).children.size() == 0) {//schedule, no children, bottom node
                        this->vertices.at(j).alapScheduled = true;
                        this->vertices.at(j).alap = i;
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
                            string currentOp = this->vertices.at(j).operation.name;
                            if(currentOp.compare("MULT") == 0) {//2 cycle delay
                                if(i <= earliestSchedule - 2) {
                                    this->vertices.at(j).alap = i;
                                    this->vertices.at(j).alapScheduled = true;
                                }
                            }
                            else if(currentOp.compare("DIV") == 0) {//3 cycle delay
                                if(i <= earliestSchedule - 3) {
                                    this->vertices.at(j).alap = i;
                                    this->vertices.at(j).alapScheduled = true;
                                }
                            }
                            else if(currentOp.compare("MOD") == 0) {//3 cycle delay
                                if(i <= earliestSchedule - 3) {
                                    this->vertices.at(j).alap = i;
                                    this->vertices.at(j).alapScheduled = true;
                                }
                            }
                            else {
                                if(i <= earliestSchedule - 1) {
                                    this->vertices.at(j).alap = i;
                                    this->vertices.at(j).alapScheduled = true;
                                }
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
        }
    }
    void asap(int latency) {
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
                                string parentOp = this->vertices.at(j).parent.at(k)->operation.name;
                                int tempLatest = 0;
                                if(parentOp.compare("MULT") == 0) {//2 cycle delay
                                    tempLatest = this->vertices.at(j).parent.at(k)->asap + 2;
                                }
                                else if(parentOp.compare("DIV") == 0) {//3 cycle delay
                                    tempLatest = this->vertices.at(j).parent.at(k)->asap + 3;
                                }
                                else if(parentOp.compare("MOD") == 0) {//3 cycle delay
                                    tempLatest = this->vertices.at(j).parent.at(k)->asap + 3;
                                }
                                else {
                                    tempLatest = this->vertices.at(j).parent.at(k)->asap + 1;
                                }
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
        }
    }
};

void writeModule(vector<Variable> inputs, vector<Variable> outputs, ofstream &file){
    cout << "module HLSM (clk, rst, Start, Done, ";
    file << "module HLSM (clk, rst, Start, Done, ";
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
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
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
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}

int writeParameters(Graph g, ofstream &file){
    int numStates = 0;
    for(vector<Node>::iterator it = g.vertices.begin(); it != g.vertices.end(); it++){
        if((*it).fds>numStates){
            numStates = (*it).fds;
        }
    }
    
    for(int i = 1; i< numStates+1;i++){
        if(i == numStates){
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
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
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
    
    while(pow(2,numBits) < numStates){
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

int main(int argc, const char * argv[]){
    string verilogFile = argv[3];
    ofstream myfile (verilogFile + ".v");
    
    //Debug values
    
    vector<Variable> inputs;
    vector<Variable> outputs;
    vector<Variable> variables;
    
    Variable i1,i2,i3,i4,o1,o2,o3,o4,v1,v2;
    
    i1.bits = 4;
    i2.bits = 4;
    i3.bits = 4;
    i4.bits = 4;
    i1.name = "shit";
    i2.name = "piss";
    i3.name = "cum";
    i4.name = "tits";
    
    o1.bits = 4;
    o2.bits = 4;
    o3.bits = 4;
    o4.bits = 4;
    o1.name = "shitOut";
    o2.name = "pissOut";
    o3.name = "cumOut";
    o4.name = "titsOut";
    
    v1.bits = 4;
    v2.bits = 4;
    v1.name = "jizz";
    v2.name = "heyThere;)";
    
    inputs.push_back(i1);
    inputs.push_back(i2);
    inputs.push_back(i3);
    inputs.push_back(i4);
    
    outputs.push_back(o1);
    outputs.push_back(o2);
    outputs.push_back(o3);
    outputs.push_back(o4);
    
    variables.push_back(v1);
    variables.push_back(v2);
    
    Operation op1, op2, op3, op4;
    
    op1.name = "MULT";
    op1.type = "s";
    op1.bits = 8;
    op1.result = Variable("w", "s", 8);
    op1.equals = "=";
    op1.op1 = "*";
    op1.var1 = Variable("a", "s", 8);
    op1.var2 = Variable("b", "s", 8);
    
    op2.name = "MULT";
    op2.type = "s";
    op2.bits = 8;
    op2.result = Variable("x", "s", 8);
    op2.equals = "=";
    op2.op1 = "*";
    op2.var1 = Variable("c", "s", 8);
    op2.var2 = Variable("d", "s", 8);
    
    op3.name = "MULT";
    op3.type = "s";
    op3.bits = 8;
    op3.result = Variable("y", "s", 8);
    op3.equals = "=";
    op3.op1 = "*";
    op3.var1 = Variable("e", "s", 8);
    op3.var2 = Variable("f", "s", 8);
    
    op4.name = "MULT";
    op4.type = "s";
    op4.bits = 8;
    op4.result = Variable("z", "s", 8);
    op4.equals = "=";
    op4.op1 = "*";
    op4.var1 = Variable("g", "s", 8);
    op4.var2 = Variable("h", "s", 8);
    
    Node n1, n2, n3, n4;
    n1.num = 1;
    n2.num = 2;
    n3.num = 3;
    n4.num = 4;
    n1.operation = op1;
    n2.operation = op2;
    n3.operation = op3;
    n4.operation = op4;
    n1.asap = 1;
    n2.asap = 1;
    n3.asap = 2;
    n4.asap = 3;
    n1.alap = 2;
    n2.alap = 2;
    n3.alap = 3;
    n4.alap = 4;
    
    n1.fds = 1;
    n2.fds = 1;
    n3.fds = 2;
    n4.fds = 3;
    
    n4.parent.push_back(&n3);
    n3.children.push_back(&n4);
    n3.parent.push_back(&n2);
    n3.parent.push_back(&n1);
    n1.children.push_back(&n3);
    n2.children.push_back(&n3);
    
    Graph g;
    g.vertices.push_back(n1);
    g.vertices.push_back(n2);
    g.vertices.push_back(n3);
    g.vertices.push_back(n4);
    
    //where the file writing starts
    if(myfile.is_open()){
        VerilogHLSM(g,inputs,outputs,variables, myfile);
    }
    else cout << "Unable to open file";
    
    return 0;
}

/*
 NOTES:
 
 - keep vector<Variable> for input, outputs when reading in file
    -easier for verilog output
 - keep track of longest time in FDS and store in Graph object
    -makes # of parameters(states) easier to write
 - how tracking variables?
    - need to make these registers
 
 */
