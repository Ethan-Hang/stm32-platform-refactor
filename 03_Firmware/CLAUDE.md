# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 仓库布局

此仓库根目录是 STM32F411 固件工程的容器，包含两个独立的固件：

| 目录 | 内容 |
|---|---|
| `01_APP/` | 主应用固件（FreeRTOS + 多传感器 + LVGL）。**几乎所有开发工作发生在这里。** |
| `00_Bootloader/` | Bootloader（当前为空占位） |
| `lancedb/` | 工具/索引数据，非固件源码 |

**关键提示**：构建、Makefile、源码树、详细架构说明都在 `01_APP/`。所有 `make` 命令必须在 `01_APP/` 目录下运行，而不是仓库根目录。

## 构建

```bash
cd 01_APP && make              # → 01_APP/build/helloworld.{elf,hex,bin}
cd 01_APP && make -j16         # 并行（VSCode build task 默认配置）
cd 01_APP && make clean
cd 01_APP && make mem-report   # Tools/mem_report.py，FLASH/RAM 占用图
cd 01_APP && make OPT=-O2      # 覆盖默认 -Os

# 外部 Flash LVGL 资源（不动 firmware）
cd 01_APP && make pack-assets  # → build/assets.bin
cd 01_APP && make flash-assets # 经 JFlash CLI + 自定义 .FLM 烧 W25Q64
```

VSCode 任务（`.vscode/tasks.json`）已将 cwd 设为 `${workspaceFolder}/01_APP`，可直接用 `build` / `clean` / `rebuild` 标签触发。

工具链：`arm-none-eabi-gcc`，目标 STM32F411xE（Cortex-M4F，硬浮点 fpv4-sp-d16）。无正式测试框架——验证任务在硬件上跑（见 `01_APP/User_Sensor/`）。

## 详细架构

**完整的分层架构、BSP 适配器模式、OSAL 设计、调试日志路由（RTT vs ITM/SWO）、新增外设步骤等，全部记录在 [01_APP/CLAUDE.md](01_APP/CLAUDE.md) 中。** 开始任何 `01_APP/` 内的工作前先读那个文件。

要点速览（细节见上述文件）：

- 严格分层：`01_APP` → `02_*_Platform`（OS / BSP / MCU / Middleware）→ ST HAL / FreeRTOS → CMSIS / 寄存器
- BSP 五段式：`driver/`（裸协议）+ `handler/`（任务）+ `bsp_wrapper_*`（vtable API）+ `bsp_adapter_port_*`（注册）+ `Bsp_Integration/`（input_arg 组装）
- 所有任务集中登记在 `01_APP/User_Task_Config/src/user_task_reso_config.c` 的 `g_user_task_cfg[]`
- ISR 不可获取 IIC 总线互斥锁——通过 `osal_notify` 唤醒 handler 任务在线程上下文操作
- 日志：`DEBUG_OUT(level, tag, fmt, ...)` 按 tag 路由到 RTT Terminal（0–4）或 ITM/SWO
- LVGL 资源（指针小图 + 240×240 表盘背景）在外部 W25Q64 上：sprite 由 firmware self-bootstrap 从 .rodata 写入；240×240 背景太大不进 .rodata，必须由 `make flash-assets` 经自定义 .FLM 写入，运行时由 `lv_port_extflash` decoder 行级 streaming

## 调试链路

JFlash 烧录 `build/helloworld.hex` → Ozone 加载 `build/helloworld.elf` 源码调试 → SystemView 看 RTOS 任务时序 → JLink RTT Viewer（通道 0，1000000 baud）看日志。
