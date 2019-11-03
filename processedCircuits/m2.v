`timescale 1ns / 1ps 

module m2(clk, rst, a, b, c);

input clk, rst;
input signed [7:0] a;
input [15:0] b;
output signed [31:0] c;
wire signed [31:0] cwire;

SADD #(.DATAWIDTH(32)) SADD0($signed({24'b1, a[7:0]}), {16'b0, b[15:0]}), cwire);
SREG #(.DATAWIDTH(32)) SREG1(cwire, clk, rst, c);

endmodule
