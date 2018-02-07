/*
*  IIR_LowPass_Filter.v
*
*  Checks encoder inputs for invalid states.
*
*/

`ifndef _IIR_LOWPASS_FILTER_
`define _IIR_LOWPASS_FILTER_

module IIR_LowPass_Filter
#(
    parameter WIDTH =   ( 16 ),
    parameter GAIN  =   ( 4  )
) (
    input clk, reset, en,
    input signed [WIDTH-1:0] in,
    output wire signed [WIDTH-1:0] out
);

reg signed [WIDTH+GAIN-1:0] accumulator;

always @( posedge clk ) begin
    if ( reset ) begin
        accumulator <= 'd0;
    end else if ( en ) begin
        accumulator <= accumulator + in - out;
    end
end

assign out = accumulator[WIDTH+GAIN-1:GAIN];

endmodule

`endif