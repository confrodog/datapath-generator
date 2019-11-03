`timescale 1ns / 1ps 

module s4(clk, rst, a, b, c, z, x);

input clk, rst;
input signed [63:0] a;
input signed [63:0] b;
input signed [63:0] c;
output signed [31:0] z;
output signed [31:0] x;
wire signed [63:0] d;
wire signed [63:0] e;
wire signed [63:0] f;
wire signed [63:0] g;
wire signed [63:0] h;
wire dLTe;
wire dEQe;
wire signed [63:0] xrin;
wire signed [63:0] zrin;
register signed [63:0] greg;
register signed [63:0] hreg;

SADD #(.DATAWIDTH(64)) SADD0(a, b, d);
SADD #(.DATAWIDTH(64)) SADD1(a, c, e);
SSUB #(.DATAWIDTH(64)) SSUB2(a, b, f);
SCOMP #(.DATAWIDTH(64)) SCOMP3(d, e, 0, 0, dEQe);
SCOMP #(.DATAWIDTH(64)) SCOMP4(d, e, 0, dLTe, 0);
SMUX2x1 #(.DATAWIDTH(64)) SMUX2x15(d, e, {63'b0, dLTe[0:0]}), g);
SMUX2x1 #(.DATAWIDTH(64)) SMUX2x16(g, f, {63'b0, dEQe[0:0]}), h);
SREG #(.DATAWIDTH(64)) SREG7(g, clk, rst, greg);
SREG #(.DATAWIDTH(64)) SREG8(h, clk, rst, hreg);
SSHL #(.DATAWIDTH(64)) SSHL9(hreg, {63'b0, dLTe[0:0]}), xrin);
SSHR #(.DATAWIDTH(64)) SSHR10(greg, {63'b0, dEQe[0:0]}), zrin);
SREG #(.DATAWIDTH(32)) SREG11(xrin[31:0], clk, rst, x);
SREG #(.DATAWIDTH(32)) SREG12(zrin[31:0], clk, rst, z);

endmodule
