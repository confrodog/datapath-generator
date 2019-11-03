`timescale 1ns / 1ps 

module u2(clk, rst, a, b, c);

input clk, rst;
input [7:0] a;
input [15:0] b;
output [31:0] c;
wire [31:0] cwire;

ADD #(.DATAWIDTH(32)) ADD0({24'b0, a[7:0]}), {16'b0, b[15:0]}), cwire);
REG #(.DATAWIDTH(32)) REG1(cwire, clk, rst, c);

endmodule
