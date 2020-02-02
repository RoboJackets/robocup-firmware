`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Will Stuckey
// 
// Create Date: 02/02/2020 12:08:15 AM
// Design Name: Flip-Flop Synchronizer
// Module Name: FfSync
// Project Name: RoboCup
// Target Devices: Artix 7 Family
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

`ifndef DEFAULT_SYNC_WIDTH
    `define DEFAULT_SYNC_WIDTH 1
`endif

`ifndef DEFAULT_SYNC_DEPTH
    `define DEFAULT_SYNC_DEPTH 2
`endif

module FfSync #(
        parameter SYNC_WIDTH=`DEFAULT_SYNC_WIDTH,
        parameter SYNC_DEPTH=`DEFAULT_SYNC_DEPTH
    )(
        input [SYNC_WIDTH-1:0] d_in, clk, rst_n,
        output [SYNC_WIDTH-1:0] d_out
    );
    
    // ffsync buf, use unpacked array for dyn gen, apply directive to prevent SRL inference
    (* srl_style = "register" *)
    reg [SYNC_WIDTH-1:0] sync_buf [SYNC_DEPTH-1:0];
    
    // initial sync stage and reset
    always_ff @(posedge clk or negedge rst_n) 
    begin : FIRST_SYNC_STAGE
        if (rst_n == 0) begin
            sync_buf[0] <= 0;
        end
        else begin
            sync_buf[0] <= d_in;
        end
    end
    
    // dynamically generate sync chain
    genvar ff_depth_index;
    for (ff_depth_index = 1; ff_depth_index < SYNC_DEPTH; ff_depth_index++)
    begin: SYNC_CHAIN_GEN
        if (SYNC_DEPTH < 2) 
        begin: E_INVAL_DEPTH
            $fatal(1, "Fatal elaboration error. Invalid parameter value %b. SYNC_DEPTH must be >= 2.", SYNC_DEPTH);
        end
        else begin
            always_ff @(posedge clk)
            begin: SYNC_CHAIN_ELEMENT
                sync_buf[ff_depth_index] <= sync_buf[ff_depth_index-1];
            end
        end
    end
    
    // output
    assign d_out = sync_buf[SYNC_DEPTH-1];
    
endmodule
