# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 构建命令

```bash
make                # 完整构建 → build/helloworld.{elf,hex,bin}
make clean          # 删除 build/ 目录
make mem-report     # 内存占用报告（Windows 下运行 PowerShell 脚本）
make OPT=-O2        # 覆盖优化等级
```

目标芯片：STM32F411xE（Cortex-M4F），工具链：`arm-none-eabi-gcc`，默认编译选项：`-O1 -g -gdwarf-2`。

无正式测试框架。应用级验证任务位于 `01_APP/User_Sensor/`，在目标硬件上运行（如 `temp_humi_test_task_a/b` 用于并发读取验证）。

## 架构总览

严格的分层依赖——上层只依赖下层：

```
01_APP                  ← 业务逻辑、任务定义、传感器测试
02_*_Platform           ← 四个能力层（见下文）
ST HAL / FreeRTOS       ← 厂商中间件
ARM CMSIS / 硬件        ← 寄存器级
```

### 平台层（`02_*/`）

| 目录 | 职责 |
|---|---|
| `02_OS_Platform/` | 对 FreeRTOS 的 OSAL 封装。`OS_Wrapper/inc/` 是公开 API；`OS_Implementation/` 包含 FreeRTOS 映射实现。替换 `OS_Implementation` 即可切换 RTOS。 |
| `02_BSP_Platform/` | 使用适配器模式的传感器/外设驱动（详见下文）。 |
| `02_MCU_Platform/` | 芯片级 I2C/SPI 总线驱动。管理总线级互斥锁（`CORE_I2C_BUS_MUTEX_TIMEOUT_MS = 8ms`）。同时支持硬件 I2C（HAL）和软件 I2C 位操作（SCL=PB14，SDA=PB15）。 |
| `02_Middleware_Platform/` | EasyLogger（异步日志），LetterShell（嵌入式 CLI），LVGL v8.3（GUI 库）。 |

### BSP 适配器模式（`02_BSP_Platform/`）

每个外设遵循以下三层结构：

```
Bsp_Drivers/<device>/driver/    ← 原始寄存器/协议通信，禁止调用 OSAL
Bsp_Drivers/<device>/handler/   ← Handler 线程逻辑（读取驱动，投递到队列）
Platform_Interface/<category>/
  bsp_wrapper_<cat>/            ← 向 01_APP 暴露的抽象 vtable API
  bsp_adapter_port_<cat>/       ← 将具体驱动注册到 wrapper vtable
Bsp_Integration/<device>_integration/  ← 将驱动+OS 资源组装为 input_arg 结构体
```

`bsp_wrapper_*` 头文件定义公开 API（`_sync` 和 `_async` 两种读取变体）及 vtable 结构体。`bsp_adapter_port_*` 头文件仅暴露 `drv_adapter_<cat>_register()`。集成层负责组装传递给 handler 线程的 `*_input_arg` 结构体。现有设备：`aht21`（温湿度）、`mpu6050`（运动）、`wt588f02`（音频）、`st7789`（LCD 显示）、`cst816t`（触摸屏）、`w25q64`（NOR Flash，driver+handler 已完成，集成层 WIP）。

### 应用层（`01_APP/`）

- **任务表**：`User_Task_Config/src/user_task_reso_config.c` — 定义 `g_user_task_cfg[]`，包含所有应用任务的名称、栈大小、优先级、入口函数和参数。
- **任务优先级**（定义于 `user_task_reso_config.h`）：`PRI_EMERGENCY`、`PRI_HARD_REALTIME`、`PRI_SOFT_REALTIME`、`PRI_NORMAL`、`PRI_BACKGROUND`。
- **任务创建**：`User_Init/user_init.c` 遍历 `g_user_task_cfg[]`，逐条调用 `osal_task_create()`；失败时回滚。
- **IO 注册**：`User_Init/Platform_IO_Register/` — 启动时将硬件 IO 绑定到驱动适配器。
- **ISR 派发**：`User_Isr_handlers/` — ISR 通过 OSAL notify 唤醒任务，避免在中断上下文中阻塞（防止 IIC 互斥锁死锁）。
- **栈水位监控**：`User_Task_Config/src/task_higher_water_monitor.c` — 运行时任务栈占用监控。

### OSAL 层（`02_OS_Platform/`）

`OSAL_Common/inc/osal_common_types.h` 定义项目全局共用类型：`osal_task_handle_t`、`osal_queue_handle_t`、`osal_mutex_handle_t`、`osal_tick_type_t` 等。始终通过 `osal_wrapper_adapter.h` 包含。

### 通用工具层（`04_Common_Utils/`）

与硬件/业务无关的工具代码（StrUtils、CRC16、MemPool、ByteConverter 等），所有层均可复用，不依赖 OSAL 或 BSP。

