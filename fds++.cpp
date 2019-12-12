
//  fds.cpp
//  574
//  Force Directed Scheduler
//
//  Created by Kristopher Rockowitz on 11/30/19.
//  Copyright © 2019 r0. All rights reserved.
//

//            // *** Idea about selecting nodes where the predecessor force is guaranteed to be 0 ***
//            // check if the parent size is 0 or if all parents are scheduled, then schedule a node
//            bool scheduled = true;
//            for(unsigned int p = 0; p < i->parent.size(); p++){
//                if(!(i->parent.at(p)->scheduled)){
//                    scheduled = false;
//                }
//            }
//            if(i->parent.size() == 0 || scheduled){
//              // then do things
//            }

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
    int selfForce; // SUM(DG(i) * deltaX(i)) where DG(i) is time probability and
    int fds;
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
        this->fds = 0;
        this->num = 0;
    }
};

class Graph{ // maybe, I don't really know how graphs work
    public:
    vector<Node> vertices;
    Graph(){
    }
};

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
            else if(node.operation.name == "COMP" || node.operation.name == "MUX" || node.operation.name == "SHR" || node.operation.name == "SHL"){
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
        else if(node.operation.name == "COMP" || node.operation.name == "MUX" || node.operation.name == "SHR" || node.operation.name == "SHL"){
            selfForce += dg_logic.at(time-1);
        }
    }
    return selfForce;
}

// calculates the predecessor forces for the given node
void PredecessorForce(Node s, int time, int size, vector<float> dg_mult, vector<float> dg_addsub, vector<float> dg_divmod, vector<float> dg_logic) {
  // s is the node, time is the current scheduling time, size is the number of total nodes

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
            cout << s.num << " ";
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
    cout << "level: " << level << endl;
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

// changes times for nodes
//void ChangeTimes(Node &s, int time, int size, vector<float> dg_mult, vector<float> dg_addsub, vector<float> dg_divmod, vector<float> dg_logic) {
//  // s is the node, time is the current scheduling time, size is the number of total nodes
//
//    // change node to scheduled
//    s.scheduled = true;
//    s.alap = time;
//    s.asap = time;
//
//
//
//    // TODO: add functionality so that when pred/succ forces are calculated, they set all the timeframes of connected nodes
//
//
//    // Initially mark all verices as not visited
//    vector<bool> visited(size, false);
//
//    // Create a stack for DFS
//    stack<Node> stack;
//
//    // Push the current source node.
//    stack.push(s);
//    // level used for time calculation
//    int level = 0;
//
//    int x = 0;
//    while (!stack.empty())
//    {
//        // Pop a vertex from stack and print it
//        s = stack.top();
//        stack.pop();
//
//        if (!visited[s.num]){
////            cout << s.num << " ";
//            visited[s.num] = true;
//            // if it's not the first node, evaluate whether pred force exists
//            if(x > 0){
//                if(s.alap >= time){
//                    s.alap = time - 1;
//                }
//            }
//        }
//        x++;
//
//        // increment the level every time the number of parents > 0
//        if(s.parent.size() > 0){
//            level++;
//        }
//
//        for (unsigned int i = 0; i < s.parent.size(); i++){
//            // check if the parent num is in the visisted array
//            if (!visited[s.parent.at(i)->num]){
//                stack.push(*s.parent.at(i));
//            }
//        }
//    }
////    cout << "level: " << level << endl;
//
//    // Initially mark all verices as not visited
//      vector<bool> visited2(size, false);
//
//      // Push the current source node.
//      stack.push(s);
//      // level used for time calculation
//      level = 0;
//
//      x = 0;
//      while (!stack.empty())
//      {
//          // Pop a vertex from stack and print it
//          s = stack.top();
//          stack.pop();
//
//          if (!visited[s.num]){
////              cout << s.num << " ";
//              visited[s.num] = true;
//              // if it's not the first node, evaluate whether pred force exists
//              if(x > 0){
//                  if(s.asap <= time){
//                      s.asap = time + 1;
//                  }
//              }
//          }
//          x++;
//
//          // increment the level every time the number of parents > 0
//          if(s.children.size() > 0){
//              level++;
//          }
//
//          for (unsigned int i = 0; i < s.children.size(); i++){
//              // check if the parent num is in the visisted array
//              if (!visited[s.children.at(i)->num]){
//                  stack.push(*s.children.at(i));
//              }
//          }
//      }
//
//}


//// calculates the successor forces for the given node
//void SuccessorForce(Node s, int time, int size, vector<float> dg_mult, vector<float> dg_addsub, vector<float> dg_divmod, vector<float> dg_logic) {
//  // s is the node, time is the current scheduling time, size is the number of total nodes
//
//    // Initially mark all verices as not visited
//    vector<bool> visited(size, false);
//
//    // Create a stack for DFS
//    stack<Node> stack;
//
//    // Push the current source node.
//    stack.push(s);
//    // level used for time calculation
//    int level = 0;
//
//    //intialize successor force
//    float successorForce = 0.0;
//    int x = 0;
//    while (!stack.empty())
//    {
//        // Pop a vertex from stack and print it
//        s = stack.top();
//        stack.pop();
//
//        if (!visited[s.num]){
//            cout << s.num << " ";
//            visited[s.num] = true;
//            // if it's not the first node, evaluate whether pred force exists
//            if(x > 0){
//                float sf = 0.0;
//                // if the pred node has an alap that is greater than adj_time, calc self force at adj_time
//                int adj_time = time + level - 1;
//                if(s.asap <= adj_time){
//                    sf = SelfForce(s, adj_time + 1, dg_mult, dg_addsub, dg_divmod, dg_logic);
//                }
//                else{
//                    sf = 0.0;
//                }
//                successorForce += sf;
//            }
//        }
//        x++;
//
//        // increment the level every time the number of parents > 0
//        if(s.children.size() > 0){
//            level++;
//        }
//
//        for (unsigned int i = 0; i < s.children.size(); i++){
//            // check if the parent num is in the visisted array
//            if (!visited[s.children.at(i)->num]){
//                stack.push(*s.children.at(i));
//            }
//        }
//    }
//    cout << "level: " << level << endl;
//}

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
                dg_divmod[j] += 1 / (mobility + 1);
            }
        }
        if(i->operation.name == "ADD" || i->operation.name == "SUB"){
            // loop over the time intervals this operation covers
            // start at asap, end at alap
            for(unsigned int j = i->asap; j <= i->alap; j++){
                // accumulate the probability for this component on the total
                // type probability for this specific time
                dg_addsub[j] += 1 / (mobility + 1);
            }
        }
        if(i->operation.name == "COMP" || i->operation.name == "MUX" || i->operation.name == "SHR" || i->operation.name == "SHL"){
            // loop over the time intervals this operation covers
            // start at asap, end at alap
            for(unsigned int j = i->asap; j <= i->alap; j++){
                // accumulate the probability for this component on the total
                // type probability for this specific time
                dg_logic[j] += 1 / (mobility + 1);
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


int main() {

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
    
    // Call the FDS function given a Graph g, and latency l
    FDS(g, 4);
    
}


