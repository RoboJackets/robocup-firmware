`timescale 1ns/10ps


module FifoModule_tb #(
    localparam MAX_SIZE = 9, 
    localparam DATA_BIT_WIDTH = 32,
    localparam ALMOST_FULL_SIZE = 8,
    localparam ALMOST_EMPTY_SIZE = 2
);

reg                     	clk, rd, wr, rst;
reg [DATA_BIT_WIDTH - 1:0]	dataIn;

wire                    	empty, full, almost_full, almost_empty;
wire [DATA_BIT_WIDTH - 1:0] dataOut;  

FifoModule #(
  .DATA_BIT_WIDTH(DATA_BIT_WIDTH),
  .MAX_SIZE(MAX_SIZE),
  .ALMOST_EMPTY_SIZE(ALMOST_EMPTY_SIZE),
  .ALMOST_FULL_SIZE(ALMOST_FULL_SIZE)
  ) dut (
	.clk(clk),
	.dataIn(dataIn),
	.rd(rd),
	.wr(wr),
    .dataOut(dataOut), 
    .rst(rst),
    .empty(empty), 
    .full(full),
    .almost_full(almost_full),
    .almost_empty(almost_empty)
);
	

initial begin

	// Reset
	clk = 0;
  	dataIn = 0;
  	rd = 0;
  	wr = 0;
  	rst = 1'b1;

  	#2;
  	// Load from 1 to 10
  	#1;
  	rst = 0;
  	wr 	= 1'b1;
  	dataIn = 32'h1;
  	#1;
  	dataIn = 32'h2;
	#1;
  	dataIn = 32'h3;
	#1;
  	dataIn = 32'h4;
	#1;
  	dataIn = 32'h5;
	#1;
  	dataIn = 32'h6;
	#1;
  	dataIn = 32'h7;
  	#1;
  	dataIn = 32'h8;
	#1;
  	dataIn = 32'h9;
  	#1;
  	dataIn = 32'hA; // Shouldn't add this
  	#1;
  	dataIn = 32'hB; // Shouldn't add this
  	#1;
  	wr = 0;
  	rd = 1'b1;
  	#10;
  	rd = 1'b1; //Read and write at the same time
  	wr 	= 1'b1;
  	dataIn = 32'h1;
  	#1;
  	dataIn = 32'h2;
	#1;
  	dataIn = 32'h3;
	#1;
  	dataIn = 32'h4;
	#1;
  	dataIn = 32'h5;
	#1;
  	dataIn = 32'h6;
	#1;
  	dataIn = 32'h7;
  	#1;
  	dataIn = 32'h8;
	#1;
  	dataIn = 32'h9;
  	#1;
  	dataIn = 32'hA; // Shouldn't add this
  	#1;
  	dataIn = 32'hB; // Shouldn't add this
  	#1;
  	wr = 0;
  	rd = 1'b1;
  	#13 $finish;
end



always #0.5 clk = ~clk;

endmodule