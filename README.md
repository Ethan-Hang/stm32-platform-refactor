# 智能手表

基于 STM32F411xE + FreeRTOS 的嵌入式固件工程，集成 LVGL 图形界面、多传感器驱动（温湿度、运动、音频、LCD、触摸屏），采用严格分层架构与 BSP 适配器模式。

STM32F411xE + FreeRTOS embedded firmware project with LVGL GUI, multi-sensor drivers (temperature/humidity, motion, audio, LCD, touch), strict layered architecture and BSP adapter pattern.

---

## 目录结构

```
00_Reference/       数据手册与参考文档
01_Function_Map/    功能规划
02_Hardware/        硬件设计（原理图、PCB）
03_Firmware/        固件工程（详见内部 README）
04_Software/        上位机软件
05_Tools/           辅助工具
```

## 快速开始

固件构建入口位于 [03_Firmware/01_APP/](03_Firmware/01_APP/)。

```bash
cd 03_Firmware/01_APP
make                # 完整构建 → build/helloworld.{elf,hex,bin}
make clean          # 清理
make mem-report     # 内存占用报告
```

### 工具链

- `arm-none-eabi-gcc`（CI 使用 13.3.rel1）
- GNU Make
- SEGGER JLink 系列工具（烧录、调试、RTT 日志、SystemView 追踪）

## 硬件平台

| 项目 | 规格 |
|---|---|
| MCU | STM32F411xE（Cortex-M4F，512KB FLASH，128KB SRAM） |
| RTOS | FreeRTOS v10.3.1 |
| 显示屏 | ST7789 240x240 LCD |
| 触摸 | CST816T 电容触摸 |
| 传感器 | AHT21（温湿度）、MPU6050（6 轴运动） |
| 音频 | WT588F02 语音播报 |

## CI/CD

GitHub Actions 自动构建（[c-cpp.yml](.github/workflows/c-cpp.yml)），Push/PR 到 master 时触发，生成 elf/hex/bin/map 产物并保留 30 天。

## 详细文档

- [固件架构与开发指南](03_Firmware/01_APP/README.md)
- [AI 辅助开发指引](03_Firmware/01_APP/CLAUDE.md)
