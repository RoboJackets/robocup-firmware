/*
*  BLDC_Motor.v
*
*  A BLDC controller that includes hall sensor error detection and incremental
*  duty cycles during startup for reducing motor startup current.
*
*/

`ifndef _BLDC_MOTOR_
`define _BLDC_MOTOR_

`include "robocup.vh"

`include "BLDC_Driver.v"
`include "BLDC_Hall_Counter.v"
`include "BLDC_Encoder_Counter.v"
`include "BLDC_Encoder_Checker.v"
`include "IIR_LowPass_Filter.v"


// BLDC_Motor module
module BLDC_Motor ( clk, en, reset_enc_count, reset_hall_count, duty_cycle, enc, hall, phaseH, phaseL, enc_count, hall_count, has_error );

// Module parameters - passed parameters will overwrite the values here
parameter MAX_DUTY_CYCLE =          ( 'h1FF             );
parameter MAX_DUTY_CYCLE_COUNTER =  ( MAX_DUTY_CYCLE    );
parameter ENCODER_COUNT_WIDTH =     ( 15                );
parameter HALL_COUNT_WIDTH =        ( 7                 );

// Local parameters - can not be altered outside this module
`include "log2-macro.v"     // This must be included here
localparam DUTY_CYCLE_WIDTH =   `LOG2( MAX_DUTY_CYCLE );

// Module inputs/outputs
input clk, en, reset_enc_count, reset_hall_count;
input [DUTY_CYCLE_WIDTH-1:0] duty_cycle;
input [1:0] enc;
input [2:0] hall;
output [2:0] phaseH, phaseL;
output signed [ENCODER_COUNT_WIDTH-1:0] enc_count;
output signed [HALL_COUNT_WIDTH-1:0] hall_count;
output has_error;
// ===============================================

wire has_error, has_hall_fault, has_enc_fault;
wire signed [ENCODER_COUNT_WIDTH-1:0] enc_count_raw;
wire signed [HALL_COUNT_WIDTH-1:0] hall_count_raw;

// Show the expected startup length during synthesis. Assumes an 18.432MHz input clock.
initial begin
    $display ("Duty cycle width from BLDC_Motor.v:\t%d", DUTY_CYCLE_WIDTH);
end

// Instantiation of all the modules required for complete functioning with all sensors
// ===============================================
BLDC_Encoder_Counter #(         // Instantiation of the encoder for counting the ticks
    .COUNTER_WIDTH              ( ENCODER_COUNT_WIDTH       )
    ) encoder_counter (
    .clk                        ( clk                       ) ,
    .reset                      ( reset_enc_count           ) ,
    .enc                        ( enc                       ) ,
    .count                      ( enc_count_raw             )
);

BLDC_Hall_Counter #(            // Instantiation of the hall effect sensor's counter
    .COUNTER_WIDTH              ( HALL_COUNT_WIDTH          )
    ) hall_counter (
    .clk                        ( clk                       ) ,
    .reset                      ( reset_hall_count          ) ,
    .hall                       ( hall                      ) ,
    .count                      ( hall_count_raw            )
);

IIR_LowPass_Filter #(           // IIR filter for the encoder count value
    .WIDTH                      ( ENCODER_COUNT_WIDTH       ) ,
    .GAIN                       ( 5                         )
    ) encoder_count_filterer (
    .clk                        ( clk                       ) ,
    .reset                      ( reset_enc_count           ) ,
    .en                         ( en                        ) ,
    .in                         ( enc_count_raw             ) ,
    .out                        ( enc_count                 )
);

IIR_LowPass_Filter #(           // IIR filter for the hall count value
    .WIDTH                      ( HALL_COUNT_WIDTH          ) ,
    .GAIN                       ( 12                        )
    ) hall_count_filterer (
    .clk                        ( clk                       ) ,
    .reset                      ( reset_hall_count          ) ,
    .en                         ( en                        ) ,
    .in                         ( hall_count_raw            ) ,
    .out                        ( hall_count                )
);

BLDC_Encoder_Checker #(         // Instantiation of the encoder checker
    .ENCODER_COUNTER_WIDTH      ( ENCODER_COUNT_WIDTH       ) ,
    .HALL_COUNTER_WIDTH         ( HALL_COUNT_WIDTH          )
    ) encoder_checker (
    .clk                        ( clk                       ) ,
    .reset                      ( ~en                       ) ,
    .enc_count                  ( enc_count                 ) ,
    .hall_count                 ( hall_count                ) ,
    .fault                      ( has_enc_fault             )
);

BLDC_Driver #(                  // Instantiation of the motor driving module
    .PHASE_DRIVER_MAX_COUNTER   ( MAX_DUTY_CYCLE_COUNTER    ) ,
    .MAX_DUTY_CYCLE             ( MAX_DUTY_CYCLE            ) ,
    .DUTY_CYCLE_STEP_RES        ( 1                         ) ,
    .DEAD_TIME                  ( 10                        )
    ) bldc_motor (
    .clk                        ( clk                       ) ,
    .en                         ( en                        ) ,
    .hall                       ( hall                      ) ,
    .direction                  ( duty_cycle[DUTY_CYCLE_WIDTH-1] ) ,
    .duty_cycle                 ( { duty_cycle[DUTY_CYCLE_WIDTH-2:0], 1'b0 } ) ,
    .phaseH                     ( phaseH                    ) ,
    .phaseL                     ( phaseL                    ) ,
    .connected                  ( is_hall_connected         ) ,
    .fault                      ( has_hall_fault            )
);

wire has_fault = has_hall_fault | has_enc_fault;

assign has_error = ~is_hall_connected | has_fault;

endmodule

`endif
