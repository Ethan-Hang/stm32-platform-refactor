# LVGL UI 资源外置 W25Q64 完全指南

> 适用工程：`03_Firmware/01_APP`（STM32F411CE + FreeRTOS + LVGL 8.3 + W25Q64 8MB SPI NOR）
> 现状：16 屏 GUI Guider UI 的 **41 张图片 + 9 套字体字形位图全部存于外部 flash**，固件 `.rodata` 零像素/零字形数据。

## 1. 为什么外置

| 资源 | 体积 | 内部 Flash 放得下吗 |
|---|---|---|
| 41 张图片（RGB565+Alpha） | ~1.1 MB | APP 槽总共 464 KB，放不下 |
| 9 套字体 glyph 位图 | ~480 KB | 同上 |
| 固件代码本身 | ~415 KB | 已占 APP 槽 ~90% |

外置后固件只携带**结构信息**（图片几何描述符、字体 cmap/glyph_dsc 表），像素和字形按需从 W25Q64 读取。代价是渲染路径多一跳 flash IO——本工程用三级缓存把这一跳的开销压到可接受（见 §5）。

## 2. 地址体系：三套地址空间

同一块数据，三个视角，三个地址。**搞混任何一个都会读错位置**：

| 视角 | LVGL 资源起点 | 谁在用 | 换算 |
|---|---|---|---|
| LVGL local（软件层） | `0x000000` | `Read_LvglData(addr, ...)`、`cfg_storage.h` 全部宏 | 基准 |
| W25Q64 物理 | `0x300000` | SPI2 驱动 | `storage_manager_task` 自动 `+0x300000` |
| JLink 虚拟（FLM） | `0x90000000` | JFlash / Ozone 烧录 | 自定义 `.FLM` 内部 `-0x90000000+0x300000` |

固件代码和打包脚本**只使用 LVGL local 地址**；物理偏移和 JLink 偏移分别由 service 层和 FLM 透明换算。FLM 的可写范围被锁死在 LVGL 分区（3 MB）内，烧资产不可能误伤 OTA / FlashDB 分区。

## 3. 单一事实源：`00_Config/inc/cfg_storage.h`

每个资产的 offset / 宽高 / 像素大小 / 字体位图 size 全部由这个头文件的宏锁定：

```c
#define CFG_LVGL_ASSET_BT32_OFFSET  (0x061000UL)   /* 4KB 扇区对齐槽位 */
#define CFG_LVGL_ASSET_BT32_W       (32U)
#define CFG_LVGL_ASSET_BT32_H       (32U)
#define CFG_LVGL_ASSET_BT32_PX_SIZE (3U)           /* RGB565 + alpha = 3 B/px */

#define CFG_LVGL_FONT_INTERTTF_16_BITMAP_OFFSET (0x10D000UL)
#define CFG_LVGL_FONT_INTERTTF_16_BITMAP_SIZE   (11603U)
```

- `Tools/pack_assets.py` 解析**同一个头文件**决定每个资产打包到 `assets.bin` 的哪个偏移；
- 固件渲染时按**同一组宏**计算读取地址。

两边永远一致，没有第二份布局表。注意宏必须写**字面量**（不能引用别的宏做别名），因为 pack 脚本的宏解析器只认字面量。

### Magic 配对机制

`CFG_LVGL_ASSET_MAGIC`（当前 `0xA55A5AAA`）写在资产包头 4 字节。**任何布局变更或内容字节序变更都必须 bump 这个值**。启动时 `storage_assets_bootstrap()` 读包头比对：

- 匹配 → 正常，顺带把 fen/time 两根表针 sprite 镜像进 RAM；
- 失配 → RTT 打错误日志提示重烧资产包，UI 照常启动（图片空白、文字缺字形，**不死机**）。

## 4. 两条数据通路

### 4.1 图片：`_ext` 描述符 + 自定义 decoder

```
setup_scr_*.c                 storage_assets.c                lv_port_extflash.c
lv_img_set_src(obj,    ──▶    const lv_img_dsc_t       ──▶   自定义 decoder
  &_BT32_alpha_32x32_ext)       _BT32_..._ext = {              info_cb: 认 magic
                                  .data = &meta,  ← 不指像素    open_cb: 整读或流式
                                  .data_size = 0,               read_line_cb: 预取
                                }                               close_cb: 释放
```

- `lv_img_dsc_t.data` 被**重载**为指向 `lv_extflash_meta_t`（含识别 magic + LVGL local offset + 几何信息）；
- decoder 的 `info_cb` 用 magic 认领"这张图归我管"，不认识的描述符放行给 LVGL 内置 decoder；
- `open_cb` 分两路：
  - **≤3200 B 小图**（32×32 及以下）：一次 `Read_LvglData` 整读进 LVGL 池缓冲，经 `img_data` 交给 LVGL；配合 `LV_IMG_CACHE_DEF_SIZE=4`，缓存命中的图标重绘**零 flash IO**；
  - **大图**：`img_data=NULL` 切行级流式，`read_line_cb` 经 5 行预取缓冲服务（280 行背景从 280 次往返降到 56 次）；
