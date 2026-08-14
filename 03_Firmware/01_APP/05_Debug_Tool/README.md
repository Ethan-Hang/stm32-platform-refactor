# 05_Debug_Tool — 调试 / 日志 / 追踪

集中放调试期工具（日志、追踪、保护），统一通过 `DEBUG_OUT(level, tag, fmt, ...)` 宏输出，按 tag 路由到不同物理通道。

完整 RTT 通道分配 + 新增 tag 步骤见 [`../CLAUDE.md`](../CLAUDE.md) "调试日志路由"节。

## 子模块

| 子目录 | 内容 |
|---|---|
| `Debug_Log/` | `Debug.h` 定义 `DEBUG_OUT` 宏 + tag 常量；`Debug.c` 实现 tag 过滤 + RTT/ITM 路由；底层走 EasyLogger（[`../04_Impl/impl_middleware/EasyLogger/`](../04_Impl/impl_middleware/EasyLogger/)） |
| `Systemview/` | SEGGER SystemView + RTT 控制块（`SEGGER_RTT.h/.c`）。RTT_RAM 段独立放在 `0x2001E400`，7 KB（上行 buffer `BUFFER_SIZE_UP`=2048）|
| `SWO_Trace/` | `itm_trace.h` —— ITM stimulus port 0 输出，给 ITM-only tag 走 `printf` → SWO Viewer / Ozone SWO 终端 |
| `MPU_Protect/` | `mpu.h/.c` —— 用两条 `NO_ACCESS` MPU region 夹住 LVGL 内存池，越界即触发 MemManage（见下节）|

## 两路输出共存

```
DEBUG_OUT(level, tag, ...)
   │
   ├── 一般 tag → EasyLogger → SEGGER_RTT_SetTerminal() → RTT 通道 0  → RTT Viewer
   │                                                       (按 Terminal Tab 分组 0-8)
   └── ITM-only tag (route=DEBUG_ROUTE_ITM) → printf → __io_putchar() → ITM port 0 → SWO Viewer
```

RTT Terminal 分组（`DEBUG_RTT_CH_*`）：

| Terminal | 覆盖 tag |
|---|---|
| 0 | 默认（未显式路由） |
| 1 | AHT21 / 温湿度 |
| 2 | WT588 handler / 测试 |
| 3 | MPU6050 / 数据解析 |
| 4 | ST7789 TFT-LCD + LVGL 自身日志 + `LVGL_MEM` 池快照 + `MPU`/`MPU_ERR` |
| 5 | CST816T 触摸 |
| 6 | W25Q64 SPI NOR |
| 7 | EM7028 PPG 心率 |
| 8 | 栈水位监控 |

## 新增 tag

tag 路由由 `Debug.c` 的 `s_route_table[]` 单表驱动，`debug_route_lookup()` 一趟扫描定路由；表里没有的 tag 自动丢弃。

1. `Debug.h` 中定义 `*_LOG_TAG` 常量
2. 在 `s_route_table[]` 加一行 `{ TAG, DEBUG_RTT_CH_x, DEBUG_ROUTE_RTT }`（`DEBUG_RTT_CH_DEFAULT` 即终端 0）

ITM-only tag：定义 `*_ITM_LOG_TAG` 后加一行 `{ TAG, 0, DEBUG_ROUTE_ITM }`，无需改 `elog_port.c`。
停用某 tag：删除/注释该行即可。

## MPU_Protect —— LVGL 内存池护栏

LVGL 池是全固件唯一持续高频进出的大 arena（32 KB），写出边界会落在相邻 `.bss` 对象上：静默损坏，很久之后才以一个无从追溯的 HardFault 浮现。`MPU_Protect/` 把这类 bug 变成**在犯事的那条指令上**触发的 MemManage，地址留在 `SCB->MMFAR`。

### 原理：护栏，不是"保护池本身"

池是堆，LVGL 每时每刻都要读写，不能标 `NO_ACCESS`。MPU 也不认识 TLSF 的块结构。所以保护的是池的**邻居位置**——上下各 32 B 永不该被碰的地址：

