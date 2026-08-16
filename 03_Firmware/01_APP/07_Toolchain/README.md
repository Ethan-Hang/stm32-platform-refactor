# 07_Toolchain — 构建与烧录工具链

"**怎么编、怎么烧**"的全部定义。业务源码一行不在这里。

顶层 `CMakeLists.txt` 与 `CMakePresets.json` **不在**本目录 —— CMake 要求 presets 文件与顶层 `CMakeLists.txt` 同目录，两者只能留在 `01_APP/` 根。根 `CMakeLists.txt` 通过 `include(07_Toolchain/*.cmake)` 把本目录的模块拉进来。

## 内容

| 文件 | 职责 |
|---|---|
| `gcc-arm-none-eabi.cmake` | CMake toolchain 文件，由 `CMakePresets.json` 的 `toolchainFile` 引用。改交叉编译器从这里改 |
| `starm-clang.cmake` | 备用 clang toolchain（当前 preset 未使用） |
| `STM32F411XX_FLASH.ld` | 链接脚本模板。根 `CMakeLists.txt` 用 `configure_file(@ONLY)` 展开 `@LD_READONLY@` 后落到 `${CMAKE_BINARY_DIR}/STM32F411XX_FLASH.ld`，实际链接用的是展开后那份 |
| `os_kernel.cmake` | 按 `APP_RTOS`（`RTTHREAD` / `FREERTOS`）派生内核源集、include 路径、`-DOSAL_RTOS_SUPPORT`，并提供 `osal_backend` INTERFACE 库 |
| `app_sources.cmake` | 业务源文件与 include 目录登记入口。**新增 `.c` 写这里** |
| `bsp_driver_libs.cmake` | 6 个 BSP 驱动静态库（`libbsp_{aht21,cst816t,em7028,mpuxxxx,st7789,w25q64}_driver.a`）的构建定义 |
| `firmware_artifacts.cmake` | elf/hex/bin/map/mxxx 产物生成与发布，以及 `mem-report` |
| `firmware_tools.cmake` | `pack-assets` / `flash-assets` / `download` 三个工具目标 |
| `legacy_sources.json` | `verify-cmake-sources` 的期望源集快照 |
| `flash_algorithm/W25Q64_8M_FLM.FLM` | 自定义 JLink Flash 算法（见下节） |
| `lvgl-assets-external-flash.md` | 外部 Flash LVGL 资源完整指南（地址体系、数据通路、新增图片/字体步骤、排查） |

CubeMX 生成的 `cmake/stm32cubemx/CMakeLists.txt` **不在**这里，它在 `06_Vendor/cmake/stm32cubemx/`，因为它由 CubeMX 拥有并会被覆盖。

> `legacy_sources.json` 目前是 FreeRTOS 时期的快照，`verify-cmake-sources` 在两个后端下都会报 missing/unexpected（RT-Thread 后端差异尤其大）。这是既有状态，该目标当前只适合做**差分**比较（改动前后输出是否一致），不能当通过/失败门。

## W25Q64 Flash 算法（`flash_algorithm/W25Q64_8M_FLM.FLM`）

本板适配版二进制，Keil MDK 源码工程未纳入本仓库，`*.FLM` 是唯一交付物。

它把 W25Q64 的 SPI bank 挂到 JLink 虚拟地址 `0x90000000`，内部重映射为

```
W25Q64 物理地址 = JLink 地址 - 0x90000000 + 0x300000
```

`0x300000` 是 LVGL 分区起点，且 FLM 的可寻址范围被锁死在该分区内 —— 因此 `JFlash` / `Ozone` 工具链**只能**写 LVGL 分区，无法误伤 OTA / FlashDB / FATFS / Reserved。`cmake --build --preset Debug --target flash-assets` 走的就是它。

### 部署（必须手动做一次）

1. 把 `W25Q64_8M_FLM.FLM` 复制到 `%APPDATA%\SEGGER\JLinkDevices\ST\STM32F4\`
2. 在同目录 `Devices.xml` 里注册自定义设备 `STM32F411CE_W25Q64`

注意：`%APPDATA%` 下那份是**独立副本**，本仓库内移动 FLM 不会自动更新它。换了 FLM 二进制要手动重新复制。

## 改动红线

- **`STM32F411XX_FLASH.ld` 的 RAM / RTT_RAM 边界与 Bootloader 联动**。RTT 控制块必须与 `00_Bootloader` 落在同一物理地址 `0x2001E400`，否则 J-Link RTT Viewer 无法同时看到两个镜像的日志。改这里要同步 `00_Bootloader/STM32F411XX_BOOTLOADER_FLASH.ld` 及两份 `SEGGER_RTT_Conf.h`
- **新增静态库必须 `target_link_libraries(<lib> PRIVATE osal_backend)`**。`osal_common_types.h` 缺 `OSAL_RTOS_SUPPORT` 宏会直接 `#error`
- **改 FLM 的地址重映射，要同步核对** `00_Config/inc/cfg_storage.h` 的 `MEMORY_LVGL_START_ADDRESS` 与 `99_Utils/pack_assets.py`
