`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/16/2020 02:36:20 PM
// Design Name: Fixed Point Add and Sub testbench
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


module FixedAddSub_tb #(
    DATA_WIDTH = 32,
    RADIX = 15
);

reg [DATA_WIDTH-1:0] x;
reg [DATA_WIDTH-1:0] y;
wire [DATA_WIDTH-1:0] zSub;
wire [DATA_WIDTH-1:0] zAdd;
wire overflowAdd;
wire overflowSub;

FixedAdd #(
    .DATA_WIDTH(DATA_WIDTH),
    .RADIX(RADIX)
) dutAdd (
    .x(x),
    .y(y),
    .z(zAdd),
    .overflow(overflowAdd)
);

FixedSub #(
    .DATA_WIDTH(DATA_WIDTH),
    .RADIX(RADIX)
) dutSub (
    .x(x),
    .y(y),
    .z(zSub),
    .overflow(overflowSub)
);

initial begin
    x = 10;
    y = 5;
    #1;
    x = 32'h0FFFFFFF;
    y = 32'hFFFFFFFF;
    #1;
    x = 32'h7FFFFFFF;
    y = 32'h7FFFFFFF;
    #1;
    x = -32'h7FFFFFFF;
    y = 32'h7FFFFFFF;
    #1 $finish;
end

endmodule
