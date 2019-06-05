#include "GenericModule.hpp"
#include "../MicroPackets.hpp" //todo setup include dir correctly

class MotionControlModule : public GenericModule {
public:
    // How many times per second this module should run
    const static float freq = 100.0f; // Hz
    const static uint32_t period = (uint32_t) (1000 / freq);

    // How long a single call to this module takes
    const static uint32_t runtime = 0; // ms

    MotionControlModule(BatteryVoltage *const batteryVoltage,
                        IMUData *const imuData,
                        MotionCommand *const motionCommand,
                        MotorFeedback *const motorFeedback,
                        MotorCommand *const motorCommand);

    virtual void entry(void);

private:
    BatteryVoltage *const batteryVoltage;
    IMUData *const imuData;
    MotionCommand *const motionCommand;
    MotorFeedback *const motorFeedback;
    MotorCommand *const motorCommand;

    // Estimator
    // Controller    
};