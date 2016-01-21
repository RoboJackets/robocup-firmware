`ifndef _SPI_MASTER_
`define _SPI_MASTER_

// ClkDivide module
module ClkDivide ( CLK_IN, EN, CLK_OUT );

parameter DIVISION = 3;

`include "log2-macro.v"     // This must be included here
localparam WIDTH = `LOG2( DIVISION );

input CLK_IN, EN;
output CLK_OUT;

reg [WIDTH-1:0] edge_cnt;

// Posedge counter
always @(posedge CLK_IN)
begin
  if ( EN == 0 ) begin
    edge_cnt <= 0;
  end else if ( edge_cnt >= DIVISION ) begin
    edge_cnt <= 0;
  end else begin
    edge_cnt <= edge_cnt + 1;
  end
end

assign CLK_OUT = edge_cnt[WIDTH-1];

endmodule // ClkDivide


// SPI_Master module
module SPI_Master ( clk, EN, SCK, MOSI, MISO, SEL, START, BUSY, VALID, DATA_OUT, DATA_IN );

// Module parameters - passed parameters will overwrite the values here
parameter DATA_BIT_WIDTH = 16;

// Local parameters - can not be altered outside this module
`include "log2-macro.v"     // This must be included here
localparam DATA_BIT_COUNTER_WIDTH = `LOG2( DATA_BIT_WIDTH );

input clk, EN, MISO, START;
input [DATA_BIT_WIDTH-1:0]  DATA_IN;
output MOSI, SEL, VALID, BUSY, SCK;
output [DATA_BIT_WIDTH-1:0]  DATA_OUT;


reg transfer_active;
reg select;
wire clksub;

ClkDivide #(
  .DIVISION ( 8                 )
  ) sck_gen (
  .CLK_IN   ( clk               ),
  .EN       ( transfer_active   ),
  .CLK_OUT  ( clksub            )
);

// 2 bit shift register for detecting rising edge of START input
reg [1:0] STARTr;  always @(posedge clk) STARTr <= {STARTr[0], START};
wire begin_transfer   =   ( STARTr == 2'b01 );

reg [DATA_BIT_COUNTER_WIDTH-1:0] bitcnt;
reg [DATA_BIT_WIDTH-1:0] byte_data_received,
          byte_data_sent,
          byte_rec_;

reg [DATA_BIT_WIDTH-1:0] data_incoming_l;
reg [DATA_BIT_WIDTH-1:0] data_in_l;
reg [DATA_BIT_WIDTH-1:0] data_recv;

wire end_transfer = ( transfer_active && ( bitcnt == DATA_BIT_WIDTH - 1 ) );

reg valid_q;
reg miso_q;

assign VALID = valid_q;
assign BUSY = transfer_active;
assign SEL = select;
assign SCK = clksub;
assign MOSI = data_in_l[0] && transfer_active;
assign DATA_OUT = valid_q ? data_incoming_l : 0;

// rising/falling edges of SCK
reg [2:0] SCKr;  always @(posedge clk) SCKr <= {SCKr[1:0], SCK};
wire SCK_risingedge   =   ( SCKr[2:1] == 2'b01 ),  // now we can detect SCK rising edges
     SCK_fallingedge  =   ( SCKr[2:1] == 2'b10 );  // and falling edges

always @(posedge clk)
begin
  if (EN != 1) begin
    transfer_active <= 0;
    select <= 0;
    valid_q <= 0;
  end

  if ( begin_transfer == 1 ) begin
    transfer_active <= 1;
    select <= 1;
    data_in_l <= DATA_IN;
    valid_q <= 0;
  end else if ( end_transfer == 1 ) begin
    transfer_active <= 0;
    select <= 0;
    valid_q <= 1;
  end
end

always @(negedge clk)
begin
  if (EN != 1) begin
    bitcnt <= 0;
    data_incoming_l <= 0;
    data_in_l <= 0;
  end

  miso_q <= MISO;

  if ( transfer_active == 1 ) begin
    if ( SCK_fallingedge == 1 ) begin
      bitcnt <= bitcnt + 1;
      data_in_l <= {1'b0, data_in_l[DATA_BIT_WIDTH-1:1]};
    end else if ( SCK_risingedge == 1 ) begin
      data_incoming_l <= {data_incoming_l[DATA_BIT_WIDTH-2:0], miso_q};
    end
  end else begin
    bitcnt <= 0;
  end
end

endmodule

`endif
