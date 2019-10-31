# datapath-generator
really cool program to generate datapaths

This program takes high level pseudo code and converts it using C++ to verilog. This is done by first reading in the input .txt file and parsing relevant information into data structures. These data structures are then evaluated for completeness and thoroughly error checked. The .v template is formed and the variables and operations are filled in appropriately, constructing a verilog implementation of the high level pseudo code.

| Contributors              | Net Id          | Course | Task                                                               |
|---------------------------|-----------------|--------|--------------------------------------------------------------------|
| Amir Mohammad Asdagh Pour | asdaghpour      | 513    | Critical Path Design and Implementation                            |
| Alex F Reyes              | alexanderfreyes | 413    | Cmake List generation, Testing on ECE3 server                      |
| Connor Jusitin Musick     | cjmusick        | 413    | File input and information parsing, Testing in Vivado              |
| Miranda Hampton           | mirandahampton  | 413    | File input and information parsing, Error checking                 |
| Kris Rockowitz            | rockowitzks     | 513    | Verilog translation, Error checking, Output, Critical Path framing |

This is not an *efficient* way to do this, but it is *a* way.

# dpgenULTRA.cpp is the completed program (no Critical Path yet). It has been tested on the ece3 server with the precise installation instructions provided by Tosi.


# TODO: visually and programmatically test the .v files and make sure they are verilog ready.

# TODO: Clean up the code by programming standards. (push functions to header files)

# TODO: Critical path analysis and inclusion
