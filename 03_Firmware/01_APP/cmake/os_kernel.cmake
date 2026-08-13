# OS kernel selection.
#
# This file is owned by the project, NOT by STM32CubeMX. The RTOS kernel used
# to be declared inside cmake/stm32cubemx/CMakeLists.txt, which CubeMX
# regenerates; the kernel tree has since moved under 04_Impl/impl_os/ so that
# the OSAL implementation layer owns its backend end to end.
#
# Kernel trees live one directory per RTOS, vendor sources only:
#   04_Impl/impl_os/00_RT_Thread_Kernel/   (planned)
#   04_Impl/impl_os/01_FreeRTOS_Kernel/
#
# The matching OSAL implementation (os_impl_*.c) stays outside those
# directories, under 04_Impl/impl_os/, so vendor code and project code are
# never mixed in the same tree.

set(FREERTOS_KERNEL_ROOT "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/01_FreeRTOS_Kernel")

set(FreeRTOS_Src
    "${FREERTOS_KERNEL_ROOT}/src/croutine.c"
    "${FREERTOS_KERNEL_ROOT}/src/event_groups.c"
    "${FREERTOS_KERNEL_ROOT}/src/list.c"
    "${FREERTOS_KERNEL_ROOT}/src/queue.c"
    "${FREERTOS_KERNEL_ROOT}/src/stream_buffer.c"
    "${FREERTOS_KERNEL_ROOT}/src/tasks.c"
    "${FREERTOS_KERNEL_ROOT}/src/timers.c"
    "${FREERTOS_KERNEL_ROOT}/src/cmsis_os2.c"
    "${FREERTOS_KERNEL_ROOT}/src/heap_4.c"
    "${FREERTOS_KERNEL_ROOT}/src/port.c"
)

# Create FreeRTOS object library
add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${FreeRTOS_Src})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)
