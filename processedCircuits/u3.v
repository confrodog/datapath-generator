`timescale 1ns / 1ps 

module u3(clk, rst, a, b, c);

input clk, rst;
input [31:0] a;
input [15:0] b;
output [7:0] c;
wire [7:0] cwire;

ADD #(.DATAWIDTH(8)) ADD0(a[23:0], b[7:0], cwire);
REG #(.DATAWIDTH(8)) REG1(cwire, clk, rst, c);

endmodule
