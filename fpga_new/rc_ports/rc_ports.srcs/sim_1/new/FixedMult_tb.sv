`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/16/2020 02:36:20 PM
// Design Name: Fixed Point Multiplier Testbench
// Module Name: FixedMult_tb
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


module FixedMult_tb #(
    DATA_WIDTH = 32,
    RADIX =15     
    );
    
reg [DATA_WIDTH-1:0] x;
reg [DATA_WIDTH-1:0] y;
wire [DATA_WIDTH-1:0] z;
wire overflow;


FixedMult #(
    .DATA_WIDTH(DATA_WIDTH),
    .RADIX(RADIX)
) dutMult (
    .x(x),
    .y(y),
    .z(z),
    .overflow(overflow)
);


initial begin
    x = 10;
    y = 5;
    #1; //Testing overflow:
    x = 32'h0FFFFFFF;
    y = 32'hFFFFFFFF;
    #1;
    x = 32'h7FFFFFFF;
    y = 32'h7FFFFFFF;
    #1;
    x = -32'h7FFFFFFF;
    y = 32'h7FFFFFFF;
    #1;
    x = 2;
    y = 32'h7FFFFFFF;
    #1;
    x = 2;
    y = 32'h40000000;
    #1;
    x = 2;
    y = -32'h40000000;
    #1;
    x = 2;
    y = -32'h3ffffffe;
    #1 $finish;
end 
    
endmodule
