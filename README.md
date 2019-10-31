# datapath-generator
really cool program to generate datapaths

This program takes high level pseudo code and converts it using C++ to verilog. This is done by first reading in the input .txt file and parsing relevant information into data structures. These data structures are then evaluated for completeness and thoroughly error checked. The .v template is formed and the variables and operations are filled in appropriately, constructing a verilog implementation of the high level pseudo code.

# Contributors              Net ID            Course     Task
Amir Mohammad Asdagh Pour   asdaghpour        513        Critical Path Design and Implementation
Alex Reyes                  alexanderfreyes   413        Cmake List generation, Testing on ECE3 server
Connor Jusitin Musick       cjmusick          413        File input and information parsing, Testing
Miranda Hampton             mirandahampton    413        File input and information parsing, Error checking
Kris Rockowitz              rockowitzks       513        Verilog translation, Error checking, Output, Critical Path framing

This is not an efficient way to do this, but it is * a * way.