- 所有池分配失败都优雅降级（整读→流式，预取→单行直读），最坏情况只是慢，不会坏。

新图的描述符统一用 `storage_assets.c` 里的宏生成：

```c
DEFINE_EXTFLASH_IMAGE(_BT32_alpha_32x32, CFG_LVGL_ASSET_BT32);
```

例外：fen / time 两根表针 sprite 因旋转重绘频率高，启动时整体镜像进 RAM（`storage_assets.c` 单独定义，`data` 直指 RAM 镜像）。

### 4.2 字体：结构表留内部，位图外置 + 回调

GUI Guider 导出的字体 `.c` 做三处手术（见 §6.2 的逐步说明）：

1. `glyph_bitmap[]` 大数组用 `#if 0` 关掉——**源文本保留**（pack 脚本仍解析它打包），但不编译进固件；占位 `{ 0x00 }` 替身满足结构引用；
2. 文件头加 `#include "lv_port_extfont.h"`；
3. `lv_font_t.get_glyph_bitmap` 从 LVGL 默认函数换成 `lv_port_extfont_get_bitmap_<字体名>`。

渲染时 `lv_port_extfont.c` 按 `glyph_dsc->bitmap_index` 算出外部偏移读取字形，经两级缓冲：

- **≤512 B 小字形**：4 KB 静态 FIFO 缓存（48 条目，按 (font, glyphId) 键）——状态栏/标签每帧重绘的字形命中后零 flash IO；
- **大字形**（82px 时钟数字，最大 4275 B）：共享 scratch 缓冲（`CFG_LVGL_FONT_GLYPH_BUFFER_SIZE`=4608），秒级低频重绘直读。

cmap / glyph_dsc 结构表（合计 ~40 KB）留在内部 Flash——查字形 ID 零 IO，只有取位图才碰外部 flash。

## 5. 性能：为什么这样设计

`Read_LvglData` 是阻塞门面，每次调用经两次任务跳转往返（调用方 →事件组→ `storage_manager_task` →异步 API→ `w25q64_handler` →SPI→ 信号量原路返回），**单次往返 ~300µs，开销在次数不在字节数**。所以所有优化都围绕"减少调用次数"：

| 层级 | 机制 | 效果 |
|---|---|---|
| 小图 | 整读 + `LV_IMG_CACHE` | 图标首读 1 次往返，缓存命中 0 |
| 大图 | 5 行预取 + 会话级缓冲 | 全屏背景 280→56 次往返 |
| 小字形 | 4 KB FIFO 缓存 | 文字重绘 0 往返 |
| 大字形 | 直读 | 每秒 ~2 次，无需优化 |

## 6. 实操手册

### 6.1 新增一张图片（共 6 处）

以新图 `_foo_alpha_24x24`（24×24，alpha）为例：

1. **图片源**：GUI Guider/LVGL 转换器导出的 `_foo_alpha_24x24.c` 拷入
   `04_Impl/impl_middleware/lvgl/lvgl_ui/images/`（**只作 pack 解析源，不编译**——不要加进 Makefile）；
2. **布局**：`cfg_storage.h` 加一组宏（OFFSET 选一个空闲的 4 KB 对齐槽位，看现有最后一个资产的结尾；W/H/PX_SIZE 按实际）；
3. **打包**：`Tools/pack_assets.py` 的 `ASSETS` 列表加一行
   `("FOO", "_foo_alpha_24x24.c", "_foo_alpha_24x24_map"),`；
4. **描述符**：`storage_assets.c` 加 `DEFINE_EXTFLASH_IMAGE(_foo_alpha_24x24, CFG_LVGL_ASSET_FOO);`；
5. **声明**：`gui_guider.h` 加 `LV_IMG_DECLARE(_foo_alpha_24x24_ext);`，
   UI 代码引用 `&_foo_alpha_24x24_ext`（注意 `_ext` 后缀）；
6. **bump magic** + `make` + `make flash-assets` 配对烧录。

### 6.2 新增一套字体（共 6 处）

以 `lv_font_bar_20` 为例：

1. **字体源**：导出的 `lv_font_bar_20.c` 拷入
   `04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/`，做三处手术：
   - include 块后加 `#include "lv_port_extfont.h"`；
   - `glyph_bitmap[]` 用 `#if 0 /* Stored in W25Q64 by pack_assets.py. */ ... #else static ... glyph_bitmap[] = { 0x00 }; #endif` 包住；
   - `.get_glyph_bitmap = ...` 改为 `lv_port_extfont_get_bitmap_lv_font_bar_20,`；
2. **布局**：`cfg_storage.h` 加 `CFG_LVGL_FONT_BAR_20_BITMAP_OFFSET/_SIZE`
   （SIZE = 位图字节数，跑一次 `make pack-assets` 会校验，对不上会报实际值）；
