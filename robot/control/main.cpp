#include "mtrain.hpp"
#include "modules/comm/CommTask.hpp"
#include "modules/comm/CommMicroPackets.hpp"

int main() {
    KickerCommand kickerCommand;
    MotorCommand motorCommand;

    KickerStatus kickerStatus;
    MotorStatus motorStatus;
    RobotStatus robotStatus;

    {
        CommTask comm;

        comm.receive(kickerCommand, motorCommand);

        // Transfer commands to other modules
        // Let them deal with whether it's invalid or not

        // Get status from the modules
        // Should just be able to pull

        comm.send(kickerStatus, motorStatus, robotStatus);
    }
    while (true) { }
}
