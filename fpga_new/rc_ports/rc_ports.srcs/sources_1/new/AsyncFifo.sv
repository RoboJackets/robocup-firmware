`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Will Stuckey
// 
// Create Date: 02/02/2020 07:48:54 PM
// Design Name: Asynchronous FIFO
// Module Name: AsyncFifo
// Project Name: RoboCup
// Target Devices: Artix 7
// Tool Versions: 2019.2
// Description: 
// 
// Dependencies: FfSync.sv
// 
// Revision: 0.01
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

`ifndef _ASYNC_FIFO_
`define _ASYNC_FIFO_

// used below to prevent the fifo buf from being inferenced into BRAM
// not sure if it causes issues with CDC for DP_BRAM
`define FORCE_NO_BRAM_INFERENCE

`ifndef DEFAULT_SYNC_DEPTH
    `define DEFAULT_SYNC_DEPTH 2
`endif

module AsyncFifo #(
        FIFO_DATA_WIDTH=1,
        localparam FIFO_DATA_WIDTH_MSB = (FIFO_DATA_WIDTH - 1),
        
        FIFO_DEPTH=16,
        localparam FIFO_DEPTH_I = (FIFO_DEPTH - 1),
        localparam ADDR_WIDTH = $clog2(FIFO_DEPTH),
        localparam ADDR_MSB = (ADDR_WIDTH - 1),
        
        SYNC_DEPTH=`DEFAULT_SYNC_DEPTH
    )(
        input clk_wr_domain,
        input rst_n_wr_domain,
        input wr_en,
        input [FIFO_DATA_WIDTH_MSB:0] d_in,
        output logic wr_full,
        output logic wr_err,
        
        input clk_rd_domain,
        input rst_n_rd_domain,
        input rd_en,
        output logic [FIFO_DATA_WIDTH_MSB:0] d_out,
        output logic rd_empty,
        output logic rd_err
    );
    
    // prevent DP_BRAM inferencing here for CDC
    // this define can be commented out at the top of the file
    // to permit inferencing
    `ifdef FORCE_NO_BRAM_INFERENCE
    (* ram_style = "registers" *)
    `endif
    reg [FIFO_DATA_WIDTH_MSB:0] fifo_mem [0:FIFO_DEPTH_I];
    
    /////////////////////////////////////////////////////////////////
    //  WRITE DOMAIN ACTION
    /////////////////////////////////////////////////////////////////
    
    reg [ADDR_MSB:0] wr_addr;
    
    always_ff @(posedge clk_wr_domain or negedge rst_n_wr_domain)
    begin: WRITE_BUFFER
        if (wr_en && !wr_full && !wr_err) 
        begin
            fifo_mem[wr_addr] <= d_in;
        end
    end
    
    always_ff @(posedge clk_wr_domain or negedge rst_n_wr_domain)
    begin: WRITE_FLAGS
        if (rst_n_wr_domain == 0) 
        begin: RESET_WRITE_FLAGS
            wr_err <= 0;
        end
        else 
        begin: CHECK_INVALID_WRITE
            if (wr_en && wr_full)
            begin: SET_INVALID_READ_FLAG
                wr_err <= 1'b1;
            end
        end
    end
    
    /////////////////////////////////////////////////////////////////
    //  WRITE DOMAIN CONTROL
    /////////////////////////////////////////////////////////////////
    
    /////////////////////////////////////////////////////////////////
    //  READ DOMAIN ACTION
    /////////////////////////////////////////////////////////////////
    
    reg [ADDR_MSB:0] rd_addr;
    
    always_comb
    begin: READ_BUFFER
        if (rd_en && !rd_empty && !rd_err) begin
            d_out = fifo_mem[rd_addr];
        end
        else begin
            d_out = 0;
        end
    end
    
    always_ff @(posedge clk_rd_domain or negedge rst_n_rd_domain)
    begin: READ_FLAGS
        if (rst_n_rd_domain == 0) 
        begin: RESET_READ_FLAGS
            rd_err <= 0;
        end
        else 
        begin: CHECK_INVALID_READ
            if (rd_en && rd_empty)
            begin: SET_INVALID_READ_FLAG
                rd_err <= 1'b1;
            end
        end
    end
    
    /////////////////////////////////////////////////////////////////
    //  READ DOMAIN CONTROL
    /////////////////////////////////////////////////////////////////
    
    /////////////////////////////////////////////////////////////////
    //  POINTER CLOCK DOMAIN SYNCHRONIZATION
    /////////////////////////////////////////////////////////////////
    
    FfSync #(
        .SYNC_WIDTH(FIFO_DATA_WIDTH),
        .SYNC_DEPTH(SYNC_DEPTH)
    ) rd_to_wd_sync (
        .clk(clk_wr_domain),
        .rst(rst_n_wr_domain),
        .d_in(),
        .d_out()
    );
    
    FfSync #(
        .SYNC_WIDTH(FIFO_DATA_WIDTH),
        .SYNC_DEPTH(SYNC_DEPTH)
    ) wd_to_rd_sync (
    
    );

endmodule

`endif
