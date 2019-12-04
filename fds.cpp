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
    string output; // max 1 output
    int asap; // this is the first element of timeframe for FDS
    int alap; // this is the second element of timeframe for FDS
    int mobility; // alap - asap. also fyi, width = mobility + 1
    bool scheduled; // default false, helps to shrink each iteration of FDS
    int selfForce; // SUM(DG(i) * deltaX(i)) where DG(i) is time probability and deltaX(i) is
    Node(){
        this->operation = Operation();
        this->output = "";
        this->alap = 0;
        this->asap = 0;
        this->mobility = 0;
        this->scheduled = false;
        this->selfForce = 0;
    }
};

class Graph{ // maybe, I don't really know how graphs work
    public:
    vector<Node> vertices;
    vector<string> edges;
    
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


