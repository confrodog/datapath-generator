`timescale 1ns / 1ps 

module s6(clk, rst, a, b, c, zero, z);

input clk, rst;
input signed [63:0] a;
input signed [63:0] b;
input signed [63:0] c;
input signed [63:0] zero;
output signed [63:0] z;
wire signed [63:0] e;
wire signed [63:0] f;
wire signed [63:0] g;
wire signed [63:0] zwire;
wire gEQz;

SDEC #(.DATAWIDTH(64)) SDEC0(a, e);
SINC #(.DATAWIDTH(64)) SINC1(c, f);
SMOD #(.DATAWIDTH(64)) SMOD2(a, c, g);
SCOMP #(.DATAWIDTH(64)) SCOMP3(g, zero, 0, 0, gEQz);
SMUX2x1 #(.DATAWIDTH(64)) SMUX2x14(e, f, {63'b0, gEQz[0:0]}), zwire);
SREG #(.DATAWIDTH(64)) SREG5(zwire, clk, rst, z);

endmodule
