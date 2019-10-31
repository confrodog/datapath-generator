`timescale 1ns / 1ps 

module scircuit2(clk, rst, a, b, c, z, x);

input clk, rst;
input signed [31:0] a;
input signed [31:0] b;
input signed [31:0] c;
output signed [31:0] z;
output signed [31:0] x;
wire signed [31:0] d;
wire signed [31:0] e;
wire signed [31:0] f;
wire signed [31:0] g;
wire signed [31:0] h;
wire signed [0:0] dLTe;
wire signed [0:0] dEQe;
wire signed [31:0] zwire;
wire signed [31:0] xwire;

SADD #(.DATAWIDTH(32)) SADD0(a, b, d);
SADD #(.DATAWIDTH(32)) SADD1(a, c, e);
SSUB #(.DATAWIDTH(32)) SSUB2(a, b, f);
SCOMP #(.DATAWIDTH(32)) SCOMP3(d, e, 0, 0, dEQe);
SCOMP #(.DATAWIDTH(32)) SCOMP4(d, e, 0, dLTe, 0);
SMUX2x1 #(.DATAWIDTH(32)) SMUX2x15(d, e, $signed({31'b1, dLTe[0:0]}), g);
SMUX2x1 #(.DATAWIDTH(32)) SMUX2x16(g, f, $signed({31'b1, dEQe[0:0]}), h);
SSHL #(.DATAWIDTH(32)) SSHL7(g, $signed({0'b1, dLTe[0:0]}), xwire);
SSHR #(.DATAWIDTH(32)) SSHR8(h, $signed({0'b1, dEQe[0:0]}), zwire);
SREG #(.DATAWIDTH(32)) SREG9(xwire, clk, rst, x);
SREG #(.DATAWIDTH(32)) SREG10(zwire, clk, rst, z);

endmodule
