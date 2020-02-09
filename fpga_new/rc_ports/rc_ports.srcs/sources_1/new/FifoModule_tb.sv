`timescale 1ns/10ps


module FifoModule_tb;


localparam MAX_SIZE = 10; 
localparam DATA_BIT_WIDTH = 32;
localparam ALMOST_FULL_SIZE = 8;
localparam ALMOST_EMPTY_SIZE = 2;

reg                     	clk, RD, WR, EN, rst;
reg [DATA_BIT_WIDTH - 1:0]	dataIn;


wire                    	EMPTY, FULL, ALMOST_FULL, ALMOST_EMPTY;
wire [DATA_BIT_WIDTH - 1:0] dataOut;  

FifoModule #(
  .DATA_BIT_WIDTH(DATA_BIT_WIDTH),
  .MAX_SIZE(MAX_SIZE),
  .ALMOST_EMPTY_SIZE(ALMOST_EMPTY_SIZE),
  .ALMOST_FULL_SIZE(ALMOST_FULL_SIZE)
  ) dut (
	.clk(clk),
	.dataIn(dataIn),
	.RD(RD),
	.WR(WR),
	.EN(EN),
    .dataOut(dataOut), 
    .rst(rst),
    .EMPTY(EMPTY), 
    .FULL(FULL),
    .ALMOST_FULL(ALMOST_FULL),
    .ALMOST_EMPTY(ALMOST_EMPTY)
);
	

initial begin

	// Pre-reset
	clk = 0;
  	dataIn = 0;
  	RD = 0;
  	WR = 0;
  	EN = 0;
  	rst = 1'b1;

  	#2;
  	// Reset
  	EN  = 1'b1;
  	// Load from 1 to 10
  	#1;
  	rst = 0;
  	WR 	= 1'b1;
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
  	dataIn = 32'hA;
  	#1;
  	dataIn = 32'hB; // Shouldn't add this
  	#1;
  	WR = 0;
  	RD = 1'b1;
  	#10;
  	RD = 0;
  	WR 	= 1'b1;
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
  	dataIn = 32'hA;
  	#1;
  	dataIn = 32'hB; // Shouldn't add this
  	#1;
  	WR = 0;
  	RD = 1'b1;
end



always #0.5 clk = ~clk;

endmodule