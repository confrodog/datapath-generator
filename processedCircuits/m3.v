`timescale 1ns / 1ps 

module m3(clk, rst, a, b, c);

input clk, rst;
input signed [31:0] a;
input [15:0] b;
output signed [7:0] c;
wire signed [7:0] cwire;

SADD #(.DATAWIDTH(8)) SADD0(a[23:0], b[7:0], cwire);
SREG #(.DATAWIDTH(8)) SREG1(cwire, clk, rst, c);

endmodule
