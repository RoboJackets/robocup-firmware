//////////////////////////////////////////////////////////////////////////////////
// Company: RoboJackets
// Engineer: Arthur Siqueira
// 
// Create Date: 02/10/2020 12:08:15 AM
// Design Name: Fifo Module
// Module Name: FifoModule
// Project Name: RoboCup
// Target Devices: Artix 7
// Tool Versions: 2019.2
// Description: 
// 
// Dependencies: 
// 
// Revision:0.01
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

`ifndef _FIFO_MODULE_
`define _FIFO_MODULE_

module FifoModule #(
    MAX_SIZE = 10, 
    DATA_BIT_WIDTH = 32,
    ALMOST_FULL_SIZE = 8,
    ALMOST_EMPTY_SIZE = 2,
    localparam COUNT_SIZE = $clog2(MAX_SIZE)
)( 
	input clk, 
    input [DATA_BIT_WIDTH - 1:0] dataIn, 
    input rd, 
    input wr, 
    output logic [DATA_BIT_WIDTH-1:0] dataOut, 
    input rst,
    output empty, 
    output full,
    output almost_full,
    output almost_empty
);


reg [COUNT_SIZE - 1:0]      count = 0; 
reg [DATA_BIT_WIDTH - 1:0]  queue [0:MAX_SIZE - 1]; 
reg [COUNT_SIZE - 1:0]      front = 0; 

assign empty = (count == 0)? 1'b1:0; 
assign full = (count == MAX_SIZE)? 1'b1:0; 
assign almost_empty = (count <= ALMOST_EMPTY_SIZE)? 1'b1:0;
assign almost_full = (count >= ALMOST_FULL_SIZE)? 1'b1:0;

generate
    if (MAX_SIZE <= 1)
    begin: INVALID_MAX_SIZE_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. MAX_SIZE must be > 1.", MAX_SIZE);
    end
    
    if (DATA_BIT_WIDTH <= 0)
    begin: INVALID_DATA_WIDTH_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. DATA_BIT_WIDTH must be > 0.", DATA_BIT_WIDTH);
    end
    
    if (ALMOST_FULL_SIZE > MAX_SIZE || ALMOST_FULL_SIZE <= 0)
    begin: INVALID_ALMOST_FULL_SIZE_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. ALMOST_FULL_SIZE must be <= FULL and > 0.", ALMOST_FULL_SIZE);
    end

    if (ALMOST_EMPTY_SIZE < 0)
    begin: INVALID_ALMOST_EMPTY_SIZE_ERROR
        $fatal(1, "Fatal elaboration error. Invalid parameter value %b. ALMOST_FULL_SIZE must be >= 0.", ALMOST_EMPTY_SIZE);
    end

endgenerate


always_ff @(posedge clk) begin
    if (rst) 
    begin: RESET 
        front <= 0;
        count <= 0; 
    end else begin
        
        if (rd == 1'b1 && count != 0)
        begin: READ 
            count = count - 1;
            dataOut = queue[front]; 
            front = (front + 1) % MAX_SIZE; 
        end
        
        if (wr == 1'b1 && count < MAX_SIZE)
        begin: WRITE
            queue[(front + count) % MAX_SIZE] = dataIn; 
            count = count + 1; 
        end
    end 
end 

endmodule

`endif