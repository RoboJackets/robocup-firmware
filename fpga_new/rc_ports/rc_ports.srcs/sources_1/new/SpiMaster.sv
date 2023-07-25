`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/02/2020 12:08:15 AM
// Design Name: SPI Master
// Module Name: SpiMaster
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


module SpiMaster #(
    DATA_BIT_WIDTH = 8,
    
    localparam COUNTER_WIDTH = $clog2(DATA_BIT_WIDTH),
    localparam RESET = 0,
    localparam IDLE = 1,
    localparam LOAD = 2,
    localparam TRANSACT = 3,
    localparam UNLOAD =4
)(
    input clk,
    input rst,
    input miso,
    input start,
    input [DATA_BIT_WIDTH-1:0] dataIn,
    
    output mosi,
    output logic cs,
    output valid,
    output busy,
    output logic [DATA_BIT_WIDTH-1:0] dataOut
);



reg [COUNTER_WIDTH-1:0] count = 0;
reg [DATA_BIT_WIDTH-1:0] mosi_d;
reg [DATA_BIT_WIDTH-1:0] miso_d;
reg [2:0] state = 0;

assign busy = (state == TRANSACT);
assign mosi = cs? 1'bz: mosi_d[DATA_BIT_WIDTH-1];
assign valid = (state == UNLOAD);
assign cs = (state == IDLE || state == RESET);

generate
    if (DATA_BIT_WIDTH < 1)
    begin: INVALID_DATA_BIT_WIDTH_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. DATA_BIT_WIDTH must be > 0.", DATA_BIT_WIDTH);
    end
endgenerate

always_ff @(posedge clk) begin
    if (rst == 0) 
        state = RESET;
    else
        case (state)
            RESET:
            begin
                state = IDLE;
            end
            IDLE:
            begin
                if (start)
                    state = LOAD;
            end
            LOAD:
            begin   
                state = TRANSACT;
            end
            TRANSACT:
            begin
                if (count == 0)
                    state = UNLOAD;
            end
            UNLOAD:
            begin
                if (start)
                    state = LOAD;
                else
                    state = IDLE;
            end
        endcase
end

always_ff @(posedge clk) begin
    if (state == TRANSACT)
        miso_d = {miso_d[DATA_BIT_WIDTH-2:0], miso};
    else
        miso_d <= 0;
end

always_ff @(negedge clk) begin
    if (state == TRANSACT)
        mosi_d <= {mosi_d[DATA_BIT_WIDTH-2:0], 1'b0};
    else if (state == LOAD)
        mosi_d <= dataIn;
    else
        mosi_d <= 0;
        
    if (state == UNLOAD)
        dataOut <= miso_d;
    else
        dataOut <= 0;
        
     if (state == LOAD)
        count <= DATA_BIT_WIDTH;
     else if (state == TRANSACT)
        count = count - 1;
     else
        count <= 0;
end

endmodule
