project(FreeRTOS_lib VERSION 9.0 LANGUAGES C)

add_library(FreeRTOS STATIC
    src/CMSIS_RTOS/cmsis_os.c
    src/croutine.c
    src/event_groups.c
    src/list.c
    src/queue.c
    src/tasks.c
    src/timers.c
    src/portable/Common/mpu_wrappers.c
    src/portable/GCC/ARM_CM7/r0p1/port.c
    src/portable/MemMang/heap_4.c
)

target_include_directories(FreeRTOS PUBLIC
    src/include
    src/CMSIS_RTOS
    src/portable/GCC/ARM_CM7/r0p1
)

target_link_libraries(FreeRTOS STM32F7xx)
