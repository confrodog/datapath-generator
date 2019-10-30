#include <iostream>
#include <vector>
#include <string>
#include <fstream>

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

string validOperations[11] = { "+", "-", "*", ">", "<", "==", "?", ">>", "<<", "/", "%"};

void PrintOperands(vector<variable> &inputs, vector<variable> &outputs, vector<variable> &wires){
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

void ParseLine(vector<variable>& inputs, vector<variable>& outputs,vector<variable>& wires, string line){
    //parse into inputs, outputs, wires and operations

    if(line.find("input")==0){
        //input Int8 a, b, c
        int BW = -1;
        BW = atoi(&line[line.find("Int")+3]);
        
        for(int i = line.find(' ',line.find("Int")); line[i] != '\0'; i++){
            if(line[i] != ' ' && line[i]!=','){
                variable input;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' '){
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
        
        for(int i = line.find(' ',line.find("Int")); line[i] != '\0'; i++){
            if(line[i] != ' ' && line[i]!=','){
                variable output;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' '){
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
        
        for(int i = line.find(' ',line.find("Int")); line[i] != '\0'; i++){
            if(line[i] != ' ' && line[i]!=','){
                variable wire;
                int j = i;
                while(line[j] != ',' &&  line[j] != '\0' && line[j]!= ' '){
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
    //else if(line.find("register")==0)
    else{
        //operand time
        //read one char at a time
        operation newOp;
        for(int i = 0; line[i] != '\0'; i++){
            if(line[i] != ' '){
                switch(i){
                    case 0:
                        int j = 0;
                        variable newOutput;
                        while(line[j]!=' '){
                            newOutput.name += line[j];
                            j++;
                        }
                        newOutput.bitwidth = CheckValidOutput(outputs, newOutput.name);
                        if(newOutput.bitwidth != -1){
                            newOp.result = newOutput;
                        }
                            
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    string filesDebug[16] = {
        "./assignment_2_circuits/474a_circuit2.txt"
    };

    vector<variable> inputs;
    vector<variable> outputs;
    vector<variable> wires;
    vector<variable> registers;
    vector<operation> flow;

    vector<string> parsedFile = ReadFile(filesDebug[0]);
    for(vector<string>::iterator i = parsedFile.begin(); i != parsedFile.end(); ++i){
        ParseLine(inputs, outputs, wires, *i);
    }

    PrintOperands(inputs, outputs, wires);
}