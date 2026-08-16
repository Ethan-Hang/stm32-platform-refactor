# 06_Vendor — CubeMX 生成物

STM32CubeMX 的工程根。`helloworld.ioc` 就在本目录，CubeMX **相对 `.ioc` 所在目录**生成代码，所以点 GENERATE CODE 产出的 `Core/` / `Drivers/` / `cmake/stm32cubemx/` 都落在这里，永远不会漏回 `01_APP/` 根目录。

| 条目 | 内容 |
|---|---|
| `helloworld.ioc` | CubeMX 工程文件（`TargetToolchain=CMake`，`UnderRoot=false`） |
| `.mxproject` | CubeMX 上次生成的文件清单，由 CubeMX 维护 |
| `Core/Inc`、`Core/Src` | 外设初始化、中断向量、HAL MSP、`FreeRTOSConfig.h`、`stm32f4xx_hal_conf.h`、引脚宏（`main.h`） |
| `Core/Startup/startup_stm32f411ceux.s` | STM32CubeIDE 时期的启动文件，**当前未参与编译** |
| `startup_stm32f411xe.s` | 实际编译的启动文件（由 `cmake/stm32cubemx/CMakeLists.txt` 引用） |
| `Drivers/STM32F4xx_HAL_Driver` | ST HAL |
| `Drivers/CMSIS` | ARM CMSIS + STM32F4xx 设备头 |
| `cmake/stm32cubemx/CMakeLists.txt` | CubeMX 生成的源集/包含路径，**由 CubeMX 拥有，会被覆盖** |

## 两条红线

**1. 本目录不是纯生成物，不能整体删除后让 CubeMX 重生。**

`Core/Src/iic_hal.c`、`Core/Src/spi_hal.c` 及对应的 `Core/Inc/iic_hal.h`、`Core/Inc/spi_hal.h` 是**手写**的软件 I2C（SCL=PB14 / SDA=PB15）与软件 SPI 位操作实现，CubeMX 不认识它们。唯一调用方是 `04_Impl/impl_mcu/MCU_Core_IIC/src/i2c_port.c` 与 `04_Impl/impl_mcu/MCU_Core_SPI/src/spi_port.c`，源文件登记在 `07_Toolchain/app_sources.cmake`。

删库重生会静默丢掉这四个文件，表现为软件 I2C / 软件 SPI 链路链接失败。

**2. CubeMX 会在本目录多吐两个废弃文件。**

`ProjectManager.TargetToolchain=CMake` 让 CubeMX 每次生成都在 `.ioc` 旁边写一份它自己的 `CMakeLists.txt` 和 `CMakePresets.json`。它们**不是**本工程的构建入口，已由 `01_APP/.gitignore` 屏蔽。真正的入口是 `01_APP/CMakeLists.txt`，它只 `add_subdirectory(06_Vendor/cmake/stm32cubemx)`。

## 与项目其余部分的关系

- RTOS 内核**不在这里**。FreeRTOS / RT-Thread 内核树在 `04_Impl/impl_os/{01_FreeRTOS_Kernel,00_RT_Thread_Kernel}/`，由 `07_Toolchain/os_kernel.cmake` 按 `APP_RTOS` 选择。CubeMX 生成的 `Core/Src/freertos.c` 已被 `04_Impl/impl_os/src_*/os_impl_kernel.c` 取代。
- 链接脚本**不在这里**，在 `07_Toolchain/STM32F411XX_FLASH.ld`。
- `cmake/stm32cubemx/CMakeLists.txt` 里的相对路径全部形如 `${CMAKE_CURRENT_SOURCE_DIR}/../../Core/…`，解析到 `06_Vendor/` 自身，因此该文件被 CubeMX 覆盖后依然正确，无需手动修补。
- 改 MCU 外设配线（USART / SPI / I2C / DMA / IRQ）应当先用 CubeMX 重新生成，而不是直接手改 `Core/Src/` 里的生成代码。
