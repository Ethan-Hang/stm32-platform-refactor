##########################################################################################################################
# Bootloader Makefile (based on gcc)
# Derived from 01_APP/Makefile, adapted for STM32F4xx StdPeriph Bootloader
##########################################################################################################################

######################################
# target
######################################
TARGET = bootloader


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
BUILD_DIR = build

# Cross-platform directory removal command
ifeq ($(OS),Windows_NT)
RM_DIR = cmd /C if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
else
RM_DIR = rm -fR $(BUILD_DIR)
endif

######################################
# source
######################################
# Core sources
C_SOURCES = \
Core/Src/main.c \
Core/Src/flash.c \
Core/Src/gpio.c \
Core/Src/iwdg.c \
Core/Src/spi.c \
Core/Src/stm32f4xx_it.c \
Core/Src/system_stm32f4xx.c \
Core/Src/tim.c \
Core/Src/uart.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c

# STM32F4xx Standard Peripheral Library (modules used by STM32F411xE)
C_SOURCES += \
Drivers/STM32F4xx_StdPeriph_Driver/src/misc.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
Drivers/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c

# BSP drivers
C_SOURCES += \
Bsp/AT24Cxx/src/at24cxx_driver.c \
Bsp/AT24Cxx/src/iic.c \
Bsp/W25Qxx/src/w25qxx_Handler.c \
Bsp/W25Qxx/src/w25qxx.c

# Task modules
C_SOURCES += \
Tasks/Bootmanager/src/bootmanager.c

# Middleware — EasyLogger
C_SOURCES += \
Middlewares/EasyLogger/src/elog.c \
Middlewares/EasyLogger/src/elog_async.c \
Middlewares/EasyLogger/src/elog_buf.c \
Middlewares/EasyLogger/src/elog_utils.c \
Middlewares/EasyLogger/port/elog_port.c

# Middleware — SEGGER RTT
C_SOURCES += \
Middlewares/RTT/SEGGER_RTT.c \
Middlewares/RTT/SEGGER_RTT_printf.c

# Middleware — SFUD (SPI Flash Universal Driver)
C_SOURCES += \
Middlewares/SFUD/src/sfud.c \
Middlewares/SFUD/src/sfud_sfdp.c \
Middlewares/SFUD/port/sfud_port.c \
Middlewares/SFUD/sfud_adapter.c

# Middleware — Ymodem
C_SOURCES += \
Middlewares/Ymodem/src/common.c \
Middlewares/Ymodem/src/ymodem.c

# Middleware — AES
C_SOURCES += \
Middlewares/AES/src/aes.c

# ASM sources
ASM_SOURCES = startup_stm32f411xe.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC  = $(GCC_PATH)/$(PREFIX)gcc
AS  = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP  = $(GCC_PATH)/$(PREFIX)objcopy
SZ  = $(GCC_PATH)/$(PREFIX)size
NM  = $(GCC_PATH)/$(PREFIX)nm
else
CC  = $(PREFIX)gcc
AS  = $(PREFIX)gcc -x assembler-with-cpp
CP  = $(PREFIX)objcopy
SZ  = $(PREFIX)size
NM  = $(PREFIX)nm
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS = \
-DUSE_STDPERIPH_DRIVER \
-DSTM32F411xE


#######################################
# include paths
#######################################

# CMSIS Core + Device
C_INCLUDES_CMSIS = \
-IDrivers/CMSIS/Include \
-IDrivers/CMSIS/Device/ST/STM32F4xx/Include

# StdPeriph
C_INCLUDES_STDPERIPH = \
-IDrivers/STM32F4xx_StdPeriph_Driver/inc

# Core
C_INCLUDES_CORE = \
-ICore/Inc \
-IDebug/inc

# BSP
C_INCLUDES_BSP = \
-IBsp/AT24Cxx/inc \
-IBsp/W25Qxx/inc

# Tasks
C_INCLUDES_TASKS = \
-ITasks/Bootmanager/inc

# Middleware
C_INCLUDES_MW = \
-IMiddlewares/EasyLogger/inc \
-IMiddlewares/EasyLogger/port \
-IMiddlewares/RTT \
-IMiddlewares/SFUD/inc \
-IMiddlewares/SFUD/port \
-IMiddlewares/Ymodem/inc \
-IMiddlewares/AES/inc

C_INCLUDES = \
$(C_INCLUDES_CORE) \
$(C_INCLUDES_CMSIS) \
$(C_INCLUDES_STDPERIPH) \
$(C_INCLUDES_BSP) \
$(C_INCLUDES_TASKS) \
$(C_INCLUDES_MW)

# AS includes
AS_INCLUDES =

CFLAGS  = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS  += -g -gdwarf-2
ASFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
LDSCRIPT = STM32F411XX_BOOTLOADER_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


#######################################
# default action: build all
#######################################
all: | $(BUILD_DIR)
	@cd Tools && uv run build_summary.py --make-exe "$(MAKE)" --build-dir "../$(BUILD_DIR)" --target "build-core"

build-core: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin mem-report


#######################################
# build the application
#######################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@


#######################################
# memory report
#######################################
# MEM_REPORT_FAIL_ABOVE: optional fraction in [0, 1] (e.g. 0.95). When set, any
# linker MEMORY region above this usage fraction causes mem-report (and thus the
# build) to fail. Leave empty to keep mem-report informational.
MEM_REPORT_FAIL_ABOVE ?=
ifneq ($(strip $(MEM_REPORT_FAIL_ABOVE)),)
MEM_REPORT_EXTRA_ARGS := --fail-above $(MEM_REPORT_FAIL_ABOVE)
else
MEM_REPORT_EXTRA_ARGS :=
endif

mem-report: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	@cd Tools && uv run mem_report.py --elf "../$(BUILD_DIR)/$(TARGET).elf" --ld "../$(LDSCRIPT)" --size-tool "$(SZ)" $(MEM_REPORT_EXTRA_ARGS)


#######################################
# clean up
#######################################
clean:
	-$(RM_DIR)

.PHONY: all build-core clean mem-report

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
