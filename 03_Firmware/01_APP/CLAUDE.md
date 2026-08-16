# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 构建命令

```bash
cmake --preset Debug
cmake --build --preset Debug --parallel 16             # 默认完整构建：elf/hex/bin/map/mxxx + mem-report
cmake --build --preset Debug --target clean
cmake --build --preset Debug --target mem-report
cmake --build --preset Debug --target ota-image

cmake --build --preset Debug --target download          # 先增量构建最新固件，再由 JFlash 烧 APP 槽
cmake --build --preset Debug --target pack-assets       # 仅打包 LVGL 资源 → build/assets.bin
cmake --build --preset Debug --target flash-assets      # pack + JFlash 烧 W25Q64 LVGL 分区

cmake --preset Release
cmake --build --preset Release --parallel 16            # -Os 发布配置
```

### RTOS 后端选择（FreeRTOS / RT-Thread）

后端由 **preset 名字**决定，一一对应，不需要记 `-D` 参数：

| preset | 后端 | 优化 | 稳定产物 |
|---|---|---|---|
| `Debug` / `Release` / `CI-O3` | RT-Thread（默认） | `-Og` / `-Os` / `-O3` | `build/helloworld-rtthread.*` |
| `Debug-FreeRTOS` / `Release-FreeRTOS` / `CI-O3-FreeRTOS` | FreeRTOS | 同上 | `build/helloworld-freertos.*` |

```bash
cmake --list-presets                                    # 列出全部（displayName 标注后端）
cmake --preset Debug-FreeRTOS && cmake --build --preset Debug-FreeRTOS --parallel 16
```

每个 preset 有独立 `binaryDir`（`build/<presetName>`），两个后端可以并存、互不覆盖。preset 显式把 `APP_RTOS` 写进 cache，所以不存在"上次配过 FreeRTOS、这次 `--preset Debug` 还是 FreeRTOS"的粘滞。裸 CMake 仍可 `cmake -B dir -DAPP_RTOS=FREERTOS`。

后端差异全部由 `07_Toolchain/os_kernel.cmake` 里的 `APP_RTOS` 派生：内核源文件集、`04_Impl/impl_os/src_{freertos,rtthread}/`、include 路径、`-DOSAL_RTOS_SUPPORT=1|2`、RT-Thread 的 `-include rtconfig_preinc.h`、以及 `07_Toolchain/app_sources.cmake` 里的源集裁剪（LetterShell 与 SystemView 的 FreeRTOS 版仅在 FreeRTOS 构建中编译）。

`OSAL_RTOS_SUPPORT` 由 `osal_backend` INTERFACE 库携带，**任何能 include 到 OSAL 头文件的 target 都必须 link 它**——`osal_common_types.h` 缺少该宏时直接 `#error`，不再静默默认成 FreeRTOS。新增静态库时记得 `target_link_libraries(<lib> PRIVATE osal_backend)`。

CMake/Ninja 中间文件位于 `build/<presetName>`；稳定产物为 `build/helloworld-<backend>.*`（`rtthread` / `freertos`），按后端区分、不互相覆盖。同后端的 Debug 与 Release 仍共享同一条稳定产物路径，不要并发构建。新增项目源文件维护 `07_Toolchain/app_sources.cmake`；`06_Vendor/cmake/stm32cubemx/CMakeLists.txt` 由 CubeMX 管理。

目标芯片：STM32F411xE（Cortex-M4F），工具链：`arm-none-eabi-gcc`，默认编译选项：`-Og -g -gdwarf-2`。

无正式测试框架。应用级验证任务位于 `01_App/User_Sensor/`，在目标硬件上运行（如 `temp_humi_test_task_a/b` 用于并发读取验证）。

## 架构总览

严格的分层依赖——上层只依赖下层：

```
00_Config              ← 项目级宏开关、地址、状态字
01_App                 ← 业务逻辑、任务定义、传感器测试
02_Service             ← OTA / storage 等业务无关服务
03_Platform            ← OS / BSP / MCU / Middleware / Common 稳定接口
04_Impl                ← OS / BSP / MCU / Middleware 具体实现
05_Debug_Tool          ← 日志、追踪、MPU 保护
06_Vendor              ← CubeMX 生成物（Core / Drivers / startup / .ioc）
ARM CMSIS / 硬件        ← 寄存器级
```

不参与运行时分层、但属于工程的两个目录：

| 目录 | 内容 |
|---|---|
| `07_Toolchain/` | 怎么编、怎么烧：toolchain 文件、全部 CMake 模块、链接脚本、W25Q64 的 JLink `.FLM`、LVGL 资源指南。详见 [07_Toolchain/README.md](07_Toolchain/README.md) |
| `99_Utils/` | 构建/烧录辅助脚本（`pack_assets.py`、`mem_report.py`、`ota_encrypt.py`、`flash.py`…）、uv 环境、SEGGER/OpenOCD 配置、测试 |

顶层只剩 `CMakeLists.txt` 与 `CMakePresets.json` 两个散文件——CMake 要求 presets 与顶层 `CMakeLists.txt` 同目录，它们无法收进 `07_Toolchain/`。

### 平台层（`03_Platform/` + `04_Impl/`）

| 目录 | 职责 |
|---|---|
| `03_Platform/platform_os/` + `04_Impl/impl_os/` | 对 FreeRTOS 的 OSAL 封装。`OS_Wrapper/inc/` 是公开 API；`04_Impl/impl_os/` 包含 FreeRTOS 映射实现。替换 `impl_os` 即可切换 RTOS。 |
| `03_Platform/platform_bsp/` + `04_Impl/impl_bsp/` | 使用适配器模式的传感器/外设驱动（详见下文）。Wrapper API 在 platform，具体 driver/handler/adapter 在 impl。 |
| `03_Platform/platform_mcu/` + `04_Impl/impl_mcu/` | 芯片级 port 层：`MCU_Core_IIC/SPI/GPIO/Systick/DWT/UART` 总线与时钟、`MCU_Core_Watchdog/`（`mcu_watchdog_refresh()`）、`MCU_Core_IFlash/`（`mcu_iflash_erase_sector + program_words`，函数体内 `__disable_irq()` 包裹整段防 F411 单 bank flash 取指死锁）。所有寄存器级触碰都收在此层，service / APP 不直接调 HAL。同时支持硬件 I2C（HAL）和软件 I2C 位操作（SCL=PB14，SDA=PB15）。 |
| `03_Platform/platform_middleware/` + `04_Impl/impl_middleware/` | 当前 middleware 接口层为空，占位给 v5；EasyLogger、LetterShell、Ymodem、LVGL v8.3、heart_rate_algo 在 `04_Impl/impl_middleware/`。 |
| `03_Platform/platform_common/`（+ `04_Impl/impl_borad/`） | 跨层公共词汇：`platform_type.h`（全大写 `_T` 定宽类型 `UINT8_T`/`INT32_T`/`FLOAT32_T`/`BOOL_T`/`SIZE_T`…，底层映射 `impl_borad/board_types.h`）、`platform_error.h`（`platform_err_t` + `PLATFORM_IS_OK/ERR`）、`platform_def.h`（`ARRAY_SIZE`/`PLATFORM_UNUSED`/`PLATFORM_MIN/MAX`/`PLATFORM_ALIGN`）。换工具链/板只改 `board_types.h`。 |