### 配置层（`03_Config/`）

用于项目级宏开关（`CFG_` 前缀）：功能特性开关、板级 IO 映射、RTOS 资源大小。当前仅有 README，实际配置头文件尚未创建。

### 调试工具（`04_Debug_Tool/`）

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
| 4 | `DEBUG_RTT_CH_STACK` | 栈水位监控 |

新增 tag 步骤：
1. 在 `Debug.h` 中定义 `*_LOG_TAG` 字符串常量。
2. 将其加入 `debug_is_tag_allowed()`（启用输出）。
3. 在 `debug_tag_to_rtt_channel()` 中指定 Terminal，或加入 `debug_is_itm_tag()` 走 ITM 路径。

新增 ITM-only tag 步骤：
1. 在 `Debug.h` 中定义 `*_ITM_LOG_TAG` 常量。
2. 将其加入 `debug_is_itm_tag()`。无需修改 `elog_port.c` 或 RTT 配置。

#### SEGGER SystemView

实时 OS 追踪，通过 RTT 传输。8KB SRAM 专用于 RTT buffer（链接脚本中 `RTT_RAM` 区域，地址 `0x2001E000`）。

## 调试工作流

| 操作 | 工具 |
|---|---|
| 烧录固件 | SEGGER JFlash — 打开 `build/helloworld.hex`，目标 STM32F411xE |
| 源码调试 | SEGGER Ozone — 加载 `build/helloworld.elf`，通过 JLink 连接 |
| OS 任务追踪 | SEGGER SystemView — 通过 JLink 附加到运行中的目标 |
| printf 日志 | JLink RTT Viewer — 通道 0，1000000 波特率 |
| SWO 输出 | JLink SWO Viewer 或 Ozone SWO 终端，波特率由 `itm_trace_init(cpu_hz, swo_hz)` 配置 |

典型流程：JFlash 烧录 → Ozone 断点/监视 → SystemView RTOS 时序 → RTT Viewer 日志输出。

## 硬件

- **MCU**：STM32F411xE — Cortex-M4F，512KB FLASH，128KB SRAM
- **RTOS**：FreeRTOS v10.3.1，heap_4，60KB 堆，1 kHz tick，CMSIS-RTOS V2 API 可用
- **FPU**：单精度硬浮点（`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`）
- **链接脚本**：`STM32F411XX_FLASH.ld` — 120KB 用户 RAM + 8KB RTT RAM

### 关键引脚分配（`Core/Inc/main.h`）

| 信号 | 引脚 |
|---|---|
| 软件 I2C SCL | PB14 |
| 软件 I2C SDA | PB15 |
| SPI1 CS/RST/DC | PA3/PA4/PA6 |
| 软件 SPI SCK/MISO/MOSI | PA5/PA6/PA7 |
| WT588 busy | PA12 |
| 触摸屏中断 TINT | PB2（EXTI2） |

## 新增外设驱动步骤

遵循现有 BSP 适配器模式：
1. `02_BSP_Platform/Bsp_Drivers/<device>/driver/` — 原始设备通信（禁止 OSAL）
2. `02_BSP_Platform/Bsp_Drivers/<device>/handler/` — 读取驱动的 handler 线程
3. `02_BSP_Platform/Platform_Interface/<category>/bsp_wrapper_<cat>/` — 抽象 vtable API
4. `02_BSP_Platform/Platform_Interface/<category>/bsp_adapter_port_<cat>/` — 将驱动注册到 vtable
5. `02_BSP_Platform/Bsp_Integration/<device>_integration/` — 组装 `*_input_arg` 结构体
6. 在 `01_APP/User_Init/Platform_IO_Register/` 中注册硬件 IO
7. 在 `User_Task_Config/src/user_task_reso_config.c` 的 `g_user_task_cfg[]` 中添加任务项
8. 将所有新增 `.c` 文件加入 `Makefile` 的 `C_SOURCES`

**ISR 规则**：禁止在中断上下文中获取 IIC 总线互斥锁。使用 `osal_notify` 唤醒 handler 任务，由线程上下文获取互斥锁。

## 关键配置文件

| 文件 | 控制内容 |
|---|---|
| `Core/Inc/FreeRTOSConfig.h` | 堆大小、tick 频率、优先级级别、启用特性 |
| `Core/Inc/stm32f4xx_hal_conf.h` | 编译哪些 ST HAL 模块 |
| `STM32F411XX_FLASH.ld` | 内存映射、段放置 |
| `Core/Inc/main.h` | 引脚定义、全局包含 |
| `02_MCU_Platform/MCU_Core_IIC/i2c_port/inc/i2c_port.h` | I2C 总线类型（硬件/软件）、互斥锁超时 |
