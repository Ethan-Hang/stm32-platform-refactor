cmake_minimum_required(VERSION 3.21)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME helloworld)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME} LANGUAGES C ASM)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Detect GCC version to decide linker-script READONLY keyword support.
# READONLY was added in binutils / GCC 11; older toolchains reject it.
if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "11")
    set(LD_READONLY "(READONLY)")
else()
    set(LD_READONLY "")
    message(STATUS "GCC ${CMAKE_C_COMPILER_VERSION} < 11 – READONLY removed from linker script")
endif()
set(GENERATED_LD "${CMAKE_BINARY_DIR}/STM32F411XX_FLASH.ld")
configure_file(
    "${CMAKE_SOURCE_DIR}/STM32F411XX_FLASH.ld"
    "${GENERATED_LD}"
    @ONLY
)

# Do not inherit CMake's host-oriented defaults (-O3/-DNDEBUG for Release).
# Target options below are the single source of optimization/debug flags.
foreach(language C ASM)
    set(CMAKE_${language}_FLAGS_DEBUG "" CACHE STRING "" FORCE)
    set(CMAKE_${language}_FLAGS_RELEASE "" CACHE STRING "" FORCE)
    set(CMAKE_${language}_FLAGS_CI-O3 "" CACHE STRING "" FORCE)
endforeach()

if(CMAKE_HOST_WIN32)
    find_program(UV_EXECUTABLE NAMES uv uv.exe)
else()
    # Only search for native uv; uv.exe via /mnt/c cannot handle Linux paths.
    find_program(UV_EXECUTABLE NAMES uv)
endif()
if(UV_EXECUTABLE)
    message(STATUS "uv found: ${UV_EXECUTABLE}")
    if(CMAKE_HOST_WIN32)
        file(TO_CMAKE_PATH "$ENV{LOCALAPPDATA}/uv/venvs/stm32-app-tools"
             APP_UV_PROJECT_ENVIRONMENT_DEFAULT)
    else()
        set(APP_UV_PROJECT_ENVIRONMENT_DEFAULT "${CMAKE_SOURCE_DIR}/Tools/.venv")
    endif()
    set(APP_UV_PROJECT_ENVIRONMENT "${APP_UV_PROJECT_ENVIRONMENT_DEFAULT}" CACHE PATH
        "Host-local uv environment for firmware build tools" FORCE)
else()
    message(STATUS "uv not found – tool targets (ota, mem-report, pack-assets, flash) disabled")
endif()
set(MEM_REPORT_FAIL_ABOVE "" CACHE STRING
    "Fail memory report when any linker region exceeds this fraction")

function(apply_firmware_compile_options target)
    target_compile_options(${target} PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -Wall
        -ffunction-sections
        -fdata-sections
        $<$<CONFIG:Debug>:-Og;-g;-gdwarf-2>
        $<$<CONFIG:Release>:-Os;-g0>
        $<$<STREQUAL:$<CONFIG>,CI-O3>:-O3;-g0>
    )
endfunction()

add_executable(${CMAKE_PROJECT_NAME})
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY C_STANDARD 11)

apply_firmware_compile_options(${CMAKE_PROJECT_NAME})

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    --specs=nano.specs
    -T${GENERATED_LD}
    -Wl,--gc-sections
    -Wl,--cref
    -u
    _printf_float
)


add_subdirectory(cmake/stm32cubemx)

# CubeMX adds DEBUG for Debug configurations, but Debug.h already owns that
# project macro. Keep the generated interface aligned with the legacy build.
set_property(TARGET stm32cubemx PROPERTY INTERFACE_COMPILE_DEFINITIONS
    USE_HAL_DRIVER
    STM32F411xE
)

# RTOS kernel (project-owned; moved out of the CubeMX-generated CMakeLists).
include(cmake/os_kernel.cmake)

include(cmake/app_sources.cmake)
app_configure_sources(${CMAKE_PROJECT_NAME})

apply_firmware_compile_options(STM32_Drivers)
apply_firmware_compile_options(FreeRTOS)
target_include_directories(STM32_Drivers PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(FreeRTOS PRIVATE ${APP_INCLUDE_DIRS})
# Same reason as the BSP driver libs: APP_INCLUDE_DIRS exposes the OSAL headers,
# so the backend selector has to come with them (cmake/os_kernel.cmake).
target_link_libraries(STM32_Drivers PUBLIC osal_backend)

include(cmake/bsp_driver_libs.cmake)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    stm32cubemx
    FreeRTOS
    -Wl,--whole-archive
    ${BSP_DRIVER_LIBS}
    -Wl,--no-whole-archive
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

include(cmake/firmware_artifacts.cmake)
firmware_configure_artifacts(${CMAKE_PROJECT_NAME})

if(UV_EXECUTABLE)
    include(cmake/firmware_tools.cmake)
    firmware_configure_tools()

    add_custom_target(verify-cmake-sources
        COMMAND ${UV_EXECUTABLE} run --no-project
                ${CMAKE_SOURCE_DIR}/Tools/verify_cmake_sources.py
                --expected-json ${CMAKE_SOURCE_DIR}/cmake/legacy_sources.json
                --compile-commands ${CMAKE_BINARY_DIR}/compile_commands.json
                --source-root ${CMAKE_SOURCE_DIR}
        DEPENDS ${CMAKE_PROJECT_NAME}
        VERBATIM
    )
endif()
