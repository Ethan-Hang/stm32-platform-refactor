function(app_configure_sources target)
    set(APP_SOURCES
        "${CMAKE_SOURCE_DIR}/01_App/User_Init/Platform_IO_Register/src/platform_io_register.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Init/user_init.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Isr_handlers/src/user_isr_handlers.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/display/src/lvgl_display_task.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/display/src/ui_hr_view.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/display/src/ui_temp_humi_view.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/em7028/src/em7028_heart_rate_task.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/mpu6050/src/mpu6050_unpack.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/storage/src/storage_assets.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/temp_humi/src/temp_humi_concurrency_test.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/touch/src/touch_calibration_boot.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Task_Config/src/task_higher_water_monitor.c"
        "${CMAKE_SOURCE_DIR}/01_App/User_Task_Config/src/user_task_reso_config.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/adapters/uart1_ota_transport.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/adapters/w25q64_ota_storage.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/src/firmware_upgrade_task.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/src/iwdg_feeder_task.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/src/ota_uart_listener.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/src/upgrade_service.c"
        "${CMAKE_SOURCE_DIR}/02_Service/service_storage/src/storage_manager_task.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/audio/bsp_wrapper_audio/src/bsp_wrapper_audio.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/display/bsp_wrapper_display/src/bsp_wrapper_display.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/externflash/bsp_wrapper_externflash/src/bsp_wrapper_externflash.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/heart_rate/bsp_wrapper_heart_rate/src/bsp_wrapper_heart_rate.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/motion/bsp_wrapper_motion/src/bsp_wrapper_motion.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/temp_humi/bsp_wrapper_temp_humi/src/bsp_wrapper_temp_humi.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/touch/bsp_wrapper_touch/src/bsp_wrapper_touch.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_middleware/platform_log/src/log_sink.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_event_group.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_heap.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_mutex.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_notify.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_queue.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_sema.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_task.c"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/src/osal_timer.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/audio/src/bsp_adapter_port_audio.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/display/src/bsp_adapter_port_display.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/externflash/src/bsp_adapter_port_externflash.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/heart_rate/src/bsp_adapter_port_heart_rate.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/motion/src/bsp_adapter_port_motion.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/temp_humi/src/bsp_adapter_port_temp_humi.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/touch/src/bsp_adapter_port_touch.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/aht21/handler/src/bsp_temp_humi_xxx_handler.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/cst816t/calibration/src/bsp_cst816t_calibration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/em7028/handler/src/bsp_em7028_handler.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/mpu6050/driver/src/circular_buffer.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/mpu6050/handler/src/bsp_mpuxxxx_handler.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/st7789/driver/src/fonts.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/w25q64/handler/src/bsp_w25q64_handler.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/wt588f02/driver/src/bsp_wt588_driver.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/wt588f02/driver/src/bsp_wt588_hal_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/wt588f02/handler/src/bsp_wt588_handler.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/wt588f02/handler/src/linklist.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/aht21_integration/src/aht21_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/cst816t_integration/src/cst816t_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/em7028_integration/src/em7028_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/mpu6050_integration/src/mpu6050_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/st7789_integration/src/st7789_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/w25q64_integration/src/w25q64_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/wt588_integration/src/wt588_integration.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_DWT/src/dwt_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_GPIO/src/gpio_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_IFlash/src/iflash_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_IIC/src/i2c_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_SPI/src/spi_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_Systick/src/systick_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_UART/src/mcu_uart_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_mcu/MCU_Core_Watchdog/src/watchdog_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/EasyLogger/port/elog_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/EasyLogger/src/elog.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/EasyLogger/src/elog_async.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/EasyLogger/src/elog_buf.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/EasyLogger/src/elog_utils.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/src/log.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/src/shell.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/src/shell_cmd_list.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/src/shell_companion.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/src/shell_ext.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/src/shell_port.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/Ymodem/src/ymodem.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/heart_rate_algo/src/hr_algo_biquad.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/heart_rate_algo/src/hr_algo_simple.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/log_adapters/log_adapter_easylogger/src/log_adapter_easylogger.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/log_adapters/log_sink_rtt/src/log_sink_rtt.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_disp.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_extflash.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_extfont.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_indev.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_mem_pool.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/analogclock/lv_analogclock.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/events_init.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/gui_guider.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_alimama_10.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_alimama_12.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_alimama_16.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_alimama_36.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_digitaldreamfatnarrow_36.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_interttf_10.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_interttf_16.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_interttf_24.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/lv_font_interttf_82.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Clock_1.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Clock_2.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Clock_3.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Error.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Heart.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_List_1.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_List_2.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_List_3.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Map.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_NFC.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_QRcode.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Set.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Systeamupdate.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_Systeamupdate_cheak.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_top_lap.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/setup_scr_under_up.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/touch_calibration/src/touch_calibration_ui.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/widgets_init.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_event_group.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_heap.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_mutex.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_notify.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_queue.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_sema.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_task.c"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/src/os_impl_timer.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Debug_Log/src/Debug.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/MPU_Protect/src/mpu.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/MPU_Protect/src/mpu_selftest.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/SWO_Trace/src/itm_trace.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/inc/SEGGER_RTT_ASM_ARMv7M.S"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/src/SEGGER_RTT.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/src/SEGGER_RTT_printf.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/src/SEGGER_SYSVIEW.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/src/SEGGER_SYSVIEW_Config_FreeRTOS.c"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/src/SEGGER_SYSVIEW_FreeRTOS.c"
        "${CMAKE_SOURCE_DIR}/Core/Src/iic_hal.c"
        "${CMAKE_SOURCE_DIR}/Core/Src/spi_hal.c"
    )

    set(LVGL_SOURCES)
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_CORE CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/core/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_CORE})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_DRAW CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/draw/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_DRAW})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_DRAW_SW CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/draw/sw/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_DRAW_SW})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_LAYOUTS_FLEX CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/layouts/flex/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_LAYOUTS_FLEX})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_LAYOUTS_GRID CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/layouts/grid/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_LAYOUTS_GRID})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_FRAGMENT CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/fragment/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_FRAGMENT})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_GRIDNAV CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/gridnav/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_GRIDNAV})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_IME CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/ime/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_IME})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_IMGFONT CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/imgfont/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_IMGFONT})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_MONKEY CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/monkey/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_MONKEY})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_MSG CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/msg/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_MSG})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_SNAPSHOT CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/others/snapshot/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_OTHERS_SNAPSHOT})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_THEMES_BASIC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/themes/basic/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_THEMES_BASIC})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_THEMES_DEFAULT CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/themes/default/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_THEMES_DEFAULT})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_THEMES_MONO CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/themes/mono/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_THEMES_MONO})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_ANIMIMG CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/animimg/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_ANIMIMG})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_CALENDAR CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/calendar/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_CALENDAR})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_CHART CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/chart/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_CHART})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_COLORWHEEL CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/colorwheel/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_COLORWHEEL})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_IMGBTN CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/imgbtn/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_IMGBTN})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_KEYBOARD CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/keyboard/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_KEYBOARD})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_LED CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/led/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_LED})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/list/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_LIST})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_MENU CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/menu/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_MENU})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_METER CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/meter/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_METER})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_MSGBOX CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/msgbox/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_MSGBOX})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_SPAN CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/span/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_SPAN})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_SPINBOX CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/spinbox/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_SPINBOX})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_SPINNER CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/spinner/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_SPINNER})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_TABVIEW CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/tabview/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_TABVIEW})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_TILEVIEW CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/tileview/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_TILEVIEW})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_WIN CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/extra/widgets/win/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_EXTRA_WIDGETS_WIN})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_FONT CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/font/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_FONT})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_HAL CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/hal/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_HAL})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_MISC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/misc/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_MISC})
    file(GLOB LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_WIDGETS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src/widgets/*.c")
    list(APPEND LVGL_SOURCES ${LVGL_04_IMPL_IMPL_MIDDLEWARE_LVGL_LVGL_SRC_WIDGETS})

    target_sources(${target} PRIVATE ${APP_SOURCES} ${LVGL_SOURCES})
    set(APP_INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_common/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_borad"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OSAL_Common/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_os/OS_Wrapper/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_IIC/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_SPI/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_UART/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_GPIO/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_Systick/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_DWT/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_Watchdog/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_mcu/MCU_Core_IFlash/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_middleware/platform_log/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/log_adapters/log_sink_rtt/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/log_adapters/log_adapter_easylogger/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/EasyLogger/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/LetterShell/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl/src"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_port/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/analogclock"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/touch_calibration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/guider_customer_fonts"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/lvgl/lvgl_ui/images"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/heart_rate_algo/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_middleware/Ymodem/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/aht21/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/aht21/handler/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/mpu6050/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/mpu6050/handler/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/wt588f02/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/wt588f02/handler/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/st7789/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/w25q64/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/w25q64/handler/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/cst816t/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/cst816t/calibration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/em7028/driver/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/em7028/handler/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/temp_humi/bsp_wrapper_temp_humi/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/temp_humi/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/motion/bsp_wrapper_motion/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/motion/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/audio/bsp_wrapper_audio/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/audio/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/display/bsp_wrapper_display/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/display/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/touch/bsp_wrapper_touch/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/touch/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/externflash/bsp_wrapper_externflash/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/externflash/inc"
        "${CMAKE_SOURCE_DIR}/03_Platform/platform_bsp/heart_rate/bsp_wrapper_heart_rate/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Adapter_Port/heart_rate/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/aht21_integration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/mpu6050_integration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/wt588_integration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/st7789_integration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/cst816t_integration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/w25q64_integration/inc"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Integration/em7028_integration/inc"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Systemview/inc"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/Debug_Log/inc"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/SWO_Trace/inc"
        "${CMAKE_SOURCE_DIR}/05_Debug_Tool/MPU_Protect/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Init"
        "${CMAKE_SOURCE_DIR}/01_App/User_Init/Platform_IO_Register/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Task_Config/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Isr_handlers/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/mpu6050/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/touch/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/em7028/inc"
        "${CMAKE_SOURCE_DIR}/01_App/User_Sensor/display/inc"
        "${CMAKE_SOURCE_DIR}/02_Service/service_storage/inc"
        "${CMAKE_SOURCE_DIR}/02_Service/service_ota/inc"
        "${CMAKE_SOURCE_DIR}/00_Config/inc"
        "${CMAKE_SOURCE_DIR}/Core/Inc"
        "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc"
        "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy"
        "${CMAKE_SOURCE_DIR}/04_Impl/impl_os/01_FreeRTOS_Kernel/inc"
        "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include"
        "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include"
    )

    target_include_directories(${target} PRIVATE ${APP_INCLUDE_DIRS})
    set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} PARENT_SCOPE)
endfunction()
