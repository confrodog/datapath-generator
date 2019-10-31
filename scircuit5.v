`timescale 1ns / 1ps 

module scircuit5(clk, rst, a, b, c, d, zero, z);

input clk, rst;
input signed [63:0] a;
input signed [63:0] b;
input signed [63:0] c;
input signed [63:0] d;
input signed [63:0] zero;
output signed [63:0] z;
wire signed [63:0] e;
wire signed [63:0] f;
wire signed [63:0] g;
wire signed [63:0] zwire;
wire signed [0:0] gEQz;

SREG #(.DATAWIDTH(64)) SREG0(a, clk, rst, e);
SREG #(.DATAWIDTH(64)) SREG1(c, clk, rst, f);
SMOD #(.DATAWIDTH(64)) SMOD2(a, b, g);
SCOMP #(.DATAWIDTH(64)) SCOMP3(g, zero, 0, 0, gEQz);
SMUX2x1 #(.DATAWIDTH(64)) SMUX2x14(e, f, $signed({63'b1, gEQz[0:0]}), zwire);
SREG #(.DATAWIDTH(64)) SREG5(zwire, clk, rst, z);

endmodule
