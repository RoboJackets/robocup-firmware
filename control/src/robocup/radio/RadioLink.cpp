#include "radio/RadioLink.hpp"
#include "drivers/ISM43340.hpp"

#include "MicroPackets.hpp"
#include "radio_protocol.pb.h"
#include <pb.h>
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>

RadioLink::RadioLink() {}

void RadioLink::init() {
    std::unique_ptr radioSPI = std::make_unique<SPI>(SpiBus5, std::nullopt, 16'000'000);
    radio = std::make_unique<ISM43340>(std::move(radioSPI),
                                       RADIO_R0_CS,
                                       RADIO_GLB_RST,
                                       RADIO_R0_INT);
    radioInitialized = radio->isInitialized();
}

// Keep these variables in static memory. We'd rather get link errors if they're too big.
// We only ever use one of these at a time
static union {
    rj_radio_RobotStatus robot_status_proto;
    rj_radio_RobotCommand robot_command_proto;
};
// Block off a huge chunk of memory. It's really only this big because of (optional) log messages.
static std::array<uint8_t, 16 * 1024> buffer{};

void RadioLink::send(const BatteryVoltage &batteryVoltage,
                     const FPGAStatus &fpgaStatus,
                     const KickerInfo &kickerInfo,
                     const RobotID &robotID,
                     DebugInfo &debugInfo) {
    // Build the proto struct
    robot_status_proto = {};

    robot_status_proto.has_last_received_command = false;
    robot_status_proto.has_last_kicker_fire = false;
    robot_status_proto.ball_sense_triggered = kickerInfo.ballSenseTriggered;

    // Set up motion debug frames
    {
        constexpr size_t kMaxNumMotionDebugInfo =
                sizeof(robot_status_proto.motion_debug_info) / sizeof(rj_radio_MotionDebugInfo);
        robot_status_proto.motion_debug_info_count = std::min<size_t>(debugInfo.debug_frames.size(),
                                                                      kMaxNumMotionDebugInfo);
        size_t start = std::max<size_t>(kMaxNumMotionDebugInfo, debugInfo.debug_frames.size()) - kMaxNumMotionDebugInfo;
        size_t end = std::min<size_t>(start + kMaxNumMotionDebugInfo, debugInfo.debug_frames.size());
        for (size_t i = start; i < end; i++) {
            const auto &debug_frame = debugInfo.debug_frames.at(i);
            auto *debug_frame_proto = &(robot_status_proto.motion_debug_info[i]);
            debug_frame_proto->frame_time_ms = debug_frame.frame_time_ms;
            debug_frame_proto->wheel_speeds_radps_count = debug_frame.wheel_speeds_radps.size();
            debug_frame_proto->motor_outputs_pwm_count = debug_frame.motor_outputs_pwm.size();
            std::copy(debug_frame.wheel_speeds_radps.begin(), debug_frame.wheel_speeds_radps.end(),
                      debug_frame_proto->wheel_speeds_radps);
            std::copy(debug_frame.motor_outputs_pwm.begin(), debug_frame.motor_outputs_pwm.end(),
                      debug_frame_proto->motor_outputs_pwm);

            debug_frame_proto->has_filtered_pose = true;
            debug_frame_proto->filtered_pose.x_position_m = debug_frame.filtered_pose[0];
            debug_frame_proto->filtered_pose.y_position_m = debug_frame.filtered_pose[1];
            debug_frame_proto->filtered_pose.heading_rad = debug_frame.filtered_pose[2];

            debug_frame_proto->has_filtered_velocity = true;
            debug_frame_proto->filtered_velocity.x_velocity_mps = debug_frame.filtered_velocity[0];
            debug_frame_proto->filtered_velocity.y_velocity_mps = debug_frame.filtered_velocity[1];
            debug_frame_proto->filtered_velocity.w_velocity_radps = debug_frame.filtered_velocity[2];

            debug_frame_proto->has_latest_command = false;

            debug_frame_proto->gyro_rate_radps = debug_frame.gyro_z_radps;
            debug_frame_proto->measured_acceleration_x_mpss = debug_frame.measured_acceleration[0];
            debug_frame_proto->measured_acceleration_y_mpss = debug_frame.measured_acceleration[1];
            debug_frame_proto->measured_acceleration_z_mpss = debug_frame.measured_acceleration[2];
        }
    }

    // Only send the _last_ 16 messages
    {
        constexpr size_t kMaxNumLogMessages = sizeof(robot_status_proto.log_messages) / sizeof(rj_radio_LogMessage);
        robot_status_proto.log_messages_count = std::min<size_t>(debugInfo.log_messages.size(), kMaxNumLogMessages);
        size_t start = std::max<size_t>(kMaxNumLogMessages, debugInfo.log_messages.size()) - kMaxNumLogMessages;
        size_t end = std::min<size_t>(start + kMaxNumLogMessages, debugInfo.log_messages.size());
        for (size_t i = start; i < end; i++) {
            const auto &log_message = debugInfo.log_messages.at(i);
            robot_status_proto.log_messages[i].level = static_cast<rj_radio_LogMessage_LogLevel>(static_cast<int>(log_message.level));
            robot_status_proto.log_messages[i].timestamp_ms = log_message.timestamp_ms;
            strncpy(robot_status_proto.log_messages[i].subsystem, log_message.subsystem.data(),
                    sizeof(robot_status_proto.log_messages[i].subsystem) / sizeof(char));
            strncpy(robot_status_proto.log_messages[i].message, log_message.message.data(),
                    sizeof(robot_status_proto.log_messages[i].message) / sizeof(char));
        }
    }

    pb_ostream_t buffer_stream = pb_ostream_from_buffer(buffer.data(), buffer.size());

    if (!pb_encode(&buffer_stream, rj_radio_RobotCommand_fields, &robot_status_proto)) {
        // Failed to encode
        // TODO: warn
        return;
    }

    radio->send(buffer.data(), buffer_stream.bytes_written);
}

