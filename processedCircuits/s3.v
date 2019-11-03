`timescale 1ns / 1ps 

module s3(clk, rst, a, b, c, d, e, f, g, h, sa, avg);

input clk, rst;
input signed [15:0] a;
input signed [15:0] b;
input signed [15:0] c;
input signed [15:0] d;
input signed [15:0] e;
input signed [15:0] f;
input signed [15:0] g;
input signed [15:0] h;
input signed [7:0] sa;
output signed [15:0] avg;
wire signed [31:0] l00;
wire signed [31:0] l01;
wire signed [31:0] l02;
wire signed [31:0] l03;
wire signed [31:0] l10;
wire signed [31:0] l11;
wire signed [31:0] l2;
wire signed [31:0] l2div2;
wire signed [31:0] l2div4;
wire signed [31:0] l2div8;

SADD #(.DATAWIDTH(32)) SADD0($signed({16'b1, a[15:0]}), $signed({16'b1, b[15:0]}), l00);
SADD #(.DATAWIDTH(32)) SADD1($signed({16'b1, c[15:0]}), $signed({16'b1, d[15:0]}), l01);
SADD #(.DATAWIDTH(32)) SADD2($signed({16'b1, e[15:0]}), $signed({16'b1, f[15:0]}), l02);
SADD #(.DATAWIDTH(32)) SADD3($signed({16'b1, g[15:0]}), $signed({16'b1, h[15:0]}), l03);
SADD #(.DATAWIDTH(32)) SADD4(l00, l01, l10);
SADD #(.DATAWIDTH(32)) SADD5(l02, l03, l11);
SADD #(.DATAWIDTH(32)) SADD6(l10, l11, l2);
SSHR #(.DATAWIDTH(32)) SSHR7(l2, $signed({24'b1, sa[7:0]}), l2div2);
SSHR #(.DATAWIDTH(32)) SSHR8(l2div2, $signed({24'b1, sa[7:0]}), l2div4);
SSHR #(.DATAWIDTH(32)) SSHR9(l2div4, $signed({24'b1, sa[7:0]}), l2div8);
SREG #(.DATAWIDTH(16)) SREG10(l2div8[15:0], clk, rst, avg);

endmodule
