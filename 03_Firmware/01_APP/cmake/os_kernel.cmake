# OS kernel selection.
#
# This file is owned by the project, NOT by STM32CubeMX. The RTOS kernel used
# to be declared inside cmake/stm32cubemx/CMakeLists.txt, which CubeMX
# regenerates; the kernel tree has since moved under 04_Impl/impl_os/ so that
# the OSAL implementation layer owns its backend end to end.
#
# This is the ONE place that decides which RTOS the firmware is built against.
# It contributes three things per backend:
#   - the vendor kernel sources (the `FreeRTOS` / `RTThread` object library)
#   - the matching OSAL implementation (os_impl_*.c)
#   - the backend-specific include directories
#
# Layout:
#   04_Impl/impl_os/inc/                   OSAL internals shared by both
#                                          backends (osal_internal_*.h,
#                                          osal_macros.h, osal_config.h)
#   04_Impl/impl_os/src_freertos/          os_impl_*.c against FreeRTOS
#   04_Impl/impl_os/src_rtthread/          os_impl_*.c against RT-Thread
#   04_Impl/impl_os/00_RT_Thread_Kernel/   vendor kernel, RT-Thread
#   04_Impl/impl_os/01_FreeRTOS_Kernel/    vendor kernel, FreeRTOS
#
# Vendor trees hold vendor code only; project code never mixes into them.

set(APP_RTOS "FREERTOS" CACHE STRING "RTOS backend: FREERTOS or RTTHREAD")
set_property(CACHE APP_RTOS PROPERTY STRINGS FREERTOS RTTHREAD)
message(STATUS "RTOS backend: ${APP_RTOS}")

set(IMPL_OS_ROOT "${CMAKE_SOURCE_DIR}/04_Impl/impl_os")

if(APP_RTOS STREQUAL "FREERTOS")

    set(OS_KERNEL_ROOT "${IMPL_OS_ROOT}/01_FreeRTOS_Kernel")
    set(OS_IMPL_ROOT   "${IMPL_OS_ROOT}/src_freertos")

    set(OS_KERNEL_SOURCES
        "${OS_KERNEL_ROOT}/src/croutine.c"
        "${OS_KERNEL_ROOT}/src/event_groups.c"
        "${OS_KERNEL_ROOT}/src/list.c"
        "${OS_KERNEL_ROOT}/src/queue.c"
        "${OS_KERNEL_ROOT}/src/stream_buffer.c"
        "${OS_KERNEL_ROOT}/src/tasks.c"
        "${OS_KERNEL_ROOT}/src/timers.c"
        "${OS_KERNEL_ROOT}/src/cmsis_os2.c"
        "${OS_KERNEL_ROOT}/src/heap_4.c"
        "${OS_KERNEL_ROOT}/src/port.c"
    )

elseif(APP_RTOS STREQUAL "RTTHREAD")

    set(OS_KERNEL_ROOT "${IMPL_OS_ROOT}/00_RT_Thread_Kernel")
    set(OS_IMPL_ROOT   "${IMPL_OS_ROOT}/src_rtthread")

    # Vendored tree holds the whole 4.0.3 kernel; only the subset this project
    # configures for is compiled. Deliberately absent:
    #   components.c  RT_USING_USER_MAIN startup, replaced by osal_kernel_start()
    #   device.c      RT_USING_DEVICE is off (project has its own BSP layer)
    #   cpu.c         SMP only
    #   memheap.c slab.c   alternative allocators, RT_USING_SMALL_MEM is chosen
    #   signal.c      RT_USING_SIGNALS is off
    set(OS_KERNEL_SOURCES
        "${OS_KERNEL_ROOT}/src/clock.c"
        "${OS_KERNEL_ROOT}/src/idle.c"
        "${OS_KERNEL_ROOT}/src/ipc.c"
        "${OS_KERNEL_ROOT}/src/irq.c"
        "${OS_KERNEL_ROOT}/src/kservice.c"
        "${OS_KERNEL_ROOT}/src/mem.c"
        "${OS_KERNEL_ROOT}/src/object.c"
        "${OS_KERNEL_ROOT}/src/scheduler.c"
        "${OS_KERNEL_ROOT}/src/thread.c"
        "${OS_KERNEL_ROOT}/src/timer.c"
        "${OS_KERNEL_ROOT}/src/cpuport.c"
        "${OS_KERNEL_ROOT}/src/context_gcc.S"
    )

else()
    message(FATAL_ERROR "Unknown APP_RTOS '${APP_RTOS}' (expected FREERTOS or RTTHREAD)")
endif()

# OSAL implementation for the selected backend. The eight module names are the
# same for every backend; only the directory changes.
set(OS_IMPL_SOURCES
    "${OS_IMPL_ROOT}/os_impl_event_group.c"
    "${OS_IMPL_ROOT}/os_impl_heap.c"
    "${OS_IMPL_ROOT}/os_impl_mutex.c"
    "${OS_IMPL_ROOT}/os_impl_notify.c"
    "${OS_IMPL_ROOT}/os_impl_queue.c"
    "${OS_IMPL_ROOT}/os_impl_sema.c"
    "${OS_IMPL_ROOT}/os_impl_task.c"
    "${OS_IMPL_ROOT}/os_impl_timer.c"
)

# Include directories the rest of the firmware needs to see the selected
# backend. Consumed by cmake/app_sources.cmake via APP_INCLUDE_DIRS.
set(OS_KERNEL_INCLUDE_DIRS
    "${OS_KERNEL_ROOT}/inc"
    "${OS_IMPL_ROOT}"
)

# Kernel object library. The target name stays `FreeRTOS` regardless of
# backend so the existing references in the top-level CMakeLists keep working.
add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${OS_KERNEL_SOURCES})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)

# OSAL implementation compiles into the firmware target itself, as before.
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${OS_IMPL_SOURCES})

# Backend selector visible to osal_config.h, which derives OSAL_PRIORITY_MAX
# from it. Values match FREERTOS_SUPPORT / RTTHREAD_SUPPORT in that header.
if(APP_RTOS STREQUAL "RTTHREAD")
    set(OSAL_RTOS_SUPPORT_VALUE 2)
else()
    set(OSAL_RTOS_SUPPORT_VALUE 1)
endif()

foreach(os_target ${CMAKE_PROJECT_NAME} FreeRTOS)
    target_compile_definitions(${os_target} PRIVATE
        OSAL_RTOS_SUPPORT=${OSAL_RTOS_SUPPORT_VALUE})
endforeach()

if(APP_RTOS STREQUAL "RTTHREAD")
    # __RTTHREAD__ and HAVE_CCONFIG_H must be defined before any kernel header
    # is parsed; nothing #includes rtconfig_preinc.h explicitly.
    foreach(os_target ${CMAKE_PROJECT_NAME} FreeRTOS)
        target_compile_options(${os_target} PRIVATE
            -include "${OS_IMPL_ROOT}/rtconfig_preinc.h")
    endforeach()

    # context_gcc.S uses conditional instructions outside IT blocks; without
    # this the assembler rejects vstmdbeq / moveq / vldmiane / bicne. Note it
    # is -Wa,...: passing -mimplicit-it=thumb to gcc directly is not accepted.
    set_source_files_properties("${OS_KERNEL_ROOT}/src/context_gcc.S"
        PROPERTIES COMPILE_OPTIONS "-Wa,-mimplicit-it=thumb")
endif()
