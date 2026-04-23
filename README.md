# STM32F411 FreeRTOS 固件框架

基于 STM32F411xE + FreeRTOS 的嵌入式固件工程，采用严格分层 + BSP 适配器模式，支持多传感器并发读取、OSAL 可替换、双路调试输出（RTT + ITM/SWO）。

---

## 📋 硬件平台

| 项目 | 规格 |
|---|---|
| MCU | STM32F411xE（Cortex-M4F，512KB FLASH，128KB SRAM） |
| RTOS | FreeRTOS v10.3.1，heap_4，60KB 堆，1 kHz tick |
| FPU | 单精度硬浮点（`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`） |
| 调试接口 | JLink（SWD） |

---

## 🛠️ 工具链要求

| 工具 | 用途 |
|---|---|
| `arm-none-eabi-gcc` | 交叉编译 |
| `make` | 构建 |
| SEGGER JFlash | 烧录 `build/helloworld.hex` |
| SEGGER Ozone | 源码级调试，加载 `build/helloworld.elf` |
| SEGGER SystemView | RTOS 实时任务追踪 |
| JLink RTT Viewer | 日志输出，通道 0，波特率 1000000 |
| JLink SWO Viewer | ITM/SWO 输出（部分 tag 专用路径） |

---

## 🔨 构建

```bash
make                # 完整构建 → build/helloworld.{elf,hex,bin}
make clean          # 清理 build/
make mem-report     # 内存占用报告（Windows PowerShell）
make OPT=-O2        # 指定优化等级
```

---

## 🏗️ 工程架构

严格分层，上层只依赖下层：

```
01_APP/                 业务逻辑、FreeRTOS 任务定义、传感器验证测试
02_OS_Platform/         OSAL 封装（替换 OS_Implementation 即可换 RTOS）
02_BSP_Platform/        外设驱动（BSP 适配器模式，见下文）
02_MCU_Platform/        芯片级 I2C/SPI 总线驱动 + 总线互斥锁
02_Middleware_Platform/ EasyLogger 异步日志 + LetterShell 嵌入式 CLI
03_Config/              项目级宏开关（CFG_ 前缀，待扩展）
04_Debug_Tool/          SEGGER SystemView、RTT 日志、ITM/SWO 追踪
ST HAL / FreeRTOS       厂商中间件
ARM CMSIS / 硬件寄存器
```

### BSP 适配器模式

每个外设拆分为五个部分，互不耦合：

```
Bsp_Drivers/<device>/driver/        原始协议通信，禁止 OSAL 调用
Bsp_Drivers/<device>/handler/       Handler 任务：读驱动，投队列
Platform_Interface/bsp_wrapper_*/   向 APP 暴露的抽象 vtable API
Platform_Interface/bsp_adapter_*/   将具体驱动注册到 vtable
Bsp_Integration/<device>_integration/  组装 input_arg 结构体
```

已实现设备：`aht21`（温湿度）、`mpu6050`（运动）、`wt588f02`（音频播报）。

### 任务管理

所有 FreeRTOS 任务集中登记在  
`01_APP/User_Task_Config/src/user_task_reso_config.c` 的 `g_user_task_cfg[]` 表中，  
包含任务名、栈大小、优先级、入口函数和参数。`User_Init/user_init.c` 在启动时遍历该表创建所有任务。

> **ISR 规则**：中断上下文禁止获取 IIC 总线互斥锁。通过 `osal_notify` 唤醒 handler 任务，由线程上下文完成总线操作。

---

## 🐛 调试

### 日志系统（`DEBUG_OUT` 宏）

使用 `04_Debug_Tool/Debug_Log/inc/Debug.h` 中的 `DEBUG_OUT(level, tag, fmt, ...)` 输出日志。

两路输出路径同时有效，按 tag 分配：

- **RTT 路径**：大多数 tag → EasyLogger → RTT channel 0，RTT Viewer 按 Terminal Tab 分组显示。
- **ITM 路径**：`debug_is_itm_tag()` 中列出的 tag → `printf()` → ITM stimulus port 0 → SWO Viewer。

RTT Terminal 分组：

| Terminal | 内容 |
|---|---|
| 0 | 默认（未显式路由的 tag） |
| 1 | AHT21 / 温湿度 |
| 2 | WT588 音频 |
| 3 | MPU6050 / 数据解析 |
| 4 | 任务栈水位监控 |

### 调试流程

```
JFlash 烧录 hex
    ↓
Ozone 加载 elf（断点 / 变量监视）
    ↓
SystemView（RTOS 任务时序追踪）
    ↓
RTT Viewer / SWO Viewer（日志输出）
```

---

## 📁 关键文件速查

| 文件 | 说明 |
|---|---|
| `Core/Inc/FreeRTOSConfig.h` | 堆大小、tick、优先级级别 |
| `STM32F411XX_FLASH.ld` | 内存映射（120KB RAM + 8KB RTT RAM） |
| `Core/Inc/main.h` | 引脚定义、全局 include |
| `02_MCU_Platform/MCU_Core_IIC/i2c_port/inc/i2c_port.h` | I2C 类型切换（HW/SW）、互斥锁超时 |
| `04_Debug_Tool/Debug_Log/inc/Debug.h` | 日志 tag 定义、过滤、RTT/ITM 路由 |
| `Makefile` | 所有源文件列表、编译选项 |