### 类型与错误码约定（platform_common）

- **错误码**：可失败函数返回 `platform_err_t`，调用方用 `PLATFORM_IS_OK/ERR` 判定——不要返回裸 `0/-1` 或新造状态码。例外：service 对外的 `ota_transport_status_t`/`ota_storage_status_t`/`ext_flash_status_t` 三个边界枚举保留，adapter 内 `translate()` 与 `platform_err_t` 互转（值映射，禁止强转）。
- **定宽类型**：`01_App` / `02_Service` 及 `03_Platform/platform_bsp` 的 `bsp_wrapper_*`（vtable 接口层）用 `platform_type.h` 的全大写 `_T` 词汇（`UINT8_T`/`INT32_T`/`BOOL_T`/`SIZE_T`/`FLOAT32_T`…），不直接 `#include <stdint.h>`；纯文本仍用 `char`。
- **分层现状**：`_T` 已落地于 `01_App`/`02_Service` + `03_Platform/platform_bsp` wrapper；`04_Impl`（BSP driver/handler/adapter/integration）、MCU port（`03_Platform`/`04_Impl`）、`00_Config` 仍用 raw stdint（分层迁移未回填）。改下层时沿用该层现状，勿混用。

### BSP 适配器模式（`03_Platform/platform_bsp/` + `04_Impl/impl_bsp/`）

每个外设遵循以下三层结构：

```
03_Platform/platform_bsp/<category>/bsp_wrapper_<cat>/
                                ← 向 01_App / 02_Service 暴露的抽象 vtable API
04_Impl/impl_bsp/Bsp_Drivers/<device>/driver/
                                ← 原始寄存器/协议通信，禁止调用 OSAL
04_Impl/impl_bsp/Bsp_Drivers/<device>/handler/
                                ← Handler 线程逻辑（读取驱动，投递到队列）
04_Impl/impl_bsp/Adapter_Port/<category>/
                                ← 将具体驱动注册到 wrapper vtable
04_Impl/impl_bsp/Bsp_Integration/<device>_integration/
                                ← 将驱动+OS 资源组装为 input_arg 结构体
```

`bsp_wrapper_*` 头文件定义公开 API 及 vtable 结构体。`Adapter_Port/<category>/` 头文件仅暴露 `drv_adapter_<cat>_register()`。集成层负责组装传递给 handler 线程的 `*_input_arg` 结构体。

Wrapper API 风格按设备类型选择：

| 风格 | 适用 | 形态 |
|---|---|---|
| 请求-响应（sync/async） | 单次按需读取的传感器，如 `temp_humi` | `*_read_*_sync(life_time)` 阻塞，`*_read_*_async(cb, life_time)` 回调 |
| 流式（streaming） | 持续产帧的传感器，如 `motion`、`heart_rate` | `*_drv_get_req(timeout) → *_get_data_addr() → *_read_data_done()`；`heart_rate` 额外提供 `start/stop/reconfigure` lifecycle |

现有设备：`aht21`（温湿度）、`mpu6050`（运动）、`wt588f02`（音频）、`st7789`（LCD 显示）、`cst816t`（触摸屏）、`w25q64`（外部 SPI NOR，五段已完成，承载 LVGL 资源分区）、`em7028`（PPG 心率，category=`heart_rate`，流式 + lifecycle，frame 类型 `wp_ppg_frame_t`）。

### 应用层（`01_App/`）

- **任务表**：`User_Task_Config/src/user_task_reso_config.c` — 定义 `g_user_task_cfg[]`，包含所有应用任务的名称、栈大小、优先级、入口函数和参数。
- **任务优先级**（定义于 `user_task_reso_config.h`）：`PRI_EMERGENCY`、`PRI_HARD_REALTIME`、`PRI_SOFT_REALTIME`、`PRI_NORMAL`、`PRI_BACKGROUND`。
- **任务创建**：`User_Init/user_init.c` 遍历 `g_user_task_cfg[]`，逐条调用 `osal_task_create()`；失败时回滚。
- **IO 注册**：`User_Init/Platform_IO_Register/` — 启动时将硬件 IO 绑定到驱动适配器。
- **ISR 派发**：`User_Isr_handlers/` — ISR 通过 OSAL notify 唤醒任务，避免在中断上下文中阻塞（防止 IIC 互斥锁死锁）。
- **栈水位监控**：`User_Task_Config/src/task_higher_water_monitor.c` — 运行时任务栈占用监控。

### 服务层（`02_Service/`）

业务无关 service 抽象，与 `01_App/` 平级。Service 只能向下调 `03_Platform/` / `04_Impl/` 的公开接口和 `00_Config/`，**禁止反向依赖 `01_App/` 任何代码**。

| 子模块 | 职责 |
|---|---|
| `service_storage/` | 异步 BSP externflash 上的阻塞门面：`Read_/Write_LvglData`、`Read_/Write_OtaData` + `storage_manager_task`。APP（LVGL 资源）与 `service_ota`（Ymodem staging）共享同一条单消费者队列。 |
| `service_ota/` | OTA 升级链路（详见 "OTA 升级链路" 节）。`ota_flag_read/write` 经 `MCU_Core_IFlash` 写内部 Flash；`iwdg_feeder_task` 经 `MCU_Core_Watchdog` 喂狗。 |

后续 FOTA、配网、电池策略等 service 都进这一层。

### OSAL 层（`03_Platform/platform_os/`）

