#ifndef CONTROL_ROBOCUP_HPP
#define CONTROL_ROBOCUP_HPP

/**
 * @file robocup.hpp
 * @brief global configuration values for robocup firmware are defined here
 */

/**
 * @def USING_RTOS
 * @brief  Whether or not we are using an RTOS.
 *       This affects how we should be delaying in some places
 */
#define USING_RTOS

/**
 * @def NORMAL_BASESTATION_PORT
 * @brief Comment this out when running with an alternate basestation port
 *        (used for two team control)
 */
#define NORMAL_BASESTATION_PORT

#endif  // CONTROL_ROBOCUP_HPP
