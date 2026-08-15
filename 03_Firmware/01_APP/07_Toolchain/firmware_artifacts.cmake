function(firmware_configure_artifacts target)
    set(FIRMWARE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build")

    # Stable artifacts carry the RTOS backend in their name. build/ is shared by
    # every configuration, so an unsuffixed helloworld.hex left you unable to
    # tell a FreeRTOS image from an RT-Thread one -- the two are wildly
    # different firmware. APP_RTOS comes from 07_Toolchain/os_kernel.cmake, which
    # CMakeLists.txt includes before calling this function.
    string(TOLOWER "${APP_RTOS}" _rtos_slug)
    set(FIRMWARE_BASENAME "helloworld-${_rtos_slug}")
    set(CONFIG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/artifacts")

    set(CONFIG_ELF "${CONFIG_OUTPUT_DIR}/${FIRMWARE_BASENAME}.elf")
    set(CONFIG_HEX "${CONFIG_OUTPUT_DIR}/${FIRMWARE_BASENAME}.hex")
    set(CONFIG_BIN "${CONFIG_OUTPUT_DIR}/${FIRMWARE_BASENAME}.bin")
    set(CONFIG_MAP "${CONFIG_OUTPUT_DIR}/${FIRMWARE_BASENAME}.map")
    set(CONFIG_OTA "${CONFIG_OUTPUT_DIR}/${FIRMWARE_BASENAME}.mxxx")

    set(FIRMWARE_ELF "${FIRMWARE_OUTPUT_DIR}/${FIRMWARE_BASENAME}.elf")
    set(FIRMWARE_HEX "${FIRMWARE_OUTPUT_DIR}/${FIRMWARE_BASENAME}.hex")
    set(FIRMWARE_BIN "${FIRMWARE_OUTPUT_DIR}/${FIRMWARE_BASENAME}.bin")
    set(FIRMWARE_MAP "${FIRMWARE_OUTPUT_DIR}/${FIRMWARE_BASENAME}.map")
    set(FIRMWARE_OTA "${FIRMWARE_OUTPUT_DIR}/${FIRMWARE_BASENAME}.mxxx")

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME ${FIRMWARE_BASENAME}
        RUNTIME_OUTPUT_DIRECTORY "${CONFIG_OUTPUT_DIR}"
    )
    target_link_options(${target} PRIVATE "-Wl,-Map=${CONFIG_MAP}")

    # --- HEX generation (normalize step requires uv) ---
    if(UV_EXECUTABLE)
        add_custom_command(
            OUTPUT "${CONFIG_HEX}"
            COMMAND ${CMAKE_OBJCOPY} -O ihex "${CONFIG_ELF}" "${CONFIG_HEX}"
            COMMAND ${UV_EXECUTABLE} run --no-project
                    "${CMAKE_SOURCE_DIR}/99_Utils/normalize_hex.py" "${CONFIG_HEX}"
            DEPENDS ${target} "${CMAKE_SOURCE_DIR}/99_Utils/normalize_hex.py"
            COMMENT "Generating ${FIRMWARE_BASENAME}.hex (normalized)"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT "${CONFIG_HEX}"
            COMMAND ${CMAKE_OBJCOPY} -O ihex "${CONFIG_ELF}" "${CONFIG_HEX}"
            DEPENDS ${target}
            COMMENT "Generating ${FIRMWARE_BASENAME}.hex"
            VERBATIM
        )
    endif()

    add_custom_command(
        OUTPUT "${CONFIG_BIN}"
        COMMAND ${CMAKE_OBJCOPY} -O binary -S "${CONFIG_ELF}" "${CONFIG_BIN}"
        DEPENDS ${target}
        COMMENT "Generating ${FIRMWARE_BASENAME}.bin"
        VERBATIM
    )

    add_custom_target(firmware-images
        DEPENDS "${CONFIG_HEX}" "${CONFIG_BIN}"
    )

    # --- OTA image (requires uv) ---
    if(UV_EXECUTABLE)
        add_custom_command(
            OUTPUT "${CONFIG_OTA}"
            COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}/99_Utils"
                    ${CMAKE_COMMAND} -E env
                    "UV_PROJECT_ENVIRONMENT=${APP_UV_PROJECT_ENVIRONMENT}"
                    ${UV_EXECUTABLE} run ota_encrypt.py
                    "${CONFIG_BIN}"
                    "${CONFIG_OTA}"
            DEPENDS
                "${CONFIG_BIN}"
                "${CMAKE_SOURCE_DIR}/99_Utils/ota_encrypt.py"
                "${CMAKE_SOURCE_DIR}/99_Utils/pyproject.toml"
            COMMENT "Generating encrypted OTA image"
            VERBATIM
        )
        add_custom_target(ota-image DEPENDS "${CONFIG_OTA}")
    endif()

    # --- Publish firmware artifacts ---
    if(UV_EXECUTABLE)
        add_custom_target(publish-firmware-artifacts
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FIRMWARE_OUTPUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_ELF}" "${FIRMWARE_ELF}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_HEX}" "${FIRMWARE_HEX}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_BIN}" "${FIRMWARE_BIN}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_MAP}" "${FIRMWARE_MAP}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_OTA}" "${FIRMWARE_OTA}"
            DEPENDS ${target} firmware-images ota-image
            COMMENT "Publishing ${CMAKE_BUILD_TYPE} firmware artifacts"
            VERBATIM
        )
    else()
        add_custom_target(publish-firmware-artifacts
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FIRMWARE_OUTPUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_ELF}" "${FIRMWARE_ELF}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_HEX}" "${FIRMWARE_HEX}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_BIN}" "${FIRMWARE_BIN}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONFIG_MAP}" "${FIRMWARE_MAP}"
            DEPENDS ${target} firmware-images
            COMMENT "Publishing ${CMAKE_BUILD_TYPE} firmware artifacts (OTA skipped – uv not available)"
            VERBATIM
        )
    endif()

    # --- Memory report (requires uv) ---
    if(UV_EXECUTABLE)
        set(MEM_REPORT_ARGS
            --elf "${CONFIG_ELF}"
            --ld "${GENERATED_LD}"
            --size-tool "${CMAKE_SIZE}"
        )
        if(NOT MEM_REPORT_FAIL_ABOVE STREQUAL "")
            list(APPEND MEM_REPORT_ARGS --fail-above "${MEM_REPORT_FAIL_ABOVE}")
        endif()

        add_custom_target(mem-report
            COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}/99_Utils"
                    ${CMAKE_COMMAND} -E env
                    "UV_PROJECT_ENVIRONMENT=${APP_UV_PROJECT_ENVIRONMENT}"
                    ${UV_EXECUTABLE} run mem_report.py ${MEM_REPORT_ARGS}
            DEPENDS ${target} firmware-images
                    "${CMAKE_SOURCE_DIR}/99_Utils/mem_report.py"
                    "${CMAKE_SOURCE_DIR}/07_Toolchain/STM32F411XX_FLASH.ld"
            COMMENT "Reporting linker memory-region usage"
            VERBATIM
        )
    endif()

    # --- Top-level firmware target ---
    add_custom_target(firmware ALL)
    set(_firmware_deps publish-firmware-artifacts)
    if(UV_EXECUTABLE)
        list(APPEND _firmware_deps mem-report)
    endif()
    add_dependencies(firmware ${_firmware_deps})

    set(_clean_files
        "${FIRMWARE_ELF}" "${FIRMWARE_HEX}" "${FIRMWARE_BIN}" "${FIRMWARE_MAP}"
    )
    if(UV_EXECUTABLE)
        list(APPEND _clean_files "${FIRMWARE_OTA}")
    endif()
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${_clean_files})

    set(FIRMWARE_OUTPUT_DIR "${FIRMWARE_OUTPUT_DIR}" PARENT_SCOPE)
    set(FIRMWARE_BASENAME "${FIRMWARE_BASENAME}" PARENT_SCOPE)
    set(FIRMWARE_HEX "${FIRMWARE_HEX}" PARENT_SCOPE)
    set(FIRMWARE_BIN "${FIRMWARE_BIN}" PARENT_SCOPE)
endfunction()
