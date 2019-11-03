`timescale 1ns / 1ps 

module u1(clk, rst, a, b, c);

input clk, rst;
input [7:0] a;
input [7:0] b;
output [7:0] c;
wire [7:0] cwire;

ADD #(.DATAWIDTH(8)) ADD0(a, b, cwire);
REG #(.DATAWIDTH(8)) REG1(cwire, clk, rst, c);

endmodule