`OSAL_Common/inc/osal_common_types.h` 定义项目全局共用类型：`osal_task_handle_t`、`osal_queue_handle_t`、`osal_mutex_handle_t`、`osal_tick_type_t` 等。始终通过 `osal_wrapper_adapter.h` 包含。

### 配置层（`00_Config/`）

用于项目级宏开关（`CFG_` 前缀）：功能特性开关、板级 IO 映射、RTOS 资源大小。当前包含 `cfg_storage.h`（W25Q64 LVGL 子分区 magic + 资源 offset/size）和 `cfg_ota.h`（OTA flag 结构 + magic + 状态宏，与 bootloader `Tasks/Bootmanager/inc/ota_flag.h` 必须保持字节兼容）。

### 调试工具（`05_Debug_Tool/`）

#### 日志系统（`Debug.h`，`DEBUG_OUT` 宏）

两路输出路径同时有效，按 tag 选择路由：

- **RTT 路径**（默认）：经由 EasyLogger → `SEGGER_RTT_SetTerminal()` → RTT 物理通道 0，在 J-Link RTT Viewer 中按 Terminal Tab 分组显示。
- **ITM/SWO 路径**：`debug_is_itm_tag()` 中列出的 tag 绕过 EasyLogger，通过 `printf()` → `__io_putchar()` → ITM stimulus port 0 输出，在 JLink SWO Viewer 或 Ozone SWO 终端中可见。

RTT Terminal 分组（`DEBUG_RTT_CH_*`）：

| Terminal | 常量 | 覆盖的 tag |
|---|---|---|
| 0 | `DEBUG_RTT_CH_DEFAULT` | 所有未显式路由的 tag |
| 1 | `DEBUG_RTT_CH_SENSOR0` | AHT21 / 温湿度相关 |
| 2 | `DEBUG_RTT_CH_SENSOR1` | WT588 handler / 测试 |
| 3 | `DEBUG_RTT_CH_SENSOR2` | MPU6050 / 数据解析 |
| 4 | `DEBUG_RTT_CH_DISPLAY` | ST7789 TFT-LCD、`LVGL`（LVGL 自身日志）、`LVGL_MEM`（池快照）、`MPU`/`MPU_ERR` |
| 5 | `DEBUG_RTT_CH_TOUCH`   | CST816T 触摸 |
| 6 | `DEBUG_RTT_CH_STORAGE` | W25Q64 SPI NOR Flash |
| 7 | `DEBUG_RTT_CH_PPG`     | EM7028 PPG 心率 |
| 8 | `DEBUG_RTT_CH_STACK`   | 栈水位监控 |

tag 路由由 `Debug.c` 的 `s_route_table[]` 单表驱动，`debug_route_lookup()` 一趟线性扫描同时回答"是否输出 / 走哪个 RTT 终端 / 走不走 ITM"。表里没有的 tag 自动丢弃。

新增 RTT tag 步骤：
1. 在 `Debug.h` 中定义 `*_LOG_TAG` 字符串常量。
2. 在 `s_route_table[]` 加一行 `{ TAG, DEBUG_RTT_CH_x, DEBUG_ROUTE_RTT }`（`DEBUG_RTT_CH_DEFAULT` 即终端 0）。

新增 ITM-only tag 步骤：
1. 在 `Debug.h` 中定义 `*_ITM_LOG_TAG` 常量。
2. 在 `s_route_table[]` 加一行 `{ TAG, 0, DEBUG_ROUTE_ITM }`。无需修改 `elog_port.c` 或 RTT 配置。

停用某 tag：从 `s_route_table[]` 删除（或注释）该行即可，不再需要在三处分别维护。

#### SEGGER SystemView

实时 OS 追踪，通过 RTT 传输。`RTT_RAM` 区域 7 KB（链接脚本，地址 `0x2001E400`，与 Bootloader 共址），其中 SystemView 上行 buffer 占 4 KB、调试日志 channel 0 上行 buffer 占 2 KB（`BUFFER_SIZE_UP`）。

## 调试工作流

| 操作 | 工具 |
|---|---|
| 烧录固件 | SEGGER JFlash — 打开 `build/helloworld-<backend>.hex`，目标 STM32F411xE |
| 源码调试 | SEGGER Ozone — 加载 `build/helloworld-<backend>.elf`，通过 JLink 连接 |
| OS 任务追踪 | SEGGER SystemView — 通过 JLink 附加到运行中的目标 |
| printf 日志 | JLink RTT Viewer — 通道 0，1000000 波特率 |
| SWO 输出 | JLink SWO Viewer 或 Ozone SWO 终端，波特率由 `itm_trace_init(cpu_hz, swo_hz)` 配置 |

典型流程：JFlash 烧录 → Ozone 断点/监视 → SystemView RTOS 时序 → RTT Viewer 日志输出。

## 硬件

- **MCU**：STM32F411xE — Cortex-M4F，512KB FLASH，128KB SRAM
- **RTOS**：FreeRTOS v10.3.1，heap_4，16 KB 堆（`configTOTAL_HEAP_SIZE`）。**全部生产任务静态栈/TCB**（`g_user_task_cfg[]` 条目带 `OSAL_TASK_ALLOC_STATIC` + `OSAL_TASK_STATIC_DEFINE` 存储，新增任务默认也走静态），ucHeap 只剩启动期队列/信号量/互斥锁/定时器（推算峰值 ~11.7 KB）。`task_higher_water_monitor` 每秒打印堆 free/min_ever 兜底，min_ever 逼近 0 按 4 KB 步进回调，1 kHz tick，CMSIS-RTOS V2 API 可用
- **FPU**：单精度硬浮点（`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`）
- **链接脚本**：`07_Toolchain/STM32F411XX_FLASH.ld` — 121 KB 用户 RAM (`RAM`, `0x20000000`) + 7 KB RTT RAM (`RTT_RAM`, `0x2001E400`)

### 关键引脚分配（`06_Vendor/Core/Inc/main.h`）

| 信号 | 引脚 |
|---|---|
| 软件 I2C SCL | PB14 |
| 软件 I2C SDA | PB15 |
| SPI1 CS/RST/DC | PA3/PA4/PA6 |
| 软件 SPI SCK/MISO/MOSI | PA5/PA6/PA7 |
| **SPI2 SCK / MISO / MOSI（W25Q64）** | **PB10 / PB14 / PB15** |
| **SPI2 CS（W25Q64）** | **PB13** |
| WT588 busy | PA12 |
| 触摸屏中断 TP_TINT | PB0（EXTI0，gpio.c 已配 IT_RISING 但 `EXTI0_IRQn` 未 enable、`it.c` 无 `EXTI0_IRQHandler` → 实际仍轮询） |

