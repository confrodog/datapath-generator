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
    Node *parent;
    vector<Node*> children;
    string output; // max 1 output
    int asap; // this is the first element of timeframe for FDS
    int alap; // this is the second element of timeframe for FDS
    int mobility; // alap - asap. also fyi, width = mobility + 1
    bool scheduled; // default false, helps to shrink each iteration of FDS
    int selfForce; // SUM(DG(i) * deltaX(i)) where DG(i) is time probability and deltaX(i) is
    Node(){
        this->parent = NULL;
        this->children = {};
        this->inputs = {};
        this->operation = Operation();
        this->output = "";
        this->alap = 0;
        this->asap = 0;
        this->mobility = 0;
        this->scheduled = false;
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
    cout << "checking to make sure I can build this correctly\n";
    vector<Operation> listOfOps;

    Variable w("w", "int", 2);
    Variable x("x", "int", 2);
    Variable x1("x1", "int", 2);
    Variable y("y", "int", 2);
    Variable z("z", "int", 2);
    Variable z2("z", "int", 2);

    //opp names add/sub, mult, logic, div/mod
    Operation op1; // z = x + y
    op1.result = z;
    op1.equals = "=";
    op1.var1 = x;
    op1.var2 = y;
    op1.op1 = "+";
    op1.bits = 2;
    op1.type = "s";
    op1.name = "add/sub";

    listOfOps.push_back(op1);

    Operation op2; //x1 = x-w
    op2.result = x1;
    op2.equals = "=";
    op2.var1 = x;
    op2.var2 = w;
    op2.op1 = "-";
    op2.bits = 2;
    op2.type = "s";
    op2.name = "add/sub";
    
    listOfOps.push_back(op2);

    Operation op3; //w = z*x1
    op3.result = w;
    op3.equals = "=";
    op3.var1 = z;
    op3.var2 = x1;
    op3.op1 = "*";
    op3.bits = 4;
    op3.type = "s";
    op3.name = "mult";

    listOfOps.push_back(op3);

    Operation op4; //z2 = w/z
    op4.result = z2;
    op4.equals = "=";
    op4.var1 = w;
    op4.var2 = z;
    op4.op1 = "/";
    op4.bits = 4;
    op4.type = "s";
    op4.name = "div/mod";

    listOfOps.push_back(op4);

    Graph g;
    //creating all of the nodes for the graph
    for(int i = 0; i < listOfOps.size(); i++) {
        Node curr;
        Operation currentOp = listOfOps.at(i);
        curr.operation = currentOp;
        curr.inputs.push_back(currentOp.var1.name);
        curr.inputs.push_back(currentOp.var2.name);
        curr.output = currentOp.result.name;
        g.vertices.push_back(curr);
    }

    vector<Node> visitedNodes = {}; //keep track of what nodes have been seen
    vector<string> visistedOutputs = {};
    for(int i = 0; i < g.vertices.size(); i++) {
        Node current = g.vertices.at(i);
        for(int j = 0; j < current.inputs.size(); j++) {
            cout << current.inputs.at(j) << endl;
            cout << "Number of outputs: ";
            cout << visistedOutputs.size() << endl;
            string inp = current.inputs.at(j); 
            if (find(visistedOutputs.begin(), visistedOutputs.end(), inp) != visistedOutputs.end()) {
                cout << "Someone has a dependency" << endl;
                //found, this means that the input is someone's output
                //so this current node must be a child of the node with that output
                current.parent = &visitedNodes.at(j);
                visitedNodes.at(j).children.push_back(&current);
            } else {
                current.parent = &g.noop;
            }
        } 
        visitedNodes.push_back(current);
        visistedOutputs.push_back(current.output);
    }

    return 0;
}