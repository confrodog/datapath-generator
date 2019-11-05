# datapath-generator
really cool program to generate datapaths

This program takes high level pseudo code and converts it using C++ to verilog. This is done by first reading in the input .txt file and parsing relevant information into data structures. These data structures are then evaluated for completeness and thoroughly error checked. The .v template is formed and the variables and operations are filled in appropriately, constructing a verilog implementation of the high level pseudo code.

| Contributors              | Net Id          | Course | Task                                                                                      |
|---------------------------|-----------------|--------|-------------------------------------------------------------------------------------------|
| Amir Mohammad Asdagh Pour | asdaghpour      | 513    | Critical Path Design and Analysis                                                         |
| Alex F Reyes              | alexanderfreyes | 413    | Cmake List generation, Testing on ECE3 server, Critical Path Analysis                     |
| Connor Jusitin Musick     | cjmusick        | 413    | File input and information parsing, Testing in Vivado, Critical Path Analysis             |
| Miranda Hampton           | mirandahampton  | 413    | File input and information parsing, Error checking                                        |
| Kris Rockowitz            | rockowitzks     | 513    | Verilog translation, Error checking, Output Files, Critical Path Algorithm Implementation |

This is not an *efficient* way to do this, but it is *a* way.

# recently added:
dpgen_withCritPath.cpp : This implements a DFS to find critical paths, complete with cout. Will need to comment out printing of individual components and latencies before submitting.

processedCircuits: This contains all circuits processed by the new dpgen with critical path enabled. Please check these to ensure they make sense.

# critical paths (please verify):

**S1**
MUL 7.811000 SUB 5.569000 REG 3.061000 Total Latency = 16.441000

Critical Path : 16.441 ns

**S2**
ADD 7.270000 COMP 7.264000 MUX2x1 8.079000 MUX2x1 8.079000 SHR 8.819000 REG 3.602000 Total Latency = 43.113003

Critical Path : 43.113 ns

**S3**
ADD 7.270000 ADD 7.270000 ADD 7.270000 SHR 8.819000 SHR 8.819000 SHR 8.819000 REG 3.061000 Total Latency = 51.327999

Critical Path : 51.328 ns

**S4**
ADD 9.566000 COMP 8.416000 MUX2x1 8.766000 SHR 11.095000 REG 3.602000 Total Latency = 41.445004

Critical Path : 41.445 ns

**S5**
MOD 250.582993 COMP 8.416000 MUX2x1 8.766000 REG 3.966000 Total Latency = 271.730988

Critical Path : 271.731 ns

**S6**
MOD 250.582993 COMP 8.416000 MUX2x1 8.766000 REG 3.966000 Total Latency = 271.730988

Critical Path : 271.731 ns
