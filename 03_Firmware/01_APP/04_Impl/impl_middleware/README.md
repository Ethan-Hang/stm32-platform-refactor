# impl_middleware — 中间件实现

通用算法/协议/GUI 库。中间件之间互不耦合，按需启停；可选依赖 OSAL / BSP。

## 当前实现

| 子目录 | 内容 | 备注 |
|---|---|---|
| `EasyLogger/` | 异步日志框架 | `port/` 适配本项目；输出经 `SEGGER_RTT_SetTerminal()` 路由到 RTT 通道 0-8（详见 [`../../05_Debug_Tool/`](../../05_Debug_Tool/)） |
| `LetterShell/` | 嵌入式 CLI | **当前未启用** —— shellTask 已废弃，新功能勿引入 UART shell I/O |
| `Ymodem/` | Ymodem 收包协议 | 零 HAL include，通过 `ota_transport_*` 抽象走全链路；APP CRC-16/XMODEM 校验 + EOT 严格握手 |
| `heart_rate_algo/` | PPG 心率算法 | EM7028 frame 输入；由 `01_App/User_Sensor/em7028/` 任务驱动 |
| `lvgl/` | LVGL v8.3 + port + UI | `lvgl/` 原生库（**带本地补丁，见下**）；`lvgl_port/lv_port_extflash.c` 自定义 decoder（行级 streaming 240×240 表盘背景）；`lvgl_port/lv_port_mem_pool.c` 持有内存池存储 + MPU 护栏；`lvgl_ui/` 业务 UI（GUI Guider 生成）|

## 依赖规则

```
EasyLogger     ──>  03_Platform/platform_os (OSAL mutex)  + 05_Debug_Tool (RTT)
Ymodem         ──>  ota_transport_* 抽象 (零 HAL；adapter 在 02_Service)
LVGL           ──>  lvgl_port → MCU_SPI (LCD) / Read_LvglData (storage_manager)
                    lv_port_mem_pool ←── 05_Debug_Tool/MPU_Protect 读护栏边界
heart_rate_algo──>  无 OS / BSP 依赖，纯算法
```

## LVGL 本地补丁（升级 LVGL 前必读）

`lvgl/` 是 vendored 上游源码，但**有本地改动**，直接覆盖升级（或从新版 `lv_conf_template.h` 重新生成配置）会静默丢失。全部改动均以 `LOCAL PATCH` 注释标记：

```bash
grep -rn "LOCAL PATCH" 04_Impl/impl_middleware/lvgl/
```

| 文件 | 改动 | 为什么不能丢 |
|---|---|---|
| `lvgl/src/misc/lv_mem.c` | `lv_mem_realloc()` 失败路径补打**请求尺寸** + `free`/`biggest`/`frag`，并提到 `LV_LOG_ERROR` 级 | 上游那句 `couldn't allocate memory` 不带任何尺寸，无法定位；上游在 `lv_mem_alloc()` 里的同类诊断是 `LV_LOG_INFO` 级，被本工程的 `LV_LOG_LEVEL = WARN` 过滤掉 |
| `lvgl/src/misc/lv_mem.c` + `.h` | 新增 `lv_mem_buf_dump()` / `lv_mem_buf_get_parked()` | `lv_mem_buf_release()` 只清 `used` 标志、**从不缩小槽位**，16 个槽各自停在历史最大尺寸并永久占池；这笔常驻开销在 `lv_mem_monitor()` 里完全不可见 |
| `lvgl/lv_conf.h` | `LV_MEM_POOL_INCLUDE` / `LV_MEM_POOL_ALLOC` 指向 `lv_port_mem_pool.c` | 内存池存储与 MPU 护栏的挂载点，丢了护栏就保护一块没人用的空内存 |
| `lvgl/lv_conf.h` | `LV_IMG_CACHE_DEF_SIZE` 4 → 1 | 抗碎片配置，见下节 |

### 为什么用 `LV_MEM_POOL_ALLOC` 而不是 `LV_MEM_ADR`

`LV_MEM_ADR` 看起来更直白（直接给地址），但**用不了**。`lv_mem.c` 里的判断是预处理指令：

```c
#if LV_MEM_ADR == 0
```

`#if` 只能求值整型常量表达式，**链接期符号地址不是**。把 `LV_MEM_ADR` 定义成 `&s_block.pool`，预处理器会把不认识的标识符当作 `0`，判断成立，于是走回默认的 `work_mem_int[]` 分支——**静默失效，且编译毫无报错**。`LV_MEM_POOL_ALLOC` 走 `#ifdef` 判断，不做求值，安全。

验证钩子是否生效：`work_mem_int` 必须从符号表消失（详见 [`../../05_Debug_Tool/README.md`](../../05_Debug_Tool/README.md) "验证它真的装上了"）。

### 内存池调优约束

池当前 32 KB（`LV_MEM_SIZE`），RAM 余量只剩约 6 KB，调整前先读 [`../../CLAUDE.md`](../../CLAUDE.md) "LVGL 内存池"节。两个已落地的抗碎片配置**不要随手改回去**：

- `LV_IMG_CACHE_DEF_SIZE = 1`（原 4）—— 缓存条目会让 extflash decoder 的 session 一直挂着，整读缓冲(≤3200 B)或 5 行预取缓冲(2400 B)随之常驻，4 条就占掉 32 KB 池里的 10–13 KB
- `LV_MEM_SIZE` 必须是 32 的倍数 —— MPU 护栏对齐要求，`lv_port_mem_pool.c` 里有静态断言兜底

## 替换

- 换 GUI 库 → 替换 `lvgl/`，重写 `lvgl_port/`
- 换日志后端 → 替换 `EasyLogger/`，保持 `DEBUG_OUT` 宏 API 不变（详见 [`../../05_Debug_Tool/`](../../05_Debug_Tool/)）
- 换 OTA 传输协议 → 替换 `Ymodem/`，service 层只依赖 `ota_transport_*` 抽象

LVGL 自定义 decoder + W25Q64 资源 bootstrap 细节见 [`../../CLAUDE.md`](../../CLAUDE.md) "外部 Flash LVGL 资源"节。
