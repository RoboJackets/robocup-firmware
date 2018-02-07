/*
 *  This Verilog header file is where you can define or comment out areas
 *  during FPGA synthesis.
 */

`ifndef _ROBOCUP_HEADER_
`define _ROBOCUP_HEADER_

/**
 * Enable/Disable the module for the dribbler motor.
 */
// `define DRIBBLER_MOTOR_DISABLE

/**
 * Select which type of PWM scheme to use.
 */
`define PWM_USE_SIX_STEP
// `define PWM_USE_PWM_ON_PWM


////////////////////////////////////////////////////////////////////////////////
`ifdef XILINX_ISIM
`define __SIMULATION__
`endif  // XILINX_ISIM

`ifdef __ICARUS__
`define __SIMULATION__
`endif  // __ICARUS__

`ifdef __SIMULATION__
`undef DRIBBLER_MOTOR_DISABLE
`endif  // __SIMULATION__

`define MAX_VALUE(WIDTH) ((1 << WIDTH) - 1)

`endif  // _ROBOCUP_HEADER_
