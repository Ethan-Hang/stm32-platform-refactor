# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Target Platform

- MCU: STM32F411xE (Cortex-M4, 512 KB Flash, 128 KB RAM)
- Toolchain: `arm-none-eabi-gcc`
- RTOS: FreeRTOS with CMSIS-RTOS V2
- Debugger: J-Link via OpenOCD

## Build Commands

```bash
make -j16            # Build (outputs to build/)
make clean           # Remove build directory
make clean && make -j16  # Full rebuild
```

Flash via OpenOCD (requires J-Link connected):
```bash
openocd -f ./Tools/interface/jlink.cfg -f ./Tools/target/stm32f4x.cfg \
        -c "program ./build/helloworld.elf verify reset exit"
```

VS Code tasks (`Ctrl+Shift+B`): **build**, **clean**, **rebuild**, **download**, **rebuild & download**.

After a successful build, `make` prints a memory usage summary (Flash/RAM used %) and the top 20 symbols by size.

## Architecture

The project uses a strict layered platform architecture. Dependencies only flow downward.

```
01_APP                  ← Business logic, tasks, ISR handlers
02_BSP_Platform         ← Hardware device drivers + abstract interfaces
02_MCU_Platform         ← MCU peripheral wrappers (IIC, GPIO clusters)
02_OS_Platform          ← RTOS abstraction layer (OSAL)
02_Middleware_Platform  ← EasyLogger, LetterShell
03_Config               ← Project-wide config headers (no implementation)
04_Common_Utils         ← Pure utility code (linked lists, CRC, etc.)
04_Debug_Tool           ← Debug_Log, SEGGER SystemView
Core/                   ← STM32CubeMX-generated HAL init & FreeRTOS hook
Drivers/                ← ST HAL library + CMSIS (do not edit)
Middlewares/            ← FreeRTOS source (do not edit)
```

### Key patterns

**OS Platform (`02_OS_Platform`)**
Two sub-layers: `OS_Wrapper/` (public `osal_*` API — validates arguments, ISR guards) calls into `OS_Implementation/` (`os_impl_*` — FreeRTOS-specific). New RTOS support requires only replacing `OS_Implementation`.

**BSP Platform (`02_BSP_Platform`)**
Each device lives in `Bsp_Drivers/<device>/driver/` and `Bsp_Drivers/<device>/handler/`. Abstract interfaces live in `Platform_Interface/<category>/`:
- `bsp_wrapper_<category>` — holds a vtable (`drv_adapter_*_mount()` populates it at runtime).
- `bsp_adapter_port_<category>` — fills the vtable for a specific chip.
`Bsp_Integration/` wires a specific driver into the wrapper at startup.

**Config Layer (`03_Config`)**
All configuration macros use the `CFG_` prefix. Feature flags (`CFG_USE_*`, `ENABLE_*`) control compile-time inclusion. Never hard-code board resources; read from `BoardConfig.h`, `RtosConfig.h`, etc.

**Debug / Logging (`04_Debug_Tool/Debug_Log`)**
Each module declares a `*_LOG_TAG` / `*_ERR_LOG_TAG` pair in `Debug.h` and is added to `debug_is_tag_allowed()`. Log calls use `DEBUG_OUT(i, TAG, ...)` / `DEBUG_OUT(e, TAG, ...)` which wrap EasyLogger (`elog_i` / `elog_e`).

## Code Style

Enforced by `.clang-format` (run `clang-format -i <file>` to auto-format):
- 4-space indent, no tabs
- Allman brace style (opening brace on its own line)
- Pointer aligned right (`int *p`)
- Consecutive declarations, assignments, and macros column-aligned
- `SortIncludes: Never`

File header template (every `.c` / `.h`):
```c
/******************************************************************************
 * @file <filename>
 * @par dependencies
 * @author Ethan-Hang
 * @brief
 * @version V1.0 YYYY-MM-DD
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
```

Section delimiters used inside files:
```c
//******************************** Includes *********************************//
//******************************** Defines **********************************//
//******************************* Declaring *********************************//
//******************************* Functions *********************************//
```
