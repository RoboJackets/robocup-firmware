`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/16/2020 02:36:20 PM
// Design Name: Fixed Point Add
// Module Name: FixedAdd
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


module FixedAdd #(
    DATA_WIDTH = 32,
    RADIX =15       // Not relevant for add
)(
    input [DATA_WIDTH-1:0] x,
    input [DATA_WIDTH-1:0] y,
    output [DATA_WIDTH-1:0] z,
    output overflow
);

wire [DATA_WIDTH-1:0] result;

assign overflow = result[DATA_WIDTH-1];
assign z = $signed(x) + $signed(y);

generate
    if (DATA_WIDTH < 1)
    begin: INVALID_DATA_WIDTH_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. DATA_BIT_WIDTH must be > 0.", DATA_WIDTH);
    end 
endgenerate

assign result = (x[DATA_WIDTH-1]==0 && y[DATA_WIDTH-1]==0)?
                    x[DATA_WIDTH-2:0] + y[DATA_WIDTH-2:0]:
                    (x[DATA_WIDTH-1]==1 && y[DATA_WIDTH-1]==1)?
                    (-x[DATA_WIDTH-2:0]) + (-y[DATA_WIDTH-2:0]):0;


endmodule
