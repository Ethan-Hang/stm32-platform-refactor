# LayerCore

**A Layered Embedded Firmware Platform**

**分层、可替换的嵌入式固件平台**

`STM32F411` · `FreeRTOS` · `LVGL` · `CMake`

应用依赖稳定的能力接口，而不是具体的 MCU、RTOS、设备或中间件实现。

Applications depend on stable capability interfaces—not concrete MCU, RTOS, device, or middleware implementations.

---

## 为什么选择 LayerCore / Why LayerCore

- **稳定的应用边界**：APP 和业务无关的 Service 只调用平台接口，不感知具体硬件与实现。

  **Stable application boundaries:** APP and hardware-independent services use platform APIs without depending on concrete hardware implementations.

- **可替换的平台实现**：MCU、RTOS、同类外设与中间件可以按层替换，业务代码保持稳定。

  **Replaceable platform implementations:** MCU, RTOS, peripheral, and middleware implementations can evolve independently while application code remains stable.

- **分离的 APP 与 Service**：业务任务位于 `01_APP/`，OTA、存储门面等通用服务位于平级的 `01_SERVICE/`。

  **Separated APP and Service layers:** Product tasks live in `01_APP/`, while reusable services such as OTA and storage live alongside them in `01_SERVICE/`.

- **独立的外部资源链路**：LVGL 图像和字体资源存放于 W25Q64，可独立于内部 Flash 固件打包与烧录。

  **Independent external asset flow:** LVGL images and fonts live in W25Q64 and can be packaged and flashed independently from internal-Flash firmware.

---

## 架构总览 / Architecture Overview

LayerCore 以 APP 与 Service 为使用者，以四个平台提供稳定能力。上层只面向抽象接口编程；下层实现可以在各自边界内替换。

LayerCore treats APP and Service as capability consumers backed by four stable platforms. Upper layers program against abstract interfaces, while lower-layer implementations remain replaceable within their boundaries.

| 平台 / Platform | 抽象能力 / Capability | 当前实现 / Current implementation | 替换方式 / Replacement boundary |
|---|---|---|---|
| **OS Platform** | 任务、互斥、信号量、队列、定时器、堆 | FreeRTOS v10.3.1 映射 | 替换 `OS_Implementation/` 以迁移 RTOS |
| **MCU Platform** | I2C、SPI、UART、内部 Flash、Watchdog 与启动/异常支持 | STM32 HAL + port 层 | 替换 port 层与启动文件以迁移 MCU |
| **BSP Platform** | 显示、触摸、环境、运动、语音、外部存储等设备类别接口 | ST7789、CST816T、AHT21、MPU6050、WT588F02、W25Q64 | 替换同类设备的 driver 与 adapter |
| **Middleware Platform** | 日志、传输、图形、算法与通用数据处理 | EasyLogger、Ymodem、LVGL、心率算法；LetterShell 已停用 | 各中间件独立接入、启停或替换 |

完整分层、接口约束和开发规则见[固件架构与开发指南](03_Firmware/01_APP/README.md)。

See the [firmware architecture and development guide](03_Firmware/01_APP/README.md) for complete layering, interface constraints, and development rules.

---

## BSP 适配器模式 / BSP Adapter Pattern

BSP 设备按职责拆分为五个阶段，APP 最终只依赖设备类别接口：

BSP devices are separated into five responsibility-focused stages, leaving APP dependent only on device-category interfaces:

1. **driver** — 实现原始协议通信，不依赖 OSAL。

   Implements raw protocol communication without depending on OSAL.

2. **handler（可选）** — 为持续流式设备运行任务，将驱动数据送入队列；请求—响应型设备通常不需要。

   Runs a task for continuous streaming devices and forwards driver data to queues; request-response devices usually omit this stage.

3. **wrapper** — 向 APP 暴露稳定的设备类别 vtable API。

   Exposes a stable device-category vtable API to APP.

4. **adapter** — 将具体 driver 映射并注册到 wrapper 接口。

   Maps and registers a concrete driver with the wrapper interface.

5. **integration** — 组装设备实例、依赖和初始化参数。

   Assembles device instances, dependencies, and initialization arguments.

新增同类外设时，主要替换 `driver` 与 `adapter`，APP 保持不变。

Adding another device of the same category primarily replaces its `driver` and `adapter`, leaving APP unchanged.

---

## 参考实现 / Reference Implementation

当前参考实现运行于 STM32F411xE 智能手表硬件，用于验证 LayerCore 的分层边界；架构本身不绑定该硬件组合。

The current reference implementation runs on STM32F411xE-based smart watch hardware to validate LayerCore's boundaries; the architecture itself is not tied to this hardware combination.

| 类别 / Category | 当前选型 / Current choice |
|---|---|
| MCU | STM32F411xE（Cortex-M4F） |
| RTOS | FreeRTOS v10.3.1 |
| GUI | LVGL |
| Display / Touch | ST7789 / CST816T |
| Sensors | AHT21 / MPU6050 |
| Voice / Storage | WT588F02 / W25Q64 |