## 外部 Flash LVGL 资源（W25Q64）

> 完整指南（地址体系、数据通路、新增图片/字体步骤、GUI Guider 重导出流程、故障排查）见 [07_Toolchain/lvgl-assets-external-flash.md](07_Toolchain/lvgl-assets-external-flash.md)，本节是速览。

UI 的**全部 41 张图片 + 9 套自定义字体的字形位图**托管在 W25Q64 上（固件 `.rodata` 不含任何像素/字形数据），省下内部 Flash 容纳 16 屏 GUI Guider UI + 业务代码。资源走两条独立路径，互不干涉：改 firmware 走 `cmake --build --preset Debug`，改图/字体走 `cmake --build --preset Debug --target flash-assets`。**固件和资产包必须配对烧录**：资产布局/字节序变更会 bump `CFG_LVGL_ASSET_MAGIC`，启动时 magic 失配只打 RTT 警告（UI 照常启动，图片空白、文字缺字形，不死机）。

**字节序契约**：`LV_COLOR_16_SWAP = 1`——LVGL 直接渲染面板字节序（大端 RGB565），flush 经 `display_flush_async` 零拷贝单段 DMA 直发 ST7789（双 20 行缓冲，渲染与传输并行，完成回调从 SPI TX-DMA 中断调 `lv_disp_flush_ready`）；资产包同样按 swap 分支打包（改 swap 必须重打包重烧）。旧的 `display_draw_image` 保持主机字节序契约（驱动内逐像素交换），仅供非 LVGL 调用方。

### 三套地址空间（关键）

| 视角 | LVGL 起点 | 大小 | 谁用 |
|---|---|---|---|
| LVGL local（软件层） | `0x000000` | 3 MB | `Read_LvglData(addr,...)` 接口 |
| W25Q64 物理 | `0x300000` | 3 MB | SPI2 驱动直接寻址 |
| JLink 虚拟（FLM） | `0x90000000` | 3 MB | JFlash / Ozone 工程 |

`storage_manager_task` 做 `LVGL → W25Q64`：`addr + MEMORY_LVGL_START_ADDRESS (0x300000)`。  
`W25Q64_8M_FLM.FLM` 做 `JLink → W25Q64`：`adr - 0x90000000 + 0x300000`。**FLM 范围被锁在 LVGL 分区内**，工具链根本无法触碰 OTA / FlashDB / FATFS / Reserved。

### 分区布局（`00_Config/inc/cfg_storage.h`）

```
W25Q64 物理        LVGL local      内容
0x300000           0x000000        magic 0xA55A5AAA (4 B)
0x301000           0x001000        41 张 UI 图片（每张独占 4KB 扇区对齐槽位）
0x396000           0x096000        9 套字体 glyph_bitmap（扇区对齐）
0x410000           0x110000        资产包结束（~1.06 MB / 3 MB 分区）
```

逐资产 offset/size 全部由 `cfg_storage.h` 宏锁定，pack_assets.py 解析同一头文件打包，固件按同一宏渲染——单一事实源。fen/time 两根表针 sprite 启动时镜像进 RAM（旋转重绘高频），其余 39 张全部行级 streaming。

### 软件链路

```
01_App/User_Sensor/storage/
├── storage_assets.c             ← 41 个 _ext lv_img_dsc_t 描述符 + magic 校验
│                                   + fen/time RAM 镜像加载

02_Service/service_storage/
├── inc/service_storage_facade.h
└── src/storage_manager_task.c    ← BSP async API 包成阻塞 Read/Write_LvglData

04_Impl/impl_middleware/lvgl/lvgl_port/
├── lv_port_extflash.c           ← 自定义 LVGL decoder，行级 streaming 39 张图
└── lv_port_extfont.c            ← 字体 get_glyph_bitmap 回调，按字形读 W25Q64
```

**资产唯一来源是 `cmake --build --preset Debug --target flash-assets`**（固件不带任何像素 seed）。`storage_assets_bootstrap()` 启动时只做 magic 校验 + fen/time RAM 镜像：magic 失配打 RTT 错误日志提示重烧资产包，UI 继续跑（降级显示）。

### LVGL 自定义 decoder（`lv_port_extflash`）与字体回调（`lv_port_extfont`）

- **图片**：`_ext` 描述符的 `lv_img_dsc_t.data` 不指像素，指 `lv_extflash_meta_t`（含 magic + offset + 几何）。decoder 的 `info_cb` 用 magic 识别"这张归我管"，`open_cb` 设 `img_data=NULL` 让 LVGL 切到行级模式，`read_line_cb` 调 `Read_LvglData` 抓一行给 LVGL（240px 宽 ALPHA 行 = 720 B ≈ 1 ms）。全屏背景首绘 ~240 ms，运行时只重绘脏区。
- **字体**：字体 `.c` 里 cmap/glyph_dsc 结构表留内部 Flash，`glyph_bitmap[]` 用 `#if 0` 关在源文本里（pack_assets.py 仍解析它打包）；`lv_font_t.get_glyph_bitmap` 换成 `lv_port_extfont_get_bitmap_<font>`，按 `bitmap_index` 从 W25Q64 读进静态字形缓冲（`CFG_LVGL_FONT_GLYPH_BUFFER_SIZE` 8 KB，LVGL 单任务渲染无并发）。每字形每次重绘一次 SPI 读，~2-3 ms。
- **GUI Guider 重新导出后的接入步骤**：generated 拷入 `lvgl_ui/` → 图片引用 `&_name` 改 `&_name_ext`（含 `gui_guider.h` 的 `LV_IMG_DECLARE`）→ 字体加 `lv_port_extfont.h` include + `#if 0` 位图守卫 + 回调替换 → `setup_scr_Clock_3.c`/`widgets_init.c` 补 `#include "lv_analogclock.h"` → 新资产进 `cfg_storage.h`/`pack_assets.py`/`storage_assets.c`，字体源码登记到 `07_Toolchain/app_sources.cmake` + bump magic → **重新把 `setup_scr_under_up.c` 里 5 个 `under_up_cont_*` 的 `shadow_spread` 改回 0**（重导出会回退成 10，那会让阴影缓冲从 2450 B 涨回 4050 B 并撕碎内存池，见"LVGL 内存池"节）→ **重新补回 sensor label 绑定与 `_del` flag 修正**（见下节，两处都会被重导出抹掉）→ **重新补回 `gui_guider.c` 的 `#include "cfg_ui.h"` 与 `ui_load_scr_animation()` 末尾的 `CFG_UI_SCR_ANIM_ENABLE` 覆写块**（见"切屏动画与撕裂"节）。

