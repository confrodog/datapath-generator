//
//  main.cpp
//  574
//
//  Created by Kristopher Rockowitz on 10/25/19.
//  Copyright © 2019 r0. All rights reserved.
//

#include <iostream>
#include <fstream>
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
    Variable result;
    string equals;
    Variable var1;
    Variable var2;
    Variable var3;
    string op1;
    string op2;
    string name;
    string type;
    int bits;
    
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

vector<string> ReadFile(string inputFile){
    cout << "Reading File...\n";
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
        cout << "unable to read file";
    }
    cout << "Done reading file.";
    return parsedFile;
}

void PrintOperands(vector<Variable> &inputs, vector<Variable> &outputs, vector<Variable> &wires, vector<Variable> &registers, vector<string>& operations){
    cout << "Inputs:\n";
    for(vector<Variable>::iterator i = inputs.begin(); i != inputs.end(); i++){
        cout << distance(inputs.begin(), i)+1 << ". "<<  (*i).name << " is " << (*i).type << " and has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nOutputs:\n";
    for(vector<Variable>::iterator i = outputs.begin(); i != outputs.end(); i++){
        cout << distance(outputs.begin(), i)+1 << ". "<<  (*i).name << " is " << (*i).type << " and has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nWires:\n";
    for(vector<Variable>::iterator i = wires.begin(); i != wires.end(); i++){
        cout << distance(wires.begin(), i)+1 << ". "<<  (*i).name << " is " << (*i).type << " and has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nRegisters:\n";
    for(vector<Variable>::iterator i = registers.begin(); i != registers.end(); i++){
        cout << distance(registers.begin(), i)+1 << ". "<<  (*i).name << " is " << (*i).type << " and has bitwidth " << (*i).bits << "\n";
    }
    cout << "\nOperations:\n";
    for(vector<string>::iterator i = operations.begin(); i != operations.end(); i++){
            cout << (*i) << "\n";
    }
}

void ParseLine(vector<Variable>& inputs, vector<Variable>& outputs,vector<Variable>& wires, vector<Variable>& registers, vector<string>& operations, string line){
    //parse into inputs, outputs, wires and operations

    if(line.find("input")==0){
        //input Int8 a, b, c
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        // string U = &line.at(line.find("Int")-1);
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
                if(U == 'U'){
                    input.type = "u";
                }
                else{
                    input.type = "s";
                }
                input.bits = BW;
                cout << "adding " << input.name << " to inputs\n";
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
                if(U == 'U'){
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
    else if(line.find("wire")==0){
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
                Variable wire;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)// check to skip rest of whole words
                        i = j;
                    wire.name += line[j];
                    j++;
                }
                if(U == 'U'){
                    wire.type = "u";
                }
                else{
                    wire.type = "s";
                }
                wire.bits = BW;
                wires.push_back(wire);
            }
        }
    }
    else if(line.find("register")==0){
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
                Variable reg;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)// check to skip rest of whole words
                        i = j;
                    reg.name += line[j];
                    j++;
                }
                if(U == 'U'){
                    reg.type = "u";
                }
                else{
                    reg.type = "s";
                }
                reg.bits = BW;
                registers.push_back(reg);
            }
        }
    }
    else if(line.find("=") != std::string::npos){
        //vector<string> op;
        //string word = "";
        // for(int i = 0; line[i] != '\0'; i++){
        //     if(line[i+1] == '\0' && line[i] != ' '){
        //         word += line[i];
        //         op.push_back(word);
        //         word = "";
        //     }
        //     else if(line[i] != ' '){
        //         word += line[i];
        //     }
        //     else{
        //         if(word != "") op.push_back(word);
        //         word = "";
        //     }
        // }
        if(line.find_first_of("//") != std::string::npos){
            line = line.substr(0,line.find_first_of("//")-1);
        }
        operations.push_back(line);
    }
}

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
void writeHeader(ofstream &file){
    cout << "`timescale 1ns / 1ps \n" << endl;
    file << "`timescale 1ns / 1ps \n" << endl;
}

