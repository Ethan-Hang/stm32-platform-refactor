function(firmware_configure_tools)
    set(ASSETS_CFG "${CMAKE_SOURCE_DIR}/00_Config/inc/cfg_storage.h")
    set(ASSETS_LV_CONF "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/lv_conf.h")
    set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/images")
    set(ASSETS_FONT_DIR "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts")
    set(ASSETS_BIN "${CMAKE_SOURCE_DIR}/build/assets.bin")

    file(GLOB ASSETS_SRCS CONFIGURE_DEPENDS
        "${ASSETS_DIR}/*.c"
        "${ASSETS_FONT_DIR}/*.c"
    )

    add_custom_command(
        OUTPUT "${ASSETS_BIN}"
        COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}/Tools"
                ${UV_EXECUTABLE} run --no-project pack_assets.py
                --config "../00_Config/inc/cfg_storage.h"
                --lv-conf "../04_Impl/impl_middleware/lvgl/lvgl/lv_conf.h"
                --image-dir "../04_Impl/impl_middleware/lvgl/lvgl_ui/images"
                --font-dir "../04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts"
                --output "../build/assets.bin"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/Tools/pack_assets.py"
            "${ASSETS_CFG}"
            "${ASSETS_LV_CONF}"
            ${ASSETS_SRCS}
        COMMENT "Packing external LVGL assets"
        VERBATIM
    )
    add_custom_target(pack-assets DEPENDS "${ASSETS_BIN}")

    set(JFLASH_EXE "" CACHE FILEPATH "Optional JFlash executable override")
    option(FLASH_DRY_RUN "Print JFlash commands without programming hardware" OFF)

    set(FLASH_COMMON_ARGS)
    if(FLASH_DRY_RUN)
        list(APPEND FLASH_COMMON_ARGS --dry-run)
    endif()

    add_custom_target(flash-assets
        COMMAND ${CMAKE_COMMAND} -E env "JFLASH_EXE=${JFLASH_EXE}"
                ${UV_EXECUTABLE} run --no-project
                "${CMAKE_SOURCE_DIR}/Tools/flash.py"
                ${FLASH_COMMON_ARGS}
                assets
                --bin "${ASSETS_BIN}"
                --addr 0x90000000
                --jflash-prj "${CMAKE_SOURCE_DIR}/Tools/segger/jflash/411_w25q64.jflash"
        DEPENDS pack-assets
        VERBATIM
    )

    add_custom_target(download
        COMMAND ${CMAKE_COMMAND} -E env "JFLASH_EXE=${JFLASH_EXE}"
                ${UV_EXECUTABLE} run --no-project
                "${CMAKE_SOURCE_DIR}/Tools/flash.py"
                ${FLASH_COMMON_ARGS}
                download
                --build-dir "${FIRMWARE_OUTPUT_DIR}"
                --target "${FIRMWARE_BASENAME}"
                --jflash-prj "${CMAKE_SOURCE_DIR}/Tools/segger/jflash/stm32f411ce.jflash"
        DEPENDS firmware
        VERBATIM
    )

    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${ASSETS_BIN}")
endfunction()