```
0x20012FA0  guard_lo  32 B  NO_ACCESS   ← 向下越界踩这里
0x20012FC0  LVGL pool 32 KB 权限不变     ← LVGL 正常读写
0x2001AFC0  guard_hi  32 B  NO_ACCESS   ← 向上越界踩这里
```

护栏是双向的：LVGL 写出去、或邻居 `.bss` 反向越界写进池子，都会命中。命中时写操作被硬件取消，**损坏被阻止而不只是被检测到**。

### 地址怎么来的

存储权从 `lv_mem.c` 的匿名 `work_mem_int[]` 夺过来，交给 [`../04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_mem_pool.c`](../04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_mem_pool.c)。三段放进**同一个 struct**：

```c
typedef struct {
    uint8_t guard_lo[LV_PORT_MEM_GUARD_SIZE];   /* 32 */
    uint8_t pool[LV_MEM_SIZE];
    uint8_t guard_hi[LV_PORT_MEM_GUARD_SIZE];
} lv_port_mem_block_t;
static lv_port_mem_block_t s_block __attribute__((aligned(LV_PORT_MEM_GUARD_SIZE)));
```

用 struct 而非三个独立数组：**C 保证成员地址按声明顺序递增**，三个 `uint8_t[]` 之间不会有 padding，于是护栏与池的相邻性在语言层面就是定理，不依赖链接脚本段排布、不依赖工具链行为。也不用在 `.ld` 里复制一份 `LV_MEM_SIZE`（两个事实源，改一处忘另一处就是错位的护栏，比没有护栏更危险）。

MPU 侧只经三个访问函数取址：`lv_port_mem_pool_guard_lo/_guard_hi/_base()`。

### 32 B 对齐要满足两次

PMSAv7 规定 **region 基址必须按 region 自身大小对齐**。这条在两个层面咬人：

1. **架构层面** —— 不对齐则地址匹配行为未定义。
2. **寄存器编码层面** —— `MPU_RBAR` 低 5 位不是地址，是 `VALID`(4) 和 `REGION`(3:0)。而 `HAL_MPU_ConfigRegion` **原样写入不做掩码**（`stm32f4xx_hal_cortex.c:319`），基址低 5 位非零会被硬件解读成"顺带改写 region 编号"——你以为在配 region 0，实际配到了别处，且无任何提示。

两道防线：`aligned(32)` 属性管住 `guard_lo`；`_Static_assert(LV_MEM_SIZE % 32 == 0)` 管住 `guard_hi`（其地址 = 基址 + 32 + `LV_MEM_SIZE`）。**改 `LV_MEM_SIZE` 成非 32 倍数会编译失败**，而不是带着错位护栏跑。

### 寄存器与使能

两条 region 的 `MPU_RASR` 均为 `0x10030009`：`XN`=1、`AP`=`0b000`(NO_ACCESS，特权非特权都禁)、`C`=`B`=1、`SIZE`=4(2^5=32 B)、`ENABLE`=1；只有 `RBAR` 不同。

```c
HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);   /* MPU_CTRL = 0x05 */
```

`PRIVDEFENA` 这一位不能省：MPU 一旦使能，**未被任何 region 覆盖的地址默认不可访问**——只定义两条 32 B region 而不开背景域，Flash / SRAM / 全部外设寄存器立刻全部失效，下一条指令就死。开了它，护栏之外一律走 ARM 默认内存映射，行为与没开 MPU 完全一致。该调用同时置 `SCB_SHCSR_MEMFAULTENA`，否则护栏命中直接升级 HardFault 并丢掉 `MMFAR`。

### 调用时机（卡在两个约束之间）

`main()` 的 `USER CODE BEGIN 2`，在 `debug_init()` 之后、`platform_io_register()` 之前：

- **不能更早**：护栏在 `.bss`，启动代码要对 `.bss` 全段清零，MPU 先武装的话**清零动作自己就踩护栏**，开机即死。
- **不能更晚**：必须早于任何可能碰池子的代码。
- 排在 `debug_init()` 后，是为了让这条自检日志发得出去：

