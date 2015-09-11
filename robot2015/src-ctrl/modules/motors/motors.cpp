#include "motors.hpp"

#include <mbed.h>
#include <numparser.hpp>

#include "commands.hpp"


motor_t mtrEx = {
	.targetVel = 		0x4D,
	.adjVel = 			0x64,
	.hallCount = 		0x0A,
	.encCounts = 		{ 0x23, 0x18 },
	.status = {
		.encOK = 		false,
		.hallOK = 		true,
		.drvStatus = 	{ 0x26, 0x0F }
	},
	.desc = "Motor "
};


std::array<motor_t, 5> motors;

void motors_Init(void)
{
	// ahhh just make a dummy vector of info for now...
	motors.fill(mtrEx);
	motors.at(0).desc = "Dribbler";
	motors.at(1).desc += "1";
	motors.at(2).desc += "2";
	motors.at(3).desc += "3";
	motors.at(4).desc += "4";
}


// An easy-to-read printf/logging function for a single motor
void motors_PrintMotor(motor_t& mtr)
{
	printf(	"%s\r\n  Target Vel:\t\t%u\r\n  Adjusted Vel:\t\t%u\r\n  Hall Cnt:\t\t%u"
	        "\r\n  Encoder Cnt:\t\t%u\t[prev: %u]\r\n  Enc OK:\t\t%s\r\n  Hall OK:\t\t"
	        "%s\r\n  DRV Addr 0x00:\t0x%02X\r\n  DRV Addr 0x01:\t0x%02X\r\n",
	        mtr.desc.c_str(),
	        mtr.targetVel,
	        mtr.adjVel,
	        mtr.hallCount,
	        mtr.encCounts[0],
	        mtr.encCounts[1],
	        mtr.status.encOK ? "YES" : "NO",
	        mtr.status.hallOK ? "YES" : "NO",
	        mtr.status.drvStatus[0],
	        mtr.status.drvStatus[1]
	      );
}


// The console function to run with the 'motor' command
void motors_cmdProcess(const std::vector<std::string>& args)
{
	if (args.empty() == true) {
		printf("Must specify a motor ID!\r\n");
	} else {
		std::vector<uint8_t> motorIDs;

		// Check for valid motor ID arguments
		for (unsigned int i = 0; i < args.size(); i++) {
			std::string mtrArg = args.at(i);

			if (isInt(mtrArg)) {
				// Get the string argument into a type we can work with
				uint8_t mtrID = (uint8_t)atoi(mtrArg.c_str());

				// they gave us an invalid motor ID...
				if (mtrID > 4) {
					printf("Motor with ID %u does not exist.\r\n", mtrID);
					return;
				}

				// Push the ID into the vector if it's not already in it
				if (std::find(motorIDs.begin(), motorIDs.end(), mtrID) == motorIDs.end())
					motorIDs.push_back(mtrID);

			} else {
				showInvalidArgs(args.at(i));
				break;
			}
		}

		// If we make it to this point, all arguments given are valid motor ID numbers - without duplicate entries
		for (unsigned int i = 0; i < motorIDs.size(); i++)
			motors_PrintMotor(motors.at(motorIDs.at(i)));
	}
}
