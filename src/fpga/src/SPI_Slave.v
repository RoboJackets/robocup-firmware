`ifndef _SPI_SLAVE_
`define _SPI_SLAVE_



module SPI_Slave #(parameter DATA_BIT_WIDTH = 8) ( clk, SCK, MOSI, MISO, SSEL, DONE, DATA_OUT, DATA_IN );

input         clk, SCK, SSEL, MOSI;
input  [DATA_BIT_WIDTH - 1:0]  DATA_OUT;
output        MISO, DONE;
output [DATA_BIT_WIDTH - 1:0]  DATA_IN;

`include "log2-macro.v"
localparam DATA_BIT_COUNTER_WIDTH = `LOG2(DATA_BIT_WIDTH);
localparam ONE = {{(DATA_BIT_COUNTER_WIDTH - 1){1'b0}}, {1'b1}};

// sync SCK to the FPGA clock using a 3-bits shift register
reg [2:0] SCKr;  always @(posedge clk) SCKr <= {SCKr[1:0], SCK};
wire SCK_risingedge = (SCKr[2:1]==2'b01),  // now we can detect SCK rising edges
         SCK_fallingedge = (SCKr[2:1]==2'b10);  // and falling edges

// same thing for SSEL
reg [2:0] SSELr;  always @(posedge clk) SSELr <= {SSELr[1:0], SSEL};
wire SSEL_active = ~SSELr[1],
         SSEL_startmessage = (SSELr[2:1]=='b10),  // message starts at falling edge
         SSEL_endmessage = (SSELr[2:1]==2'b01);  // message stops at rising edge

// and for MOSI
reg [1:0] MOSIr;  always @(posedge clk) MOSIr <= {MOSIr[0], MOSI};
wire MOSI_data = MOSIr[1];

reg [1:0] DONE_d; always @(posedge clk) DONE_d <= {DONE_d[0], DONE};

// we handle SPI in 8-bits format, so we need a 3 bits counter to count the bits as they come in
reg [DATA_BIT_COUNTER_WIDTH-1:0] bitcnt;
reg [DATA_BIT_WIDTH - 1:0] byte_data_received,
                    byte_data_sent,
                    byte_rec_;

assign MISO = byte_data_sent[DATA_BIT_WIDTH - 1];  // send MSB first
// signal out to load the next byte half an SCK period before we latch it
assign DONE = (SSEL_active && SCK_fallingedge && (bitcnt == 0));
assign DATA_IN = byte_rec_;

always @(posedge clk)
begin
    if (~SSEL_active)
        bitcnt <= 0;

    else if(SCK_risingedge) begin
        bitcnt <= bitcnt + ONE;
        // shift reg. for data being received
        byte_data_received <= {byte_data_received[DATA_BIT_WIDTH - 2:0], MOSI_data};
    end
end

always @(negedge clk) byte_rec_ <= DONE ? byte_data_received : byte_rec_;

always @(negedge clk)
if(SSEL_active)
begin
    if ( (bitcnt == 0 && DONE_d == 2'b10) || SSEL_startmessage ) begin
        byte_data_sent <= DATA_OUT;  // after that, we send 0s

    end else if ( SCK_fallingedge ) begin
        if ( bitcnt != 0 ) begin
            byte_data_sent <= {byte_data_sent[DATA_BIT_WIDTH - 2:0], 1'b0};
        end
    end
end

endmodule

`endif
