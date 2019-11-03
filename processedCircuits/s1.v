`timescale 1ns / 1ps 

module s1(clk, rst, a, b, c, z, x);

input clk, rst;
input signed [7:0] a;
input signed [7:0] b;
input signed [7:0] c;
output signed [7:0] z;
output signed [15:0] x;
wire signed [7:0] d;
wire signed [7:0] e;
wire signed [15:0] f;
wire signed [15:0] g;
wire signed [15:0] xwire;

SADD #(.DATAWIDTH(8)) SADD0(a, b, d);
SADD #(.DATAWIDTH(8)) SADD1(a, c, e);
SCOMP #(.DATAWIDTH(8)) SCOMP2(d, e, g, 0, 0);
SMUX2x1 #(.DATAWIDTH(8)) SMUX2x13(d, e, g[7:0], z);
SMUL #(.DATAWIDTH(16)) SMUL4($signed({8'b1, a[7:0]}), $signed({8'b1, c[7:0]}), f);
SSUB #(.DATAWIDTH(16)) SSUB5(f, $signed({8'b1, d[7:0]}), xwire);
SREG #(.DATAWIDTH(16)) SREG6(xwire, clk, rst, x);

endmodule
