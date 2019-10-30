//
//  main.cpp
//  574
//
//  Created by Kristopher Rockowitz on 10/25/19.
//  Copyright © 2019 r0. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;


// inputs, outputs, registers, wires are of this type
class Variable {
    public:
    string name;
    string type;
    int bits;
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
class Operation {
    public:
    string result;
    string equals;
    string var1;
    string var2;
    string var3;
    string op1;
    string op2;
    string name;
    string type;
    int bits;
    
    Operation(){
        this->result = "";
        this->equals = "";
        this->var1 = "";
        this->var2 = "";
        this->var3 = "";
        this->op1 = "";
        this->op2 = "";
        this->name = "";
        this->type = "";
        this->bits = 0;
    }
};

// search single vector, 1 if founde, else 0
int findVariable(vector<Variable> in, string find){
    int found = 0;
    for(unsigned int i = 0; i < in.size(); i++){
        if(in.at(i).name == find){
            found = 1;
        }
    }
    return found;
}

// search single vector and return the Variable, else empty Variable
Variable getVariable(vector<Variable> in, string find){
    Variable found;
    for (std::vector<Variable>::iterator it = in.begin() ; it != in.end(); ++it){
        if(it->name == find){
            return *it;
        }
    }
    return found;
}

// search all the vectors and return the Variable, else empty Variable
Variable searchVariables(vector<Variable> in, vector<Variable> out, vector<Variable> wire, vector<Variable> reg, string find){
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
    for(it = wire.begin(); it != wire.end(); ++it){
        if(it->name == find){
            return *it;
        }
    }
    for(it = reg.begin(); it != reg.end(); ++it){
        if(it->name == find){
            return *it;
        }
    }
    Variable var;
    return var;
}

// write header
void writeHeader(){
    cout << "`timescale 1ns / 1ps \n" << endl;
}

// write module
void writeModule(string name, vector<Variable> in, vector<Variable> out){
    cout << "module " << name << "(";
    std::vector<Variable>::iterator it;
    for ( it = in.begin() ; it != in.end(); ++it){
        cout << it->name << ",";
    }
    for (it = out.begin() ; it != out.end(); ++it){
        if(it == out.end()-1){
            cout << it->name;
        }
        else{
            cout << it->name << ",";
        }
    }
    cout << ");" << endl << endl;
}

// write inputs
void writeInputs(vector<Variable> var){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "input ";
        if(it->type == "s"){
            cout << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}

// write outputs
void writeOutputs(vector<Variable> var){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "output ";
        if(it->type == "s"){
            cout << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}

// write wires
void writeWires(vector<Variable> var){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "wire ";
        if(it->type == "s"){
            cout << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}

// write registers
void writeRegisters(vector<Variable> var){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "register ";
        if(it->type == "s"){
            cout << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}
// write operations
void writeOperations(vector<Operation> ops){
    unsigned int i = 0;
    for (std::vector<Operation>::iterator it = ops.begin() ; it != ops.end(); ++it){
        cout << it->name << " #(.DATAWIDTH(" << it->bits << ")) " << it->name << i << "(" << it->var1 << ",";
        if(it->var2 != "")
            cout << it->var2 << ",";
        if(it->var3 != "")
            cout << it->var3 << ",";
        cout << it->result << ");" << endl;
        ++i;
    }
    cout << endl << "endmodule" << endl;
}




int main(int argc, const char * argv[]) {
    // exits if valid is false
    bool validCircuit = true;
                 
    // store original circuit name
    string circuitName = "circuit1";

    // inputs
    Variable in0("a", "u", 8);
    Variable in1("b", "s", 8);
    Variable in2("c", "u", 8);
    vector<Variable> inputs = {in0, in1, in2};
    
    // outputs
    Variable out0("z", "s", 8);
    Variable out1("x", "s", 16);
    vector<Variable> outputs = {out0, out1};

    // wires
    Variable wi0("d", "s", 8);
    Variable wi1("e", "u", 8);
    Variable wi2("f", "s", 16);
    Variable wi3("g", "u", 16);
    Variable wi4("xwire", "s", 16);
    vector<Variable> wires = {wi0, wi1, wi2, wi3, wi4};
    
    // registers
    Variable reg0("greg", "s", 64);
    Variable reg1("hreg", "s", 64);
    vector<Variable> registers = {reg0, reg1};
    
    // operations directly from file, stored as strings in vector
    vector<string> operations = {"d = a", "d = a ? b : c", "g = d - 1", "d = a > f"};
    // processed operations done by yours truly
    vector<Operation> ops;
    

    // TODO: move this shit to a function
    // create a stringstream to parse the elements of the operation line string
    // iterate through the operation vector
    for(unsigned int i = 0; i < operations.size() ; i++){
        stringstream opstream;
        string result, equals, var1, op1, var2, op2, var3;
        // throw the line into the stream
        opstream << operations.at(i);
        // stream into the variables
        opstream >> result >> equals >> var1 >> op1 >> var2 >> op2 >> var3;
        
        // validate operators and create new Operation, if valid
        Operation o1;
        o1.result = result;
        o1.equals = equals;
        o1.var1 = var1;
        o1.op1 = op1;
        o1.var2 = var2;
        o1.op2 = op2;
        o1.var3 = var3;
        
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
            // enforce the correct equality operation
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
            cout << "invalid operator: unexpected operator type, circuit invalidated" << endl;
            validCircuit = false;
            return -1;
        }

        // validate result
        Variable r1 = searchVariables(inputs, outputs, wires, registers, result);
        if(r1.bits == 0){
            //didn't find it if bits are 0 (default)
            cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
            validCircuit = false;
            return -1;
        }
        else if(o1.name != "COMP"){
            // if not comparator, set the operation bits to the result bits
            o1.bits = r1.bits;
        }
        
        // validate the equals operator
        if(equals != "="){
            // must be equal operator, lose the circuit
            cout << "invalid operator: expected '='. Circuit invalidated!" << endl;
            validCircuit = false;
            return -1;
        }

        // validate var1
        // mandatory: has to be either input, output, register or wire
        
        // keep track of max bit width for variables
        int max = 0;
        Variable v1 = searchVariables(inputs, outputs, wires, registers, var1);
        if(v1.bits == 0){
            //didn't find it if bits are 0 (default)
            cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
            validCircuit = false;
            return -1;
        }
        else if(o1.name == "COMP" && v1.bits > max){
            // if not comparator, set the operation bits to the result bits
            o1.bits = v1.bits;
        }
        
        // check var3 and conditon for COMP
        Variable v2 = searchVariables(inputs, outputs, wires, registers, var2);
        if(v2.bits == 0 && o1.name != "INC" && o1.name != "REG" && o1.name != "DEC"){
            //didn't find it if bits are 0 (default)
            cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
            validCircuit = false;
            return -1;
        }
        else if(o1.name == "COMP" && v2.bits > max){
            // if not comparator, set the operation bits to the result bits
            o1.bits = v2.bits;
        }
        
        Variable v3 = searchVariables(inputs, outputs, wires, registers, var3);
        if(v3.bits != 0 && o1.name != "MUX2x1"){
            //didn't find it if bits are 0 (default)
            cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
            validCircuit = false;
            return -1;
        }
        else if(o1.name == "COMP" && v3.bits > max){
            // if not comparator, set the operation bits to the result bits
            o1.bits = v3.bits;
        }
        // TODO: signed integer handling, if a single input is signed, the whole component is signed.
        
        
        // TODO: figure out the comparator situation, how it knows whether to place the component in GT, LT, EQ

        // push the validated operation into ops vector
        ops.push_back(o1);

    }
    
    // write the verilog code (just cout for now)
    writeHeader();
    writeModule(circuitName, inputs, outputs);
    writeInputs(inputs);
    writeOutputs(outputs);
    writeWires(wires);
    writeRegisters(registers);
    writeOperations(ops);

    return 0;
}
