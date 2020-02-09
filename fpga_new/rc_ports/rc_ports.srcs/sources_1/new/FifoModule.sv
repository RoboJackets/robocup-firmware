`ifndef _FIFO_MODULE_
`define _FIFO_MODULE_

module FifoModule( 
	clk, 
    dataIn, 
    RD, 
    WR, 
    EN, 
    dataOut, 
    rst,
    EMPTY, 
    FULL,
    ALMOST_FULL,
    ALMOST_EMPTY
);

parameter MAX_SIZE = 10; 
parameter DATA_BIT_WIDTH = 32;
parameter ALMOST_FULL_SIZE = 8;
parameter ALMOST_EMPTY_SIZE = 2;



input                           clk, RD, WR, EN, rst;
input [DATA_BIT_WIDTH - 1:0]    dataIn;


output                          EMPTY, FULL, ALMOST_FULL, ALMOST_EMPTY;
output reg [DATA_BIT_WIDTH-1:0] dataOut;  


reg [$clog2(MAX_SIZE) - 1:0]    count = 0; 
reg [DATA_BIT_WIDTH - 1:0]      queue [0:MAX_SIZE - 1]; 
reg [$clog2(MAX_SIZE) - 1:0]    front = 0; 

assign EMPTY = (count == 0)? 1'b1:0; 
assign FULL = (count == MAX_SIZE)? 1'b1:0; 
assign ALMOST_EMPTY = (count <= ALMOST_EMPTY_SIZE)? 1'b1:0;
assign ALMOST_FULL = (count >= ALMOST_FULL_SIZE)? 1'b1:0;

always @ (posedge clk) begin 

    if (EN) begin 
        if (rst) begin 
            front = 0;
            count = 0; 
        end else if (RD == 1'b1 && count != 0) begin 
            dataOut  = queue[front]; 
            front = (front + 1) % MAX_SIZE;
            count = count - 1; 
        end else if (WR == 1'b1 && count < MAX_SIZE) begin
            queue[(front + count) % MAX_SIZE]  = dataIn; 
            count = count + 1; 
        end 
    end 

end 

endmodule

`endif