# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
make                # Full build → build/helloworld.{elf,hex,bin}
make clean          # Remove build/ directory
make mem-report     # Memory usage breakdown (runs PowerShell script on Windows)
```

Target: STM32F411xE (Cortex-M4F), toolchain: `arm-none-eabi-gcc`, default flags: `-O0 -g -gdwarf-2`.

Override optimization: `make OPT=-O2`

There is no formal test runner. Application-level tests live in `01_APP/User_Sensor/` and `01_APP/User_Task_Config/` and run on-target.

## Architecture

The project uses a strict layered dependency: upper layers depend only on layers below them.

```
01_APP                  ← Business logic, task definitions, sensor tests
02_*_Platform           ← Four capability layers (see below)
ST HAL / FreeRTOS       ← Vendor middleware
ARM CMSIS / Hardware    ← Register-level
```

### Platform Layers (`02_*/`)

| Directory | Purpose |
|---|---|
| `02_OS_Platform/` | OSAL wrapper over FreeRTOS. `OS_Wrapper/inc/` is the public interface; `OS_Implementation/` is the FreeRTOS mapping. Swap RTOS by replacing `OS_Implementation` only. |
| `02_BSP_Platform/` | Hardware drivers (AHT21, MPU6050, WT588F02). Uses adapter pattern: `Bsp_Drivers/` = raw driver, `Platform_Interface/` = wrapper + adapter port, `Bsp_Integration/` = wires driver to OS/MCU. |
| `02_MCU_Platform/` | Chip-level services: software I2C bit-bang (SCL=PB14, SDA=PB15), GPIO, SPI utilities. |
| `02_Middleware_Platform/` | EasyLogger (async logging), LetterShell (embedded CLI). |

### Application Layer (`01_APP/`)

Task creation entry point: `User_Init/user_init.c`. Hardware IO registration happens in `User_Init/Platform_IO_Register/`. Task priorities/stacks are defined in `User_Task_Config/`. ISR dispatch in `User_Isr_handlers/`.

### Debug Tools (`04_Debug_Tool/`)

- **SEGGER SystemView**: Real-time OS tracing via RTT. 28KB of SRAM is dedicated to the RTT buffer (linker script: `RTT_RAM` region at `0x20019000`).
- **Debug_Log**: Printf-style logging, output via JLink RTT Viewer.

## Debug Workflow

| Task | Tool |
|---|---|
| Flash firmware | SEGGER JFlash — open `build/helloworld.hex`, target STM32F411xE |
| Source-level debug | SEGGER Ozone — load `build/helloworld.elf`, connects via JLink |
| OS task tracing | SEGGER SystemView — attach to running target via JLink |
| Printf log output | JLink RTT Viewer — channel 0, 1000000 baud |

All tools connect through the same JLink probe. Typical workflow: flash with JFlash → open Ozone for breakpoints/watch → attach SystemView for RTOS timeline → RTT Viewer for log output.

## Hardware

- **MCU**: STM32F411xE — Cortex-M4F, 512KB FLASH, 128KB SRAM
- **RTOS**: FreeRTOS v10.3.1, heap_4, 60KB heap, 1 kHz tick, CMSIS-RTOS V2 API available
- **FPU**: Single-precision hard-float (`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`)
- **Linker**: `STM32F411XX_FLASH.ld` — 100KB user RAM + 28KB RTT RAM

### Key Pin Assignments (`Core/Inc/main.h`)

| Signal | Pin |
|---|---|
| Soft I2C SCL | PB14 |
| Soft I2C SDA | PB15 |
| Soft SPI SCK/MISO/MOSI/CS | PA5/PA6/PA7/PA4 |
| WT588 busy | PA12 |

## Adding a New Peripheral Driver

Follow the existing BSP adapter pattern:
1. `02_BSP_Platform/Bsp_Drivers/<device>/` — raw device communication (no OSAL calls)
2. `02_BSP_Platform/Platform_Interface/<category>/` — wrapper API + adapter port header
3. `02_BSP_Platform/Bsp_Integration/<device>_integration/` — wires driver into OS tasks
4. Register hardware IO in `01_APP/User_Init/Platform_IO_Register/`
5. Add source files to the `C_SOURCES` list in `Makefile`

## Key Configuration Files

| File | What it controls |
|---|---|
| `Core/Inc/FreeRTOSConfig.h` | Heap size, tick rate, priority levels, enabled features |
| `Core/Inc/stm32f4xx_hal_conf.h` | Which ST HAL modules are compiled in |
| `STM32F411XX_FLASH.ld` | Memory map, section placement |
| `Core/Inc/main.h` | Pin definitions, global includes |