### 屏幕生命周期红线（GUI Guider 生成层）

`ui_load_scr_animation()` 先 `lv_obj_clean(lv_scr_act())` 再 `lv_scr_load_anim(..., delay, ...)`，而 LVGL 直到动画 start 回调才切 `disp->act_scr`。所以**在 delay 期间，旧屏仍是 active screen，但它的子控件已经全部 free 了**（"< Menu" 按钮传 `delay=200`，窗口 200 ms；手势翻页传 `delay=0`，窗口仍有一整个 `lv_timer_handler` 轮次）。由此两条红线：

- **绝不能用 `lv_scr_act() == ui->SomeScreen` 推断"这屏的子控件还活着"**。传感器数值视图（`ui_hr_view` / `ui_temp_humi_view`）改为显式绑定：`setup_scr_*()` 的 `//The custom code of X.` 槽里调 `ui_hr_view_bind_heart()` / `ui_hr_view_bind_under_up()` / `ui_temp_humi_view_bind()`，视图自己挂 `LV_EVENT_DELETE` 回调置空指针。三个 hook 声明在 `lvgl_ui/custom.h`（生成屏幕已经 include 它），所以 `04_Impl` 不会反向依赖 `01_App`。
- **`ui_load_scr_animation()` 第 4 个参数必须是"当前屏"自己的 `_del` flag**，不是目标屏的、也不是被复制粘贴来的别的屏的。传错会让本屏 `_del` 永远保持 false，下次进入时跳过 `setup_scr_*()` 直接 `lv_scr_load_anim()` 一个已 free 的屏对象。GUI Guider 导出的 `Heart/Map/NFC/QRcode/Systeamupdate` 五个 `< Menu` handler 全部误填 `&guider_ui.Set_del`，`top_lap`/`under_up` 手势 handler 也有 `top_lap_del`/`under_up_del` 互串，已在 `events_init.c` 修正。

### 烧录工具链

| 命令 | 作用 |
|---|---|
| `cmake --build --preset Debug` | 编固件（不含任何图片像素/字形位图，省 ~600 KB Flash） |
| `cmake --build --preset Debug --target pack-assets` | `99_Utils/pack_assets.py` 解析 cfg_storage.h + lv_conf.h + LVGL .c 数组 → `build/assets.bin`（4KB-aligned） |
| `cmake --build --preset Debug --target flash-assets` | pack + `JFlash.exe -openprj ... -auto -exit` 经 .FLM 直写 W25Q64 LVGL 分区 |

JLink 设备 `STM32F411CE_W25Q64` 注册在 `%APPDATA%\SEGGER\JLinkDevices\ST\STM32F4\Devices.xml`。FLM 是本板适配版二进制（SPI2/PB10/14/15、CS PB13），位于 `07_Toolchain/flash_algorithm/W25Q64_8M_FLM.FLM`（Keil MDK 源码工程未纳入本仓库，二进制为唯一交付物）。

### 切屏动画与撕裂

**硬件上不存在消除撕裂的同步通道**，这是排查过的结论，不要再往 TE 方向找：

- 显示 FPC（原理图 X3，`AFC01-S18FCA-00`）只引出 `LCD_3V3/GND/RST/MOSI/SCK/CS/DC/BACK_LIGHT` 和触摸的 `TP_INT`/`IIC_TP_*`，**没有 TE 引脚** → 无法做 TE 中断同步
- `hspi1` 是 `SPI_DIRECTION_1LINE`（半双工只发）且无 MISO 走线 → **也读不回 `GETSCANLINE(0x45)`**，软件轮询扫描线同样不可行

时钟侧也没有余量：PLL 源是 **HSI 16 MHz**（不是 HSE 8 MHz），`16/8×100/2` = SYSCLK **100 MHz**，已在 F411 上限；SPI1 = APB2/2 = **50 MHz**、SPI2 = APB1/2 = **25 MHz**，都是各自总线能给的最快档（F411 的 SPI 最高即 PCLK/2，APB1 上限 50 MHz）。**提频路线不存在。**

于是全屏刷新的物理下限是 240×284×2 B ÷ 50 MHz = **21.8 ms**，仍大于面板 60 Hz 的 16.7 ms 帧周期 → 单次全屏重绘必然跨扫描线。唯一能做的是**减少全屏重绘的次数和单次成本**。

`ui_load_scr_animation()` 的全部 `anim_type`（`OVER_*`/`MOVE_*`/`FADE_ON`）都会在**每一动画帧重绘进出两个屏**，200 ms 的动画因此变成几百 ms 的连续全屏重绘 —— 这正是"一操作就掉帧 + 剧烈撕裂"的来源。`00_Config/inc/cfg_ui.h` 的 `CFG_UI_SCR_ANIM_ENABLE`（默认 `0`）在 `gui_guider.c` 的单一收敛点把 `anim_type/time/delay` 强制成 `NONE/0/0`，把一次切屏收敛成一次重绘。置 `1` 即恢复生成的动画。

顺带缩小了"屏幕生命周期红线"那一节描述的 use-after-free 窗口（`delay` 归零），不会放大它。

**资产格式是下一个待优化项**：41 张图全部是 `LV_IMG_CF_TRUE_COLOR_ALPHA`（3 B/px，GUI Guider 默认导出行为）。扫描各图 alpha 通道的结果：

| 资源 | 尺寸 / 字节 | 不透明像素占比 | 结论 |
|---|---|---|---|
| `MDLBG` | 240×280 / 201600 B | 99.4% | Clock_2 最底层背景，下方是纯黑 `bg_opa=255` → 预乘黑底转 `TRUE_COLOR` 视觉无损 |
| `BIAOPAN1` | 200×200 / 120000 B | **100.0%** | 表盘底图，表针每次走动都重绘 → alpha 纯浪费 |
| `ELLIPSE` | 40×40 / 4800 B | **100.0%** | 同上 |
| 其余 38 张 | 小图标 | 0–60% | 真需要 alpha，保持不变 |

