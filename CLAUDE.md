# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 仓库性质

智能手表项目的**多层级容器仓库**，根目录不是代码工程而是组织结构。固件本体在子目录中，并配有更详细的 CLAUDE.md。

## 目录索引

| 路径 | 内容 | 何时进入 |
|---|---|---|
| [03_Firmware/](03_Firmware/) | 固件工程容器 | 任何固件相关工作 → 先读 [03_Firmware/CLAUDE.md](03_Firmware/CLAUDE.md) |
| [03_Firmware/01_APP/](03_Firmware/01_APP/) | **主应用固件**（FreeRTOS + LVGL + 多传感器），所有构建在此 | 改代码前必读 [03_Firmware/01_APP/CLAUDE.md](03_Firmware/01_APP/CLAUDE.md) |
| [02_Hardware/](02_Hardware/) | 原理图、PCB 工程 | 仅硬件设计相关 |
| [00_Reference/](00_Reference/) | 数据手册、参考文档 | 查芯片寄存器/外设手册 |
| [01_Function_Map/](01_Function_Map/) | 功能规划（当前为空） | — |
| [04_Software/](04_Software/) | 上位机软件 | 仅 PC 端工作 |
| [05_Tools/](05_Tools/) | 辅助脚本（当前为空） | — |
| [lancedb/](lancedb/) | 工具索引数据，**非固件源码** | — |

**重要**：构建命令、Makefile、源码树都在 [03_Firmware/01_APP/](03_Firmware/01_APP/)。`make` 必须在该目录下运行，不是在仓库根目录。

## 架构核心思想（细节见内层 CLAUDE.md）

以 **APP 为核心、四个能力平台环绕** 的可移植架构：APP 不感知具体 MCU、RTOS、外设、中间件实现，只通过四个平台的抽象 API 调用能力。

```
                ┌──────────── OS Platform (OSAL) ────────────┐
                │   Task / Mutex / Sem / Queue / Timer / Heap │
                └─────────────────────────────────────────────┘
                                    │
   ┌──────────────┐                 ▼               ┌──────────────────┐
   │ MCU Platform │ ──────────── APP ────────────▶  │ Middleware Plat. │
   │ I2C/SPI/UART │                ▲                │ Shell/Log/LVGL   │
   └──────────────┘                │                └──────────────────┘
                ┌──────────── BSP Platform ───────────────────┐
                │   driver / handler / wrapper / adapter      │
                │   integration（五段式）                      │
                └─────────────────────────────────────────────┘
```

替换粒度：
- 换 RTOS → 改 `02_OS_Platform/OS_Implementation/`
- 换 MCU → 改 `02_MCU_Platform/` port 层 + 启动文件
- 换某个外设 → 替换该外设的 `driver/` + `adapter/`，APP 不动

## 何时去读哪个 CLAUDE.md

- **固件构建/烧录/调试链路** → [03_Firmware/CLAUDE.md](03_Firmware/CLAUDE.md)
- **改 APP 代码、加任务、加传感器、改日志路由、ISR/OSAL 规则** → [03_Firmware/01_APP/CLAUDE.md](03_Firmware/01_APP/CLAUDE.md)
- **完整架构图与开发指南** → [03_Firmware/01_APP/README.md](03_Firmware/01_APP/README.md)

## CI

GitHub Actions（[.github/workflows/c-cpp.yml](.github/workflows/c-cpp.yml)）在 push/PR 到 master 时自动构建固件，产出 elf/hex/bin/map 保留 30 天。