// write module
void writeModule(string name, vector<Variable> in, vector<Variable> out, ofstream &file){
    cout << "module " << name << "(clk, rst, ";
    file << "module " << name << "(clk, rst, ";
    std::vector<Variable>::iterator it;
    for ( it = in.begin() ; it != in.end(); ++it){
        cout << it->name << ", ";
        file << it->name << ", ";
    }
    for (it = out.begin() ; it != out.end(); ++it){
        if(it == out.end()-1){
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
    cout << "input clk, rst;" << endl;
    file << "input clk, rst;" << endl;
    
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "input ";
        file << "input ";
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
        cout << "output ";
        file << "output ";
        if(it->type == "s"){
            cout << "signed ";
            file << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}

// write wires
void writeWires(vector<Variable> var, ofstream &file){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "wire ";
        file << "wire ";
        if(it->type == "s"){
            cout << "signed ";
            file << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}

// write registers
void writeRegisters(vector<Variable> var, ofstream &file){
    for (std::vector<Variable>::iterator it = var.begin() ; it != var.end(); ++it){
        cout << "register ";
        file << "register ";
        if(it->type == "s"){
            cout << "signed ";
            file << "signed ";
        }
        cout << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
        file << "[" << it->bits - 1 << ":0] " << it->name << ";" << endl;
    }
}
// write operations
void writeOperations(vector<Operation> ops, ofstream &file){
    cout << endl;
    file << endl;
    unsigned int i = 0;
    for (std::vector<Operation>::iterator it = ops.begin() ; it != ops.end(); ++it){
        // comp(a,b,gt,lt,eq)
        cout << it->name << " #(.DATAWIDTH(" << it->bits << ")) " << it->name << i << "(";
        file << it->name << " #(.DATAWIDTH(" << it->bits << ")) " << it->name << i << "(";

        cout << it->var1.name << ", ";
        file << it->var1.name << ", ";
        if(it->name == "REG" || it->name == "SREG"){
            cout << "clk, rst, ";
            file << "clk, rst, ";
        }
        if(it->var2.name != ""){
            cout << it->var2.name << ", ";
            file << it->var2.name << ", ";
        }
        if(it->var3.name != ""){
            cout << it->var3.name << ", ";
        file << it->var3.name << ", ";
        }
        if(it->name == "COMP" || it->name == "SCOMP"){
            if(it->op1 == ">"){
                cout << it->result.name << ", 0" << ", 0" << ");" << endl;
                file << it->result.name << ", 0" << ", 0" << ");" << endl;
            }
            else if(it->op1 == "<"){
                cout << "0, " << it->result.name << ", 0" << ");" << endl;
                file << "0, " << it->result.name << ", 0" << ");" << endl;
            }
            else if(it->op1 == "=="){
                cout << "0, " << "0, " << it->result.name << ");" << endl;
                file << "0, " << "0, " << it->result.name << ");" << endl;
            }
        }
        else{
            cout << it->result.name << ");" << endl;
            file << it->result.name << ");" << endl;
        }
        ++i;
    }
    cout << endl << "endmodule" << endl;
    file << endl << "endmodule" << endl;
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

Operation SignExtend(Operation o1){
    // Sign Extend (signed and unsigned)
    // var1
    if(o1.var1.type == "s" && o1.var1.bits < o1.bits){
        // calculate bit difference
        int diff = o1.bits - o1.var1.bits;
        // $signed({24'hFFFFFF, temp2[7:0]})
        string nameSE = "$signed({" + to_string(diff) + "'b1, " + o1.var1.name + "[" + to_string(o1.var1.bits-1) + ":0]})";
        o1.var1.name = nameSE;
    }
    else if(o1.var1.type == "u" && o1.var1.bits < o1.bits){
        // calculate bit difference
        int diff = o1.bits - o1.var1.bits;
        // $signed({24'hFFFFFF, temp2[7:0]})
        string nameSE = "{" + to_string(diff) + "'b0, " + o1.var1.name + "[" + to_string(o1.var1.bits-1) + ":0]})";
        o1.var1.name = nameSE;
    }
    // var2
    if(o1.var2.type == "s" && o1.var2.bits < o1.bits){
        // calculate bit difference
        int diff = o1.bits - o1.var1.bits;
        // $signed({24'hFFFFFF, temp2[7:0]})
        string nameSE = "$signed({" + to_string(diff) + "'b1, " + o1.var2.name + "[" + to_string(o1.var2.bits-1) + ":0]})";
        o1.var2.name = nameSE;
    }
    else if(o1.var2.type == "u" && o1.var2.bits < o1.bits){
        // calculate bit difference
        int diff = o1.bits - o1.var2.bits;
        // $signed({24'hFFFFFF, temp2[7:0]})
        string nameSE = "{" + to_string(diff) + "'b0, " + o1.var2.name + "[" + to_string(o1.var2.bits-1) + ":0]})";
        o1.var2.name = nameSE;
    }
    // var3
    if(o1.var3.type == "s" && o1.var3.bits < o1.bits){
        // calculate bit difference
        int diff = o1.bits - o1.var3.bits;
        // $signed({24'hFFFFFF, temp2[7:0]})
        string nameSE = "$signed({" + to_string(diff) + "'b1, " + o1.var3.name + "[" + to_string(o1.var3.bits-1) + ":0]})";
        o1.var3.name = nameSE;
    }
    else if(o1.var3.type == "u" && o1.var3.bits < o1.bits){
        // calculate bit difference
        int diff = o1.bits - o1.var3.bits;
        // $signed({24'hFFFFFF, temp2[7:0]})
        string nameSE = "{" + to_string(diff) + "'b0, " + o1.var3.name + "[" + to_string(o1.var3.bits-1) + ":0]})";
        o1.var3.name = nameSE;
    }
    return o1;
}

// if input too large, take the lsb
Operation LSB(Operation o1){
    if(o1.var1.bits > o1.bits){

        string nameLSB = o1.var1.name + "[" + to_string(o1.bits-1) + ":0]";
        o1.var1.name = nameLSB;
    }
    if(o1.var2.bits > o1.bits){

        string nameLSB = o1.var2.name + "[" + to_string(o1.bits-1) + ":0]";
        o1.var2.name = nameLSB;
    }
    if(o1.var3.bits > o1.bits){
        string nameLSB = o1.var3.name + "[" + to_string(o1.bits-1) + ":0]";
        o1.var3.name = nameLSB;
    }
    return o1;
}


// process the string operations and return the Operation vector
vector<Operation> processOperation(vector<string> operations, vector<Variable> inputs, vector<Variable> outputs, vector<Variable> wires, vector<Variable> registers, bool* validCircuit){
    vector<Operation> temp;
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
           
           // create new operation with parsed operators
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
               // if it doesn't match any of these, it's not valid
               cout << "invalid operator: unexpected operator type, circuit invalidated" << endl;
               *validCircuit = false;
           }

           // validate 'result'
           Variable r1 = searchVariables(inputs, outputs, wires, registers, result);
           if(r1.bits == 0){
               //didn't find it if bits are 0 (default)
               cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
               *validCircuit = false;
           }
           else{
               o1.result = r1;
           }
           
           // validate the 'equals' operator
           if(equals != "="){
               // must be equal operator, lose the circuit
               cout << "invalid operator: expected '='. Circuit invalidated!" << endl;
               *validCircuit = false;
           }

           // validate 'var1'
           Variable v1 = searchVariables(inputs, outputs, wires, registers, var1);
           if(v1.bits == 0){
               //didn't find it if bits are 0 (default)
               cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
               *validCircuit = false;
           }
           else{
               // if this is a MUX, the order of variables needs to be fixed
               if(o1.name == "MUX2x1"){
                   o1.var3 = v1;
               }
               else{
                   o1.var1 = v1;
               }

           }
           
           // validate 'var3'
           Variable v2 = searchVariables(inputs, outputs, wires, registers, var2);
           if(v2.bits == 0 && o1.name != "INC" && o1.name != "REG" && o1.name != "DEC"){
               //didn't find it if bits are 0 (default)
               cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
               *validCircuit = false;
           }
           else{
               // if this is a MUX, the order of variables needs to be fixed
               if(o1.name == "MUX2x1"){
                   o1.var1 = v2;
               }
               else{
                   o1.var2 = v2;
               }
           }
           
           Variable v3 = searchVariables(inputs, outputs, wires, registers, var3);
           if(v3.bits != 0 && o1.name != "MUX2x1"){
               //didn't find it if bits are 0 (default)
               cout << "invalid operand: result variable not found. Circuit invalidated!" << endl;
               *validCircuit = false;
           }
           else{
               // if this is a MUX, the order of variables needs to be fixed
               if(o1.name == "MUX2x1"){
                   o1.var2 = v3;
               }
               else{
                   o1.var3 = v3;
               }
           }
           
           // operation bits for comparators depends on the largest input
           if(o1.name == "COMP"){
               o1.bits = maxInputBits(o1);
           }
           // otherwise operation bits depends upon result bits
           else{
               o1.bits = o1.result.bits;
           }

           // Sign Extend (signed and unsigned)
           o1 = SignExtend(o1);
           
           // if input to large, take the lsb
           o1 = LSB(o1);
           
           // if any input is signed, the operation is signed
           if(o1.result.type == "s" || o1.var1.type == "s" || o1.var2.type == "s" || o1.var3.type == "s"){
               o1.type = "s";
               string temp = "S";
               o1.name = temp + o1.name;
           }
           else{
               o1.type = "u";
           }

           // push the validated operation into ops vector
           temp.push_back(o1);
       }
    return temp;
}



int main(int argc, const char * argv[]) {
    // validate command line args
    if (argc != 3) {
        // show usage
        cerr << "Usage: " << argv[1] << " netlistFile" << endl << argv[2] << " verilogFile" << endl;

        return 1;
    }

    // begin Miranda, Connor implementation
    vector<Variable> inputs;
    vector<Variable> outputs;
    vector<Variable> wires;
    vector<Variable> registers;
    vector<string> operations;
//    // commented out to allow command line args
//    string filesDebug[16] = {
//        "474a_circuit1.txt"
//    };
    vector<string> parsedFile = ReadFile(argv[1]);
    
    for(vector<string>::iterator i = parsedFile.begin(); i != parsedFile.end(); ++i){
        ParseLine(inputs, outputs, wires, registers, operations, *i);
    }
    
    // valid circuit boolean, corresponding pointer
    bool validCircuit = true;
    bool* validPointer;
    validPointer = &validCircuit;
    
    // PrintOperands(inputs, outputs, wires, registers, operations);
    

//    // inputs
//    Variable in0("a", "u", 16);
//    Variable in1("b", "s", 8);
//    Variable in2("c", "u", 8);
//    vector<Variable> inputs = {in0, in1, in2};
//
//    // outputs
//    Variable out0("z", "s", 8);
//    Variable out1("x", "s", 16);
//    vector<Variable> outputs = {out0, out1};
//
//    // wires
//    Variable wi0("d", "s", 8);
//    Variable wi1("e", "u", 8);
//    Variable wi2("f", "s", 8);
//    Variable wi3("g", "u", 16);
//    Variable wi4("xwire", "s", 16);
//    vector<Variable> wires = {wi0, wi1, wi2, wi3, wi4};
//
//    // registers
//    Variable reg0("greg", "s", 64);
//    Variable reg1("hreg", "s", 64);
//    vector<Variable> registers = {reg0, reg1};
//
//    // operations directly from file, stored as strings in vector
//    vector<string> operations = {"d = a", "d = a ? b : c", "f = a + g", "d = a >> f", "d = a < f", "d = a == f"};
//    // processed operations done by yours truly
    
    // validate operations
    vector<Operation> ops = processOperation(operations, inputs, outputs, wires, registers, validPointer);


    // one last check before printing the circuit
    if(!validCircuit){
        cout << "Circuit marked invalid previously."<< endl;
        return -1;
    }

    // create the file
    // TODO: get actual filename from input file and retitle
    string verilogFile = argv[2];
    ofstream myfile (verilogFile + ".v");
    if (myfile.is_open())
    {
        // write the verilog to file (and cout)
        writeHeader(myfile);
        writeModule(verilogFile, inputs, outputs, myfile);
        writeInputs(inputs, myfile);
        writeOutputs(outputs, myfile);
        writeWires(wires, myfile);
        writeRegisters(registers, myfile);
        writeOperations(ops, myfile);
        myfile.close();
    }
    else cout << "Unable to open file";
    

    return 0;
}