去掉这三张的 alpha 可省 **108800 B** 的 W25Q64 读取量，并让 LVGL 从逐像素 `blend_normal` 切到 `memcpy` 快路径。注意**这只减少 flash 读和 CPU 混合，不减少 SPI1 的面板写**（面板恒定 2 B/px）。实施要动 `pack_assets.py`（按资产选格式）+ `cfg_storage.h`（`*_PX_SIZE` 与 magic）+ `storage_assets.c`（`header.cf`）+ `lv_port_extflash.c`（行读格式）。

### 实测性能基线（关动画后）

`LVGL_PERF`（RTT 终端 4，每秒一行，见 `lvgl_display_task.c::lvgl_perf_report`）：

```
LVGL_PERF  scr/s=13.22 busy=88% dma=314ms(349ns/px) rend=596ms(662ns/px) flush=201 px=900762 err=0
```

| 场景 | 屏/秒 | dma ns/px | rend ns/px | busy |
|---|---|---|---|---|
| 空闲稳态（px≈22800） | 0.33 | 351 | 3333 | 8% |
| 切屏重载（px≈900762） | 13.2 | 349 | 662 | 88% |

两条结论锁死了后续方向：

- **SPI1 已跑满**：2 B/px @ 50 MHz 的理论下限是 **320 ns/px**，实测恒定 349–351 ns/px = 理论带宽的 92%（差额是 CASET/RASET 命令开销）。`dma` 侧没有任何优化空间，只能靠少画像素来减。
- **`rend` 的成本按重绘面积倒挂**：大面积图片重绘 662 ns/px，而空闲时的小面积文字重绘是 **3333 ns/px（贵 5 倍）**。空闲只画 0.33 屏却花 76 ms，9 次 flush 摊下来 8.4 ms/次 —— 对应 `lv_port_extfont` 每字形一次 W25Q64 读（2–3 ms）且**无缓存**，时钟标签每秒刷新即常驻此开销。想再压空闲功耗就得做 glyph 缓存，但 RAM 只剩约 2.5 KB。

`scr/s` 是**全屏等效重绘次数/秒**，不是常规 FPS —— LVGL 只重绘脏区，空闲时读数是零点几属正常。屏幕上 `LV_USE_PERF_MONITOR` overlay 的 FPS 静止时恒为 100，那只是 `1000 / LV_DISP_DEF_REFR_PERIOD(10)` 的空闲天花板，不含负载信息，别拿它判断性能。

## LVGL 内存池（32 KB，MPU 护栏保护）

> 原理、寄存器编码、故障分类、验证手段见 [05_Debug_Tool/README.md](05_Debug_Tool/README.md) "MPU_Protect" 节；LVGL 本地补丁清单见 [04_Impl/impl_middleware/README.md](04_Impl/impl_middleware/README.md)。

池存储**不在** `lv_mem.c` 的 `work_mem_int[]` 里，而由 `lvgl_port/lv_port_mem_pool.c` 持有，上下各夹 32 B `NO_ACCESS` 的 MPU region。越界立即进 `MemManage_Handler`（`__disable_irq()` 窗口内则由 `HardFault_Handler` 兜底），地址在 `SCB->MMFAR`，同时落 `g_mpu_fault` 供 Ozone 取证。`mpu_protect_init()` 在 `main()` 的 `USER CODE BEGIN 2` 里调，位置卡死：早于它 `.bss` 清零会自己踩护栏，晚于它池子可能已被访问。

两个后端的护栏本身完全一致（`mpu_protect_init()` 与后端无关，`MemManage_Handler` 两边都是本项目的），但 **HardFault 兜底路径的接法不同**：FreeRTOS 下 `stm32f4xx_it.c` 的 `HardFault_Handler` 直接调 `mpu_hardfault_report()`；RT-Thread 下该向量归内核的 `context_gcc.S`，改由 `src_rtthread/os_impl_kernel.c` 的 `osal_rt_exception_hook()`（`rt_hw_exception_install()` 注册，返回 `-RT_ERROR` 让内核继续打 register dump）承接。**改动任一后端的 fault 接线时两边都要同步**——RT-Thread 的自带 dump 不含 CFSR 解码（`hard_fault_track()` 在 `RT_USING_FINSH` 后面，shell 已移除），少了这个 hook 就只剩一堆寄存器、没有 MMFAR 归属。

注意护栏只管**池边界越界**：池内部的 use-after-free / 野指针写不会碰到护栏，MPU 不会响——那类问题看 `LVGL_MEM` 快照和对象生命周期（见上面"屏幕生命周期红线"）。

改动红线：

- **`LV_MEM_SIZE` 必须是 32 的倍数** —— MPU region 基址须按自身大小对齐，`lv_port_mem_pool.c` 有 `_Static_assert` 兜底，改成非 32 倍数会编译失败
- **不要把 `LV_MEM_POOL_ALLOC` 换成 `LV_MEM_ADR`** —— 后者要过 `#if LV_MEM_ADR == 0`，而链接期符号地址无法被 `#if` 求值，会静默退回 `work_mem_int[]` 且无任何报错
- **不要覆盖 `lv_mem.c` 里的 `LOCAL PATCH`** —— 升级 LVGL 前先 `grep -rn "LOCAL PATCH" 04_Impl/impl_middleware/lvgl/`
- **抗碎片配置不要改回默认**：`LV_IMG_CACHE_DEF_SIZE = 1`（原 4）、`under_up_cont_1..5` 的 `shadow_spread = 0`（原 10）

RAM 余量只剩约 6 KB（117936 / 123904，95.2%），扩池前先从 `lv_port_disp.c` 的双绘制缓冲（20 行 × 2 = 19200 B）里腾，别直接吃掉余量——MSP 中断栈也在里面。

排查内存问题：RTT 终端 4 的 `LVGL_MEM` 每 100 ms 打一次池快照（切屏时额外补一条），字段含义见 `lvgl_display_task.c::lvgl_mem_report`。**`used`/`peak` 看我们自己算的 `total_size - free_size`，不要信 `lv_mem_monitor()` 的 `max_used`** —— LVGL 只在 `lv_mem_alloc()` 里更新它，而 `lv_mem_realloc()` 完全不维护 `cur_used`，style 属性数组扩容走的正是 realloc，故上游峰值系统性偏低（实测 6981 vs 真实 20504）。`free` 大而 `big` 小即碎片化，非耗尽。

