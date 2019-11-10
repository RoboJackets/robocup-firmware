/*
 *  Selectable PWM schemes.
 */

`ifndef _HALL_EFFECT_SENSOR_
`define _HALL_EFFECT_SENSOR_

`include "robocup.vh"

`ifdef PWM_USE_SIX_STEP
    `include "Hall_Effect_Sensor_6StepPWM.v"
`elsif PWM_USE_PWM_ON_PWM
    `include "Hall_Effect_Sensor_PWMonPWM.v"
`else
    `ERROR__must_define_pwm_type_in__robocup_vh
`endif

`endif  // _HALL_EFFECT_SENSOR_