bool RadioLink::receive(KickerCommand &kickerCommand,
                        MotionCommand &motionCommand) {
    // Make sure there is actually data to read
    if (!radio->isAvailable()) {
        cyclesWithoutPackets++;
        return false;
    }

    size_t num_bytes = radio->receive(buffer.data(), buffer.size());
    pb_istream_t buffer_stream = pb_istream_from_buffer(buffer.data(), num_bytes);

    robot_command_proto = {};

    if (!pb_decode(&buffer_stream, rj_radio_RobotCommand_fields, &robot_command_proto)) {
        // TODO: Log an error
        cyclesWithoutPackets++;
        return false;
    }

    kickerCommand.isValid = true;
    kickerCommand.lastUpdate = HAL_GetTick();
    // All possible values must be defined in the enum otherwise this is undefined behavior
    if (robot_command_proto.which_kicker_command == rj_radio_RobotCommand_kick_tag) {
        kickerCommand.kickStrength = 128;
        kickerCommand.shootMode = KickerCommand::ShootMode::KICK;
        kickerCommand.triggerMode = kickerCommand.triggerMode;
    } else if (robot_command_proto.which_kicker_command == rj_radio_RobotCommand_chip_tag) {
        kickerCommand.kickStrength = 128;
        kickerCommand.shootMode = KickerCommand::ShootMode::KICK;
        kickerCommand.triggerMode = kickerCommand.triggerMode;
    }

    if (robot_command_proto.motion_command.which_motion_command == rj_radio_MotionCommand_local_velocity_tag) {
        const auto &velocity = robot_command_proto.motion_command.motion_command.local_velocity.robot_relative_velocity;
        motionCommand.bodyWVel = velocity.w_velocity_radps;
        motionCommand.bodyXVel = velocity.x_velocity_mps;
        motionCommand.bodyYVel = velocity.y_velocity_mps;

        motionCommand.isValid = true;
        motionCommand.lastUpdate = HAL_GetTick();
    } else if (robot_command_proto.motion_command.which_motion_command == rj_radio_MotionCommand_global_velocity_tag) {
        // Error!
        return false;
    }

    if (robot_command_proto.dribbler_speed_radps > 0) {
        motionCommand.dribbler = 128;
    }

    cyclesWithoutPackets = 0;
    return true;
}