```
I/MPU  LVGL pool guarded: lo=0x20012FA0 pool=0x20012FC0..0x2001AFC0 hi=0x2001AFC0
```

### 错误处理

```
越界写 → MPU 命中 region 且 AP=000 → 写被取消（内存未改）
      → CFSR.MMFSR: DACCVIOL(bit1)=1, MMARVALID(bit7)=1；MMFAR=出事地址
      → MemManage_Handler → mpu_memmanage_report()
```

`mpu_memmanage_report()` 拿 `MMFAR` 与池边界比对，分成 `GUARD_LO` / `GUARD_HI` / `NO_ADDRESS` / `UNKNOWN` 并算出越界字节数：

```
E/MPU_ERR  LVGL pool OVERRUN at 0x2001AFC4 (4 B past pool end 0x2001AFC0) cfsr=0x00000082
```

三个要点：

- **必须先查 `MMARVALID`**。`MSTKERR`/`MUNSTKERR`（异常入栈/出栈时的违例）**不更新 `MMFAR`**，不查就读会拿到上一次故障的陈旧值——看起来合理、实则完全误导。
- **先落 `g_mpu_fault` 全局体，再打日志**。日志要过 EasyLogger → RTT 环形缓冲 → 等主机轮询，链条长且身处异常上下文；结构体赋值几乎不会失败，RTT 出不去时 Ozone 仍能取证。`count` 字段区分"从未故障"与"故障过 N 次"。
- **`HardFault_Handler` 兜底**。MemManage 是可配置故障，`PRIMASK` 置位时会升级成 HardFault——`MCU_Core_IFlash` 的 `__disable_irq()` 窗口就是真实存在的场景。`mpu_hardfault_report()` 用 `HFSR.FORCED && MMFSR != 0` 判定是升级而来，转走同一套分类（升级不清 `MMFSR`/`MMFAR`）。

`CFSR` 是 write-1-to-clear 粘滞寄存器，报告完清掉本次置起的位，否则下次读到新旧并集、分类错乱。报告后落回 `while(1)`，**不尝试恢复**：护栏被踩说明内存一致性已不可信，继续跑只会搅糊现场。

### 作用边界

| 抓得到 | 抓不到 |
|---|---|
| 写出池头/池尾（哪怕 1 字节） | **池内部块间越界**——地址仍在池内，MPU 看不见（TLSF 层面的问题） |
| 指向池附近的野指针读写 | **DMA 越界**——MPU 只管 CPU 访问 |
| 邻居 `.bss` 反向越界写进池 | 越界跨度 > 32 B 而"跳过"护栏 |
| 上述发生在 `__disable_irq()` 窗口内 | |

成本：RAM +112 B / Flash +1800 B。

### 验证它真的装上了

护栏没有正常访问路径，"一直不触发"既可能是没 bug，也可能是**压根没装上**。两个确认手段：

1. 开机看上面那条 `I/MPU` 日志的地址是否合理。
2. 查符号表确认存储权真的转移了 —— `work_mem_int` 必须**不存在**，否则说明 LVGL 还在用自己的池，护栏保护的是一块没人用的空内存：

```bash
arm-none-eabi-nm -S --size-sort build/helloworld.elf | grep -iE 's_block|work_mem_int'
# 20012fa0 00008040 b s_block      ← 0x8040 = 32 + 32768 + 32
```

主动打一次（验完删掉）：

```c
*((volatile uint8_t *)lv_port_mem_pool_base() + lv_port_mem_pool_size()) = 0xAA;
```

## 注意

- `elog_port_init()` 调 `SEGGER_RTT_Init()` 会无条件重置 `WrOff/RdOff` —— Bootloader 在 `jump_to_app()` 前 `delay_ms(200)` 留给 RTT Viewer 轮询，否则最后一行日志被 APP 抹掉。
- `_SEGGER_RTT` 控制块固定在 `0x2001E400`，APP 与 Bootloader 共享同一物理地址才能让 RTT Viewer 不切换（改地址需同步两份 ld）。
