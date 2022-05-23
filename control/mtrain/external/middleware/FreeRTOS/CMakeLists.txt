project(FreeRTOS_lib LANGUAGES C)

cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_C_STANDARD 11 CACHE STRING "")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "")

add_library(FreeRTOS STATIC
    Source/CMSIS_RTOS/cmsis_os.c
    Source/croutine.c
    Source/event_groups.c
    Source/list.c
    Source/queue.c
    Source/tasks.c
    Source/timers.c
    Source/stream_buffer.c
    Source/portable/Common/mpu_wrappers.c
    Source/portable/GCC/ARM_CM7/r0p1/port.c
    Source/portable/MemMang/heap_3.c
)

target_include_directories(FreeRTOS PUBLIC
    Source/include
    Source/CMSIS_RTOS
    Source/portable/GCC/ARM_CM7/r0p1
)

target_link_libraries(FreeRTOS STM32F7xx)