3. **打包**：`pack_assets.py` 的 `FONTS` 列表加 `("BAR_20", "lv_font_bar_20.c"),`；
4. **回调**：`lv_port_extfont.c` 加
   `LV_EXTFONT_DEFINE(lv_font_bar_20, CFG_LVGL_FONT_BAR_20_BITMAP_OFFSET, CFG_LVGL_FONT_BAR_20_BITMAP_SIZE)`，
   `lv_port_extfont.h` 加对应原型；
5. **编译**：字体 `.c` **要**加进 Makefile `C_SOURCES`（结构表需要编译，区别于图片）；
6. **bump magic** + 配对烧录。检查最大字形是否超过
   `CFG_LVGL_FONT_GLYPH_BUFFER_SIZE`（4608 B），超了同步放大。

### 6.3 GUI Guider 重新导出整套 UI

1. generated 的屏幕代码拷入 `lvgl_ui/`；
2. 全部图片引用 `&_name` → `&_name_ext`（含 `gui_guider.h` 的 `LV_IMG_DECLARE`）；
3. 字体按 §6.2 步骤 1 逐个手术；
4. `setup_scr_Clock_3.c` / `widgets_init.c` 补 `#include "lv_analogclock.h"`（GUI Guider 不带）；
5. 新增/删除的资产按 §6.1/§6.2 同步四处登记；删掉的资产把宏、ASSETS 行、DEFINE、声明一并清理；
6. 活代码控件核对：`ui_hr_view`（under_up_label_1 / Heart_label_2）、`ui_temp_humi_view`（under_up_label_2）的控件名和字体字形覆盖（中文字形集由工程文案决定，文案改了字形跟着变）；
7. bump magic + 配对烧录。

### 6.4 影响字节序/像素格式的全局变更

改 `lv_conf.h` 的 `LV_COLOR_DEPTH` 或 `LV_COLOR_16_SWAP` 会改变打包字节序（pack 按这两个宏选 `.c` 里的条件分支）。当前 `SWAP=1`（面板字节序，flush 零拷贝的前提）。**改了必须 bump magic + 重打包重烧**，否则颜色全错。

## 7. 烧录与命令速查

```bash
make                # 编固件（不含像素/字形数据）
make pack-assets    # 解析 cfg_storage.h + lv_conf.h + 资产 .c → build/assets.bin
make flash-assets   # pack + JFlash 经自定义 .FLM 直写 W25Q64 LVGL 分区
make download       # JFlash 烧固件到内部 APP 槽
```

固件与资产包**必须配对**：动了任何资产相关内容，`make download` 和 `make flash-assets` 都要跑。只改业务代码不动资产时单烧固件即可。

## 8. 故障排查

| 现象 | 原因 | 处理 |
|---|---|---|
| RTT 报 "assets bootstrap: magic mismatch" | 资产包没烧/版本不配对 | `make flash-assets` |
| 图片全空白但文字正常 | 同上（图片走 decoder，magic 失配描述符读不到数据） | 同上 |
| 颜色整体反色/错乱 | 字节序不配对（SWAP 改了没重烧资产） | bump magic + 重打包重烧 |
| 个别字显示空白 | 字形超 `CFG_LVGL_FONT_GLYPH_BUFFER_SIZE`，回调拒绝（RTT 有 "glyph out of range"） | 放大该宏 |
| 文案改后个别中文不显示 | 字体字形集没覆盖新字符 | GUI Guider 里把文案写进控件重新导出字体 |
| pack-assets 报 "extracted N bytes, expected M" | cfg_storage.h 的 SIZE/几何宏与 `.c` 实际不符 | 按报错的实际值修宏 |
| 切屏明显变卡 | LVGL 池吃紧导致缓存/预取降级 | 看屏上 MEM 监视器，调小 `LV_IMG_CACHE_DEF_SIZE` 或排查池泄漏 |

## 9. 关键文件索引

| 文件 | 角色 |
|---|---|
| `00_Config/inc/cfg_storage.h` | 布局单一事实源 + magic |
| `Tools/pack_assets.py` | 资产打包（ASSETS / FONTS 清单） |
| `01_App/User_Sensor/storage/src/storage_assets.c` | `_ext` 描述符 + magic 校验 + 表针 RAM 镜像 |
| `04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_extflash.c` | 图片 decoder（整读/预取/流式） |
| `04_Impl/impl_middleware/lvgl/lvgl_port/src/lv_port_extfont.c` | 字体位图回调 + 字形缓存 |
| `02_Service/service_storage/src/storage_manager_task.c` | `Read_LvglData` 阻塞门面 + 地址换算 |
| `04_Impl/impl_middleware/lvgl/lvgl_ui/images/*.c` | 图片 pack 解析源（不编译） |
| `04_Impl/impl_middleware/lvgl/lvgl_ui/guider_fonts/*.c` | 字体（编译结构表，位图 `#if 0`） |
| `05_Common_Utils/01_Flash_Algorithm/W25Q64_8M_FLM.FLM` | JLink 烧录算法（地址换算 + 分区锁定；源码在 `std_program_algorithms/` Keil 工程，部署于 `%APPDATA%\SEGGER\JLinkDevices\ST\STM32F4\`） |
