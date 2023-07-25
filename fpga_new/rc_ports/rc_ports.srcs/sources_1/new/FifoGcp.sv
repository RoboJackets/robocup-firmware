`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: William Stuckey
// 
// Create Date: 02/06/2020 09:55:39 AM
// Design Name: FIFO Gray Code Pointer 
// Module Name: FifoGcp
// Project Name: RoboCup
// Target Devices: Artix 7
// Tool Versions: 2019.2
// Description: 
// 
// Dependencies: none
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module FifoGcp #(
        parameter SUPPORTED_DEPTH = 0,
        localparam BIN_ADDR_WIDTH = $clog2(SUPPORTED_DEPTH),
        localparam BIN_ADDR_MSB = BIN_ADDR_WIDTH - 1,
        localparam GC_PTR_WIDTH = BIN_ADDR_WIDTH + 1,
        localparam GC_PTR_MSB = GC_PTR_WIDTH - 1
    )(
        input clk,
        input rst_n,
        input inc,
        
        output logic [BIN_ADDR_MSB:0] bin_ptr,
        output logic [GC_PTR_MSB:0] gc_ptr
    );
    
    generate
        if (SUPPORTED_DEPTH <= 0)
        begin: DEPTH_NOT_SET_ELABORATION_ERROR
            $fatal(1, "Fatal elaboration error. Invalid parameter value %b. SUPPORTED_DEPTH must be > 0.", SUPPORTED_DEPTH);
        end
    endgenerate
    
    generate
        if ((SUPPORTED_DEPTH & (SUPPORTED_DEPTH - 1)) != 0)
        begin: DEPTH_NOT_POW2_ELABORATION_ERROR
            $fatal(1, "Fatal elaboration error. Invalid parameter value %b. SUPPORTED_DEPTH must be a power of 2.", SUPPORTED_DEPTH);
        end
    endgenerate
    
    
    
endmodule: FifoGcp
