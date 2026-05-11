# Commit Message 规范

本仓库使用 **Conventional Commits** 中文变体。所有提交必须遵守，CI / 代码审查会以此为依据。

## 格式

```
<type>(<scope>): <subject>
```

- 单行 subject，不写 body（除非必要，要写时空一行后再写）。
- subject 用 **中文**，结尾不加句号。
- 整条不超过 70 个字符（GitHub PR 列表显示也舒服）。
- 一条 commit 只做一件事；跨多个 type/scope 的改动必须拆分。

## type（必填，小写）

| type | 含义 | 用例 |
|---|---|---|
| `feat` | 新功能 / 新模块 / 新接口 | 新增传感器驱动、新增任务、新平台层接口 |
| `fix` | bug 修复 | 修内存越界、修死锁、修地址越界 |
| `refactor` | 重构（外部行为不变） | 重命名、目录调整、拆分文件 |
| `style` | 仅格式化（空白 / 缩进 / 注释排版） | clang-format、对齐调整 |
| `docs` | 文档 | README、CLAUDE.md、本规范文档 |
| `test` | 测试代码 / 测试任务 | mock test、硬件验证任务 |
| `build` | 构建系统、链接脚本、工具链配置 | Makefile、`.ld`、`startup_*.s`、`pyproject.toml` |
| `ci` | 持续集成相关 | `.github/workflows/*` |
| `chore` | 杂项（不影响代码运行） | 升级依赖、清理无用文件、调整 `.gitignore` |

## scope（必填，注意大小写）

代表本次改动的**主战场**。一条 commit 只允许一个 scope。

| scope | 范围 |
|---|---|
| `APP` | `03_Firmware/01_APP/` 下所有内容（业务代码、平台层、Middleware、Tools） |
| `Bootloader` | `03_Firmware/00_Bootloader/` 下所有内容 |
| `Firmware` | 跨 APP + Bootloader 的固件级共享改动（链接布局对齐、RTT 共址、外层 `c_cpp_properties.json` / workspace / CLAUDE.md） |
| `CI` | 仓库级 CI / workflow（搭配 type `ci` 使用，或当改动跨多个工程的 CI 配置时） |
| `Repo` | 仓库根目录级别（`.gitignore`、`README.md`、本规范文档等） |

> 当一条改动**确实横跨 APP 和 Bootloader** 时（例如同时改两边的 `.ld` 来对齐 Flash 布局），用 `Firmware` 作为 scope，而不是把改动塞进 `APP` 或 `Bootloader` 之一。

## subject

- **动词开头**："新增 / 完成 / 修复 / 更新 / 移除 / 重构 / 优化 / 修正 / 调整 / 添加 / 移植"。
- 描述**改了什么**，不描述**为什么改**（why 留给 PR 描述 / 代码注释）。
- 不写文件名，写模块名 / 功能点。
- 不缩写专有名词（W25Q64、EM7028、LVGL、FreeRTOS、ST7789、CST816T、AHT21、MPU6050 写全）。

## 示例

历史里的好例子：

```
feat(APP): 新增EM7028心率算法与计算任务
feat(APP): 完成W25Q64 Wrapper层
fix(APP): 修复Makefile
build(APP): 移除链接脚本READONLY兼容旧版GCC
ci(APP): 优化CI并增加工具链缓存
test(APP): EM7028硬件测试通过
refactor(APP): 修改目录结构
docs(APP): 更新README
```

跨工程改动应该这样写：

```
feat(Firmware): 重排内部Flash分配并打通Bootloader直跳APP链路
fix(Firmware): 统一RTT控制块地址解决Bootloader日志不显示
build(Firmware): 调整Bootloader与APP链接脚本到新Flash分区
ci(CI): 添加Bootloader构建产物
docs(Firmware): 更新CLAUDE.md并新增提交规范
```

反例（不要这样写）：

```
update something                       ← 没 type、没 scope、没说清楚
feat: 添加东西                          ← 缺 scope
fix(APP): fixed a bug in main.c        ← 应中文、应说模块名
feat(Bootloader): 加了直跳APP的逻辑、改了Flash地址、加了mem-report、修了RTT     ← 一条 commit 做四件事，必须拆
```

## 跨多次提交时的拆分原则

如果一次开发涉及多类改动，按以下优先级拆分：

1. 先 `build` / `ci`（底盘变化）
2. 再 `feat` / `fix` / `refactor`（功能变化）
3. 最后 `docs` / `style` / `test`（文档与样式）

每条 commit 都应该能独立构建通过。

## 工具

提交前可以用如下命令快速回顾：

```bash
git log --oneline -20                  # 看最近 20 条习惯
git log --pretty=format:'%h %s' -40    # 同上但更紧凑
```

如果不确定用哪个 type / scope，参考本文件或查最近 30 条历史。