---

## 快速开始 / Quick Start

所有 APP CMake 命令都在 `03_Firmware/01_APP/` 中运行。

Run all APP CMake commands from `03_Firmware/01_APP/`.

```bash
cd 03_Firmware/01_APP

# Configure and build firmware
cmake --preset Debug
cmake --build --preset Debug --parallel 16

# Build and flash internal-Flash firmware with SEGGER JFlash
cmake --build --preset Debug --target download --parallel 16

# Inspect memory usage or clean generated files
cmake --build --preset Debug --target mem-report
cmake --build --preset Debug --target clean

# Package or flash LVGL assets in external W25Q64
cmake --build --preset Debug --target pack-assets
cmake --build --preset Debug --target flash-assets
```

固件构建产物位于 `build/helloworld.{elf,hex,bin,map,mxxx}`。内部 Flash 固件与 W25Q64 LVGL 资源是两条独立烧录路径；完整 UI 需要配套烧录正确版本的资源包。

Firmware artifacts are written to `build/helloworld.{elf,hex,bin,map,mxxx}`. Internal-Flash firmware and W25Q64 LVGL assets use independent flashing paths; the complete UI requires the matching asset package.

工具链包括 CMake、Ninja、`arm-none-eabi-gcc`、由 uv 管理脚本环境的 Python，以及用于烧录、调试和追踪的 SEGGER J-Link 工具。Bootloader 仍使用 GNU Make。

The toolchain includes CMake, Ninja, `arm-none-eabi-gcc`, Python script environments managed by uv, and SEGGER J-Link tools for flashing, debugging, and tracing. The Bootloader continues to use GNU Make.

---

## 仓库导航 / Repository Guide

| 路径 / Path | 职责 / Responsibility |
|---|---|
| [`00_Reference/`](00_Reference/) | 数据手册与参考资料 / Datasheets and reference material |
| [`01_Function_Map/`](01_Function_Map/) | 功能规划 / Feature planning |
| [`02_Hardware/`](02_Hardware/) | 原理图与 PCB 工程 / Schematics and PCB projects |
| [`03_Firmware/`](03_Firmware/) | Bootloader 与主应用固件 / Bootloader and main application firmware |
| [`04_Software/`](04_Software/) | 上位机软件 / Host-side software |
| [`05_Tools/`](05_Tools/) | 仓库级辅助工具 / Repository-level tools |
| [`docs/`](docs/) | 开发约定与项目文档 / Development conventions and project documentation |

主应用固件位于 `03_Firmware/01_APP/`：

The main application firmware lives in `03_Firmware/01_APP/`:

| 路径 / Path | 职责 / Responsibility |
|---|---|
| `01_APP/` | 业务逻辑、任务表、IO 注册与 ISR 派发 / Product logic, task table, IO registration, and ISR dispatch |
| `01_SERVICE/` | OTA、存储门面等业务无关服务 / Hardware-independent services such as OTA and storage |
| `02_OS_Platform/` | OSAL 接口与 RTOS 实现映射 / OSAL interfaces and RTOS mappings |
| `02_MCU_Platform/` | MCU 总线、内部 Flash、Watchdog 与 port 实现 / MCU buses, internal Flash, Watchdog, and ports |
| `02_BSP_Platform/` | 设备 driver、handler、wrapper、adapter 与 integration / Device drivers and adaptation stages |
| `02_Middleware_Platform/` | 日志、Ymodem、LVGL 与算法 / Logging, Ymodem, LVGL, and algorithms |
| `03_Config/` | `CFG_` 项目级配置 / Project-level `CFG_` configuration |
| `04_Common_Utils/` | 硬件无关工具库与自定义 `.FLM` / Hardware-independent utilities and custom `.FLM` |
| `04_Debug_Tool/` | 日志、追踪、ITM/RTT 与 MPU 保护 / Logging, tracing, ITM/RTT, and MPU protection |

---

## CI 与文档 / CI and Documentation

Push 或 Pull Request 到 `master` 时，[GitHub Actions](.github/workflows/c-cpp.yml) 自动构建固件并保存 elf、hex、bin、map 与 mxxx 产物 30 天。

On pushes or pull requests to `master`, [GitHub Actions](.github/workflows/c-cpp.yml) builds the firmware and retains elf, hex, bin, map, and mxxx artifacts for 30 days.

- [固件架构与开发指南 / Firmware architecture and development guide](03_Firmware/01_APP/README.md)
- [LVGL 外部 Flash 资源说明 / LVGL external-Flash asset guide](03_Firmware/01_APP/05_Common_Utils/02_docs/lvgl-assets-external-flash.md)
- [AI 辅助开发指引 / AI-assisted development guide](03_Firmware/01_APP/CLAUDE.md)
- [CI 工作流 / CI workflow](.github/workflows/c-cpp.yml)
