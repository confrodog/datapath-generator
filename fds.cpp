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
    void alap(int latency) {
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

    Graph g;
    //creating all of the nodes for the graph
    g.populateNodes(listOfOps);
    g.cdfg();
    g.alap(6);
    return 0;
}