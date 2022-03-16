// #pragma once

// #include <LockedStruct.hpp>
// #include "GenericModule.hpp"
// #include "MicroPackets.hpp" 
// #include "drivers/TSL2572.hpp"
// #include "drivers/PCA9858PWJ.hpp"

// /**
//  * Module interfacing with Rotary Dial to handle Robot IDs
//  */
// class RobotShellModule : public GenericModule {
// public:
//     /**
//      * Number of times per second (frequency) that RobotShellModule should run (Hz)
//      */
//     static constexpr float kFrequency = 1.0f;

//     /**
//      * Number of seconds elapsed (period) between RobotShellModule runs (milliseconds)
//      */
//     static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

//     /**
//      * Priority used by RTOS
//      */
//     static constexpr int kPriority = 3;

//     /**
//      * Constructor for RobotShellModule
//      * 
//      */
//     RobotShellModule();

//     /**
//      * Code which initializes module
//      */
//     void start() override;

//     /**
//      * Code to run when called by RTOS once per system tick (`kperiod`)
//      *
//      * 
//      */
//     void entry() override;

// private:

// };