## OTA 升级链路

PC → UART1 → APP（Ymodem 收 + 写 W25Q64 OTA 分区）→ NVIC_SystemReset → Bootloader（AES-256-CBC 解密 + BLOCK_1→BLOCK_2→内部 Flash + 回滚兜底）。**触发不走 shell**，靠 UART 魔术字 `0x11 22 33`（启动）/ `0x77 88 99`（应用）。

### 分层（v4 — MCU port 接管 HAL，adapter 进 Service）

```
03_Platform/platform_mcu/MCU_Core_UART/ ← UART 在 MCU 端口（与 IIC/SPI/IFlash 同辈）
  inc/mcu_uart_port.h                   稳定 API（uart_id 参数化）
04_Impl/impl_mcu/MCU_Core_UART/         STM32 HAL + ISR dispatch
                                        + USART1_IRQHandler 也在这

02_Service/service_ota/                 ← OTA 业务 + 板级适配同位
  inc/ota_transport.h                   抽象（不绑定具体 UART id）
  inc/ota_storage.h                     抽象
  inc/firmware_upgrade.h                服务入口 / 任务声明
  inc/upgrade_service.h                 ota_flag 持久化
  src/ota_uart_listener.c               状态机
  src/firmware_upgrade_task.c           consumer
  src/iwdg_feeder_task.c                喂狗
  src/upgrade_service.c                 ota_flag 读写（走 MCU_Core_IFlash）
  adapters/                             ← 板级 wiring
    uart1_ota_transport.c               1-行翻译：ota_transport_* → mcu_uart_*
                                        (OTA_UART_ID == MCU_UART_1)
                                        + firmware_upgrade_signal_apply
                                          (NVIC_SystemReset，待 MCU_Core_Reset)
    w25q64_ota_storage.c                ota_storage_* → service_storage Write/Read_OtaData

04_Impl/impl_middleware/Ymodem/         ← 中间件，0 HAL include
  src/ymodem.c                          调 ota_transport_* 走全链路
```

调用栈：

```
ota_service_task                    ┐
  ota_transport_listen_byte_wait    │  service 层（业务）
    ↓                               ┘
  mcu_uart_recv_byte_wait(MCU_UART_1, ...)   ← adapter 1 行翻译
    ↓                               ┐
  osal_sema_take(s_state[1].byte_sem)         MCU port 内部
    ↓                                         （HAL + ISR + OS）
  [ ISR fires when byte arrives ]
    HAL_UART_RxCpltCallback
      state_for_handle(huart) → &s_state[1]
      osal_sema_give_from_isr(byte_sem)     ┘
```

替换 transport：
- 换 UART：改 `adapters/uart1_ota_transport.c` 里 `OTA_UART_ID` 一行
- 换 BLE：新增 `adapters/bleN_ota_transport.c` 实现 `ota_transport.h`，service / Ymodem / MCU_Core_UART 都不动
- 换 MCU：替换 `mcu_uart_port.c` 一个文件即可（API 不变），service / adapter / Ymodem 都不动

### 链路任务

| 任务 | 优先级 | 职责 |
|---|---|---|
| `ota_service_task` | `PRI_SOFT_REALTIME` | 状态机 `SCAN_START → YMODEM_ACTIVE → SCAN_APPLY`：`ota_transport_listen_byte_*` 1-byte 滑窗扫 `0x11 22 33` magic → 调 `Ymodem_Receive()`（内部走 `ota_transport_frame_*`）→ `firmware_upgrade_flush_staged()` + `ota_flag_write()` → IT 模式扫 `0x77 88 99` apply magic → `firmware_upgrade_signal_apply()`（adapter 内 `NVIC_SystemReset`） |
| `firmware_upgrade_task` | `PRI_SOFT_REALTIME` | consume `Queue_AppDataBuffer`，4 KB sector 缓冲后调 `ota_storage_write` 写下层（默认 W25Q64） |
| `iwdg_feeder_task` | `PRI_BACKGROUND` | always-on 500 ms 喂狗（F411 IWDG 起后不可关）|

OS 全局对象按所有权分两组：

- **adapter 内部**（`uart1_ota_transport.c` 私有）：
  - `s_byte_sem`（binary）—— `HAL_UART_RxCpltCallback` 给一次
  - `s_frame_queue`（uint16_t × 4）—— `HAL_UARTEx_RxEventCallback` 给段长
  - 创建/管理由 `ota_transport_init()` 处理
- **service 共享**（`firmware_upgrade_task.c` 定义，Ymodem extern）：
  - `Queue_AppDataBuffer`（`Ymodem_RxContext_t*` × 2）—— Ymodem user_handler → consumer
  - `Semaphore_ExtFlashState`（binary）—— consumer 写完一包 → Ymodem 切 ping-pong buffer
  - 创建由 `firmware_upgrade_service_init()` 处理（一站式：post-OTA verify + transport_init + storage_init + service resources）

### UART1 RX 双路（互斥不并存）

| 模式 | 谁用 | 抽象 API |
|---|---|---|
| 中断单字节 | `ota_service_task` 在 SCAN_START / SCAN_APPLY 状态 | `ota_transport_listen_byte_arm` + `_wait` |
| DMA-idle frame | `Ymodem_Receive` 整段（YMODEM_ACTIVE 状态内） | `ota_transport_frame_arm` + `_is_armed` + `_wait` + `_stop` |

具体 HAL 实现细节封在 `uart1_ota_transport.c`：IT single-shot → magic 第 3 字节命中自然消耗；YMODEM_ACTIVE 进入时 RxState 已经回 READY，frame_arm 直接成功。session 结束（无论成败）后 `ota_service_task` 调 `listen_byte_arm` 重 arm IT 回到扫 magic。任一时刻只有一个 HAL 回调被武装。

### 加密格式（`99_Utils/ota_encrypt.py`）

`cmake --build --preset Debug --target ota-image` 跑 Python 脚本（pycryptodome，uv 自动装）：

