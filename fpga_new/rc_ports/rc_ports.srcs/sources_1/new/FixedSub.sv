`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/16/2020 02:36:20 PM
// Design Name: Fixed Point Subtract
// Module Name: FixedSub
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


module FixedSub #(
    DATA_WIDTH = 32,
    RADIX =15       // Not relevant for add
)(
    input [DATA_WIDTH-1:0] x,
    input [DATA_WIDTH-1:0] y,
    output [DATA_WIDTH-1:0] z,
    output logic overflow
);

FixedAdd #(
    .DATA_WIDTH(DATA_WIDTH),
    .RADIX(RADIX)
) dutAdd (
    .x(x),
    .y(-y),
    .z(z),
    .overflow(overflow)
);

endmodule
