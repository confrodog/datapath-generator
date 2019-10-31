#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

struct variable {
    int bitwidth;
    string name;
};

struct operation {
    variable result;
    vector<variable> operands;
    char operation;
};

vector<string> validOperations = { "+", "-", "*", ">", "<", "==", "?", ">>", "<<", "/", "%"};
void PrintOperands(vector<variable> &inputs, vector<variable> &outputs, vector<variable> &wires, vector<variable> &registers, vector<string>& operations){
    cout << "Inputs:\n";
    for(vector<variable>::iterator i = inputs.begin(); i != inputs.end(); i++){
        cout << distance(inputs.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bitwidth << "\n";
    }
    cout << "\nOutputs:\n";
    for(vector<variable>::iterator i = outputs.begin(); i != outputs.end(); i++){
        cout << distance(outputs.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bitwidth << "\n";
    }
    cout << "\nWires:\n";
    for(vector<variable>::iterator i = wires.begin(); i != wires.end(); i++){
        cout << distance(wires.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bitwidth << "\n";
    }
    cout << "\nRegisters:\n";
    for(vector<variable>::iterator i = registers.begin(); i != registers.end(); i++){
        cout << distance(registers.begin(), i)+1 << ". "<<  (*i).name << " has bitwidth " << (*i).bitwidth << "\n";
    }
    cout << "\nOperations:\n";
    for(vector<string>::iterator i = operations.begin(); i != operations.end(); i++){
            cout << (*i) << "\n";
    }
}

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

bool CheckValidOutput(vector<variable>& outputs, string newOutput){
    for(vector<variable>::iterator it = outputs.begin(); it != outputs.end(); it++){
        if((*it).name.compare(newOutput)==0){
            return (*it).bitwidth;
        }
    }
    return -1;
}

void ParseLine(vector<variable>& inputs, vector<variable>& outputs,vector<variable>& wires, vector<variable>& registers, vector<string>& operations, string line){
    //parse into inputs, outputs, wires and operations

    if(line.find("input")==0){
        //input Int8 a, b, c
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        
        for(int i = line.find(' ',line.find("Int")); i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] = '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                variable input;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)
                        i = j;
                    input.name += line[j];
                    j++;
                }
                input.bitwidth = BW;
                cout << "adding " << input.name << " to inputs\n";
                inputs.push_back(input);
            }
        }
    }
    else if(line.find("output")==0){
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        
        for(int i = line.find(' ',line.find("Int"));  i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] = '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                variable output;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)
                        i = j;
                    output.name += line[j];
                    j++;
                }
                output.bitwidth = BW;
                outputs.push_back(output);
            }
        }
    }
    else if(line.find("wire")==0){
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        
        for(int i = line.find(' ',line.find("Int"));  i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] = '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                variable wire;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)// check to skip rest of whole words
                        i = j;
                    wire.name += line[j];
                    j++;
                }
                wire.bitwidth = BW;
                wires.push_back(wire);
            }
        }
    }
    else if(line.find("register")==0){
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        
        for(int i = line.find(' ',line.find("Int"));  i< line.length(); i++){
            if(line[i] == '/'){
                if(line[i+1] = '/'){
                    i = line.length();
                }
            }
            else if(line[i] != ' ' && line[i]!=',' && line[i] != '\t'){
                variable reg;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' ' && line[j] != '\t'){
                    if((j-i)>0)// check to skip rest of whole words
                        i = j;
                    reg.name += line[j];
                    j++;
                }
                reg.bitwidth = BW;
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

// bool validations(vector<variable>& inputs, vector<variable>& outputs,vector<variable>& wires, vector<string>& operations){
//     bool broken = false;
//     for(vector<vector<string>>::iterator i = operations.begin(); i != operations.end(); ++i){
//         int len = (*i).size();
//         if(len > 0){
//             string result = (*i)[0];
//             string eq = (*i)[1];
//             string oper1 = (*i)[2];
//             string op = "";
//             string oper2 = "";
//             string mux = ":";
//             string oper3 = "";
//             bool correctOp = false;
//             bool outputFound = false;
//             bool input1Found = false;
//             bool input2Found = true;
//             bool input3Found = true;
//             bool wireFound = true;
//             if(len > 3){
//                 op = (*i)[3];
//                 oper2 = (*i)[4];
//                 input2Found = false;
//                 if(len > 5){
//                     mux = (*i)[5];
//                     oper3 = (*i)[6];
//                     input3Found = false;
//                 }
//             }
            
//             if(op != ""){
//                 if(find(begin(validOperations), end(validOperations), op) != end(validOperations)){
//                     correctOp = (mux == ":") ? true : false;
//                 }
//             }

//             //ouput
//             for(vector<variable>::iterator i = outputs.begin(); i != outputs.end(); i++){
//                 if(result == (*i).name){
//                     outputFound = true;
//                 }
//             }
//             //input
//             for(vector<variable>::iterator i = inputs.begin(); i != inputs.end(); i++){
//                 if(!input1Found) input1Found = (oper1 == (*i).name) ? true : false;
//                 if(!input2Found) { if (len > 3) input2Found = (oper2 == (*i).name) ? true : false; }
//                 if(!input3Found) { if (len > 5) input3Found = (oper3 == (*i).name) ? true : false; }
//             }

//             //wires
//             if(!outputFound || !input1Found || !input2Found || !input3Found){
//                 wireFound = false;
//                 for(vector<variable>::iterator i = wires.begin(); i != wires.end(); i++){
//                     if(!outputFound){
//                         wireFound = (result == (*i).name)? true : false;
//                         if(wireFound)
//                             outputFound = true;
//                     }
//                     if(!input1Found){
//                         wireFound = (oper1 == (*i).name)? true : false;
//                         if(wireFound)
//                             input1Found = true;
//                     }
//                     if(!input2Found){
//                         wireFound = (oper2 == (*i).name)? true : false;
//                         if(wireFound)
//                             input2Found = true;
//                     }
//                     if(!input3Found){
//                         wireFound = (oper3 == (*i).name)? true : false;
//                         if(wireFound)
//                             input3Found = true;
//                     }
//                 }
//             }

//             if(!correctOp || !outputFound || !input1Found || !input2Found || !input3Found || !wireFound){
//                 broken = true;
//             }
//         }
//     }

//     return broken;
// }

int main(int argc, char** argv)
{
    string filesDebug[16] = {
        "./assignment_2_circuits/error4.txt"
    };

    vector<variable> inputs;
    vector<variable> outputs;
    vector<variable> wires;
    vector<variable> registers;
    vector<string> operations;
    vector<operation> flow;

    vector<string> parsedFile = ReadFile(filesDebug[0]);
    for(vector<string>::iterator i = parsedFile.begin(); i != parsedFile.end(); ++i){
        ParseLine(inputs, outputs, wires, registers, operations, *i);
    }

    // bool isValid = validations(inputs, outputs, wires, operations);
    // if(isValid){
    //     cout << "INVALID";
    // }
    // else{
    //     cout << "VALID";
    // }
    //cout << "\n";

    PrintOperands(inputs, outputs, wires, registers, operations);
}