#pragma once

#include "LockedStruct.hpp"
#include "SPI.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp"
#include "drivers/TMC4671.hpp"
#include "drivers/TMC6200.hpp"

class TrinamicModule : public GenericModule {
    public:
        /**
        * Number of times per second (frequency) that TrinamicModule should run (Hz)
           */
        static constexpr float kFrequency = 25.0f;

        /**
         * Number of seconds elapsed (period) between TrinamicModule runs (milliseconds)
         */
        static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

        /**
         * Priority used by RTOS
         */
        static constexpr int kPriority = 2;

        /**
         * Constructor for TrinamicModule
         * @param spi Pointer to SPI object which handles communication on SPI bus
         * @param trinamicInfo Shared memory location containing trinamic status
         */
        TrinamicModule(LockedStruct<SPI>& spi, LockedStruct<TrinamicInfo>& trinamicInfo);

        /**
         * Code which initializes module
         */
        void start() override;

        /**
         * Code to run when called by RTOS once per system tick (`kperiod`)
         */
        void entry() override;

    private:
        LockedStruct<TrinamicInfo>& trinamicInfo;

        TMC6200 tmc6200;

};