`timescale 1ns / 1ps 

module HLSM (clk, rst, Start, Done, a, b, c, z, x);

	input Clk, Rst, Start;
	input signed [31:0] a;
	input signed [31:0] b;
	input signed [31:0] c;
	output reg Done;
	output reg signed [31:0] z;
	output reg signed [31:0] x;
	parameter S0 = 0, S1 = 1 ,S2 = 2 ,S3 = 3 ,S4 = 4 ,S5 = 5 ,S6 = 6;
	reg signed [31:0] d;
	reg signed [31:0] e;
	reg signed [31:0] f;
	reg signed [31:0] g;
	reg signed [31:0] h;
	reg [0:0] dLTe;
	reg [0:0] dEQe;
	reg [2:0] State;
	always @(posedge Clk) begin
		if(Rst) begin
			State <= S0;
			z <= 0;
			x <= 0;
			d <= 0;
			e <= 0;
			f <= 0;
			g <= 0;
			h <= 0;
			dLTe <= 0;
			dEQe <= 0;
		end
		else begin
			case(State)
				S0: begin
					if (Start) begin
						State <= S1;
					end
					else begin
						State <= S0;
					end
				end
				S1: begin
					d= a+b;
				end
				S2: begin
					e= a+c;
				end
				S3: begin
					dLTe= d<e;
				end
				S4: begin
					dEQe= d==e;
					f= a-b;
					g= d?e:dLTe;
				end
				S5: begin
					h= g?f:dEQe;
					x= g<<dLTe;
				end
				S6: begin
					z= h>>dEQe;
				end
				S7: begin
					State <= S0;
					Done <= 1;
				end
			endcase
		end
	end
endmodule
