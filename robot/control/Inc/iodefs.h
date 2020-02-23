#pragma once
#include "drivers/MCP23017.hpp"

#define SHARED_I2C_BUS I2CBus::I2CBus1

#define SHARED_SPI_BUS SpiBus::SpiBus2

#define RADIO_SPI_BUS SpiBus::SpiBus5
#define RADIO_R0_CS   p17
#define RADIO_R0_INT  p30
#define RADIO_R1_CS   p3
#define RADIO_R1_INT  p18
#define RADIO_GLB_RST p19

#define FPGA_SPI_BUS SpiBus::SpiBus3
#define FPGA_PROG  p13
#define FPGA_INIT  p14
#define FPGA_DONE  p15
#define FPGA_CS    p31
#define KICKER_CS  p8
#define KICKER_RST p7

#define DOT_STAR_CS p11

#define HEX_SWITCH_BIT0 MCP23017::ExpPinName::PinA7
#define HEX_SWITCH_BIT1 MCP23017::ExpPinName::PinA4
#define HEX_SWITCH_BIT2 MCP23017::ExpPinName::PinA6
#define HEX_SWITCH_BIT3 MCP23017::ExpPinName::PinA5
#define DIP_SWITCH_1    MCP23017::ExpPinName::PinA1
#define DIP_SWITCH_2    MCP23017::ExpPinName::PinA2
#define DIP_SWITCH_3    MCP23017::ExpPinName::PinA3
#define PUSHBUTTON      MCP23017::ExpPinName::PinA0

#define ERR_LED_M1    MCP23017::ExpPinName::PinB6
#define ERR_LED_M2    MCP23017::ExpPinName::PinB5
#define ERR_LED_M3    MCP23017::ExpPinName::PinB3
#define ERR_LED_M4    MCP23017::ExpPinName::PinB4
#define ERR_LED_MPU   MCP23017::ExpPinName::PinB2
#define ERR_LED_DRIB  MCP23017::ExpPinName::PinB7
#define ERR_LED_RADIO MCP23017::ExpPinName::PinB1
#define ERR_LED_KICK  MCP23017::ExpPinName::PinB0
