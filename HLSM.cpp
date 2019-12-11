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


void VerilogHLSM(Graph g, ofstream& file){
    //STATIC timescale
    cout << "`timescale 1ns / 1ps \n" << endl;
    file << "`timescale 1ns / 1ps \n" << endl;

    //STATIC module
    cout << "module HLSM (Clk, Rst, Start, Done);\n";
    file << "module HLSM (Clk, Rst, Start, Done);\n";

    //inputs
        //STATIC inputs
        cout << "\tinput Clk, Rst, Start\n";
        file << "\tinput Clk, Rst, Start\n";
        //DYNAMIC inputs (TODO)

    //outputs
        //STATIC outputs
        cout << "\toutput reg Done\n";
        file << "\toutput reg Done\n";

        //DYNAMIC outputs (TODO)

    //parameters (states)

    cout << "\tparameter S0 = 0\n";
    file << "\tparameter S0 = 0\n";

    //registers (interim states and states)

    cout << "\treg ["<< 2 << ":0] State\n"; //replace 2 with number of bits needed for states
    file << "\treg ["<< 2 << ":0] State\n";

    //STATIC always block
    cout << "\talways @(posedge Clk) begin\n";
    file << "\talways @(posedge Clk) begin\n";

    //STATIC if (Rst) begin
    cout << "\t\tif(Rst) begin\n";
    file << "\t\tif(Rst) begin\n";

    //STATIC end/else for reset condition and case statement
    cout << "\t\tend\n\t\telse begin\n\t\t\tcase(State)\n";
    file << "\t\tend\n\t\telse begin\n\t\t\tcase(State)\n";

    //STATIC wait state
    cout << "\t\t\t\tS0: begin\n\t\t\t\t\tif (Start) begin\n\t\t\t\t\t\tState <= S1;\n\t\t\t\t\tend\n"
     << "\t\t\t\t\telse begin\n\t\t\t\t\t\tState <= S0;\n\t\t\t\t\tend\n\t\t\t\tend\n";
    file << "\t\t\t\tS0: begin\n\t\t\t\t\tif (Start) begin\n\t\t\t\t\t\tState <= S1;\n\t\t\t\t\tend\n"
     << "\t\t\t\t\telse begin\n\t\t\t\t\t\tState <= S0;\n\t\t\t\t\tend\n\t\t\t\tend\n";

    //STATIC end for else and endcase
    cout << "\t\t\tendcase\n\t\tend\n";
    file << "\t\t\tendcase\n\t\tend\n";

    //STATIC end for always
    cout << "\tend\nendmodule";
    file << "\tend\nendmodule";
}

int main(int argc, const char * argv[]){
    string verilogFile = argv[3];
    ofstream myfile (verilogFile + ".v");

    //Debug values
    Graph g;

    //where the file writing starts
    if(myfile.is_open()){
        VerilogHLSM(g, myfile);
    }
    else cout << "Unable to open file";

    return 0;
}