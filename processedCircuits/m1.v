`timescale 1ns / 1ps 

module m1(clk, rst, a, b, c);

input clk, rst;
input signed [7:0] a;
input [7:0] b;
output signed [7:0] c;
wire signed [7:0] cwire;

SADD #(.DATAWIDTH(8)) SADD0(a, b, cwire);
SREG #(.DATAWIDTH(8)) SREG1(cwire, clk, rst, c);

endmodule