```
[12 B 零 | 4 B LE app_size | helloworld-<backend>.bin | 0xFF pad 到 16 B 对齐]
        ↓
AES-256-CBC 加密（key/iv = bootmanager.c 硬编码 32×{0x31,0x32} 交替，硬编码仅过渡）
        ↓
build/helloworld-<backend>.mxxx
```

bootloader 的 `exA_to_exB_AES` 解密首块取 bytes[12..15] 当 LE uint32 = app_size。

### 关键地址与状态字（`00_Config/inc/cfg_ota.h`）

| 项 | 值 | 说明 |
|---|---|---|
| `CFG_OTA_FLAG_ADDRESS` | `0x08008000` | 内部 Flash Sector 2（16 KB），存 `ota_flag_t` |
| `CFG_OTA_FLAG_MAGIC` | `0xA55A5AA5` | magic，与空 sector（0xFFFFFFFF）区分 |
| W25Q64 OTA staging | `0x000000` | 1 MB，`MEMORY_OTA_START_ADDRESS` |
| 内部 APP 槽 | `0x0800C000` | 464 KB（sectors 3-7）|
| linker `__app_size__` | `LOADADDR(.data) + SIZEOF(.data) - ORIGIN(FLASH)` | 当前 APP 字节数，写入 `ota_flag.current_app_size` 供 bootloader 回滚备份 |

### 状态机（`CFG_OTA_*` 数值）

| state | 谁写 | 触发 |
|---|---|---|
| `0xFF INIT_NO_APP` | （magic 失配兜底） | 全片 erase 首启 |
| `0x00 NO_APP_UPDATE` | APP user_init / Bootloader CHECKING 兜底 | APP 已 confirm，正常跳 APP |
| `0x22 DOWNLOAD_FINISHED` | APP `ymodem_recv_task` | Ymodem 完成 + W25Q64 写完 |
| `0x33 APP_CHECK_START` | Bootloader `ota_apply_update` 末尾 | 解密 + 拷贝完成，跳 APP 等 confirm |
| `0x44 APP_CHECKING` | Bootloader `OTA_StateManager::CHECK_START` 分支 | 跳 APP 前再推进一次（防 reset 中断后死循环）|

APP `user_init` 看到 `0x33` 或 `0x44` 都 auto-confirm 写 `0x00`；若 IWDG 在 6 s 内没被 APP 喂上，bootloader CHECKING 分支兜底回滚。

### 易踩坑

- **W25Q64 Page Program 跨 page 回卷**：`bsp_w25q64_driver.c::w25q64_write_data_erase` 必须按 256 B page 边界拆 chunk，erase 仍 per-sector (4 KB)。否则单个 Page Program 跨 page 时 W25Q64 地址回卷到 page 起点 → 后续字节覆盖前面字节
- **APP `Write_OtaData` 每次 erase 整 sector**：sub-sector 写入会抹掉同 sector 之前的数据。`firmware_upgrade_task.c` 用 4 KB `s_sector_buf[]` 攒满再写，ymodem_recv_task 调 `firmware_upgrade_flush_staged()` 冲尾段。该不变量现已在 seam 处强制：`ota_storage_write` 对非扇区对齐/非整扇区写直接返回 `OTA_STORAGE_ERR`（把静默擦写变显式错误），`firmware_upgrade_service_init` 启动时校验 `UPGRADE_SECTOR_BUF_SIZE == ota_storage_sector_size()`
- **内部 Flash 操作必须关中断**：F411 单 bank flash erase 阻塞 ~1 s，期间 ISR 在 flash 取指会死锁。`MCU_Core_IFlash/iflash_port.c` 已经在 `mcu_iflash_erase_sector` / `mcu_iflash_program_words` 函数体内用 `__disable_irq()` + 出口恢复 PRIMASK 兜住，service 层调用方不用再手动关
- **UART1 polled HAL 会饿死 PRI_BACKGROUND**：listener 必须走中断/DMA 模式，否则 `iwdg_feeder` 抢不到 CPU → IWDG fire 整机 reset

## 新增外设驱动步骤

遵循现有 BSP 适配器模式：
1. `04_Impl/impl_bsp/Bsp_Drivers/<device>/driver/` — 原始设备通信（禁止 OSAL）
2. `04_Impl/impl_bsp/Bsp_Drivers/<device>/handler/` — 读取驱动的 handler 线程
3. `03_Platform/platform_bsp/<category>/bsp_wrapper_<cat>/` — 抽象 vtable API
4. `04_Impl/impl_bsp/Adapter_Port/<category>/` — 将驱动注册到 vtable
5. `04_Impl/impl_bsp/Bsp_Integration/<device>_integration/` — 组装 `*_input_arg` 结构体
6. 在 `01_App/User_Init/Platform_IO_Register/` 中注册硬件 IO
7. 在 `User_Task_Config/src/user_task_reso_config.c` 的 `g_user_task_cfg[]` 中添加任务项
8. 将所有新增 `.c` 文件加入 `07_Toolchain/app_sources.cmake`

**ISR 规则**：禁止在中断上下文中获取 IIC 总线互斥锁。使用 `osal_notify` 唤醒 handler 任务，由线程上下文获取互斥锁。

## 关键配置文件

| 文件 | 控制内容 |
|---|---|
| `06_Vendor/Core/Inc/FreeRTOSConfig.h` | 堆大小、tick 频率、优先级级别、启用特性 |
| `06_Vendor/Core/Inc/stm32f4xx_hal_conf.h` | 编译哪些 ST HAL 模块 |
| `07_Toolchain/STM32F411XX_FLASH.ld` | 内存映射、段放置 |
| `06_Vendor/Core/Inc/main.h` | 引脚定义、全局包含 |
| `07_Toolchain/app_sources.cmake` | 业务源文件登记入口（新增 `.c` 写这里；CubeMX 管的源在 `06_Vendor/cmake/stm32cubemx/CMakeLists.txt`） |
| `07_Toolchain/bsp_driver_libs.cmake` | 6 个核心 BSP 驱动静态库（`libbsp_{aht21,cst816t,em7028,mpuxxxx,st7789,w25q64}_driver.a`）构建定义；driver 源文件单独成 lib，改驱动只重链该 lib |
| `03_Platform/platform_mcu/MCU_Core_IIC/inc/i2c_port.h` | I2C 总线索引枚举、互斥锁超时（硬件/软件描述符已下沉到 `04_Impl/impl_mcu/MCU_Core_IIC/src/i2c_port.c`，头文件 MCU 无关） |
