# Create static libraries (.a) for the 6 core BSP peripheral drivers
set(BSP_DRIVER_LIBS
    bsp_aht21_driver
    bsp_cst816t_driver
    bsp_em7028_driver
    bsp_mpuxxxx_driver
    bsp_st7789_driver
    bsp_w25q64_driver
)

add_library(bsp_aht21_driver STATIC "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/aht21/driver/src/bsp_aht21_driver.c")
add_library(bsp_cst816t_driver STATIC "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/cst816t/driver/src/bsp_cst816t_driver.c")
add_library(bsp_em7028_driver STATIC "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/em7028/driver/src/bsp_em7028_driver.c")
add_library(bsp_mpuxxxx_driver STATIC "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/mpu6050/driver/src/bsp_mpuxxxx_driver.c")
add_library(bsp_st7789_driver STATIC "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/st7789/driver/src/bsp_st7789_driver.c")
add_library(bsp_w25q64_driver STATIC "${CMAKE_SOURCE_DIR}/04_Impl/impl_bsp/Bsp_Drivers/w25q64/driver/src/bsp_w25q64_driver.c")

foreach(lib ${BSP_DRIVER_LIBS})
    apply_firmware_compile_options(${lib})
    target_include_directories(${lib} PUBLIC ${APP_INCLUDE_DIRS})
    # APP_INCLUDE_DIRS puts the OSAL headers on the search path, so these libs
    # need the backend selector too -- see 07_Toolchain/os_kernel.cmake. Drivers are
    # not supposed to call OSAL at all, but the include path lets them, and
    # osal_common_types.h #errors rather than guessing a backend.
    target_link_libraries(${lib} PRIVATE osal_backend)
endforeach()
