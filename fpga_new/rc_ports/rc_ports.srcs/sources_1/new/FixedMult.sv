`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/16/2020 02:36:20 PM
// Design Name: Fixed Point Multiplier
// Module Name: FixedMult
// Project Name: RoboCup
// Target Devices: Artix 7
// Tool Versions: 2019.2
// Description: 
// 
// Dependencies: 
// 
// Revision: 0.01
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module FixedMult #(
    DATA_WIDTH = 32,
    RADIX =15 
)(
    input [DATA_WIDTH-1:0] x,
    input [DATA_WIDTH-1:0] y,
    output [DATA_WIDTH-1:0] z,
    output overflow
);

wire [2*DATA_WIDTH-1:0] result;

generate
    if (DATA_WIDTH < 1)
    begin: INVALID_DATA_WIDTH_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. DATA_BIT_WIDTH must be > 0.", DATA_WIDTH);
    end 
endgenerate


assign overflow = (result >> (DATA_WIDTH-1) != 0);
assign result = ((x[DATA_WIDTH-1]==1)?$unsigned(-x):$unsigned(x))*((y[DATA_WIDTH-1]==1)?$unsigned(-y):$unsigned(y));

assign z = result[DATA_WIDTH-1:0]; 
        

endmodule
