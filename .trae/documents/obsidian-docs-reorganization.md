# Obsidian 文档库重组方案

## 概述

对 XY7320_ST 项目的 `docs/` 目录进行全面重组，解决内容重叠、类型混杂、知识碎片化等问题，使用 Obsidian Flavored Markdown 特性（frontmatter、wikilinks、callouts、tags、embeds）构建一个阅读性、维护性、工程性兼备的技术文档库。

---

## 一、当前问题诊断

### 1.1 内容重叠热点

| 知识点 | 重复出现的文件 | 次数 |
|---|---|---|
| Flash 分区表（Bootloader 64KB / APP 960KB） | 链接脚本与启动流程、mOTA移植步骤、串口移植与升级流程规划、项目笔记、README、开发日志、boot_config.h | 7 |
| Bootloader 跳转 APP 流程（8步） | 链接脚本与启动流程、mOTA移植步骤、项目笔记、simple_update.c | 4 |
| APP 合法性判断（SRAM mask） | 链接脚本与启动流程、mOTA移植步骤、串口移植与升级流程规划、simple_update.c | 4 |
| SCB->VTOR + enable_irq | 链接脚本与启动流程、mOTA移植步骤、串口移植与升级流程规划、项目笔记 | 4 |
| USART1/USART2 分配 | 串口移植与升级流程规划、mOTA移植步骤、开发日志 | 3 |

### 1.2 文档类型混杂

| 文件 | 混合的类型 |
|---|---|
| `项目笔记.md` | 技术参考 + 操作指南 + 方法论 |
| `mOTA移植步骤.md`（1238行） | 永久参考 + 一次性操作记录 + 检查清单 |
| `串口移植与升级流程规划.md` | 规划 + 技术参考 + 操作指南 |

### 1.3 缺失的 Obsidian 特性

- 无 frontmatter（无法追踪状态、无法 Dataview 查询）
- 无 wikilinks（文档间靠手动路径引用）
- 无 callouts（重要警告无视觉区分）
- 无 tags（无法按主题筛选）
- 无 MOC（缺少统一导航入口）

---

## 二、新目录结构

```
docs/
├── 00-索引/
│   ├── 00-项目索引.md                       ← MOC 库首页
│   └── 标签定义.md                           ← tags 字典
│
├── 01-核心参考/
│   ├── 硬件参考.md                           ← Flash 分区 / Sector 表 / 串口 / 引脚
│   ├── Bootloader开发指南.md                  ← 链接脚本 / 跳转 / Flash操作 / 启动决策
│   ├── APP工程配置.md                        ← APP 链接脚本 / VTOR / minimal_runtime
│   └── RTOS接入注意事项.md                    ← FreeRTOS 约束清单
│
├── 02-mOTA与升级/
│   ├── 升级协议概述.md                       ← 升级架构总览 / 两条协议路线
│   ├── 简化升级协议.md                       ← simple_update 协议格式与实现
│   ├── mOTA移植指南.md                      ← 移植永久参考（从1238行拆分）
│   └── mOTA移植验收清单.md                   ← 三阶段 checklist
│
├── 03-上位机/
│   ├── Qt6架构设计.md                       ← 原 Qt6+CMake+QML 架构说明
│   └── XY7320Host上位机说明.md               ← 原串口升级上位机说明
│
├── 04-项目管理/
│   ├── 开发路线.md                           ← 原上下位机综合项目开发路线
│   ├── 系统架构设计.md                       ← 原项目系统结构与理解深度要求
│   ├── 开发日志.md                           ← 保留，加 frontmatter
│   └── 踩坑记录.md                           ← 保留，加 frontmatter
│
├── 05-工程工具/
│   ├── CMake与STM32工程笔记.md               ← 原 STM32_CMake 笔记，去重
│   └── GCC裸机开发笔记.md                    ← 新文档，从踩坑+项目笔记提取
│
└── _templates/
    ├── 技术参考模板.md
    └── 踩坑记录模板.md
```

**设计原则**：
1. **编号前缀** `00-`~`05-`：保证 Obsidian 文件列表按逻辑排序
2. **单一信息源**：每个知识点只在一个文件完整描述，其他地方用 `![[文档#标题]]` 嵌入或 `[[文档]]` 链接
3. **类型分离**：参考文档、操作指南、日志、规划严格分开
4. **代码对应**：`01-核心参考/` 对应 `Core/`，`02-mOTA与升级/` 对应 `MOTA/`

---

## 三、每个文档的定位与内容来源

### 3.1 `00-索引/00-项目索引.md` — 库首页 MOC

**定位**：整个 Obsidian 库的入口

**内容**：
- 项目一句话描述
- 当前工程状态（callout）
- 按模块分组的 wikilink 导航表
- Flash 分区速览（嵌入 `![[硬件参考#Flash 分区表]]`）
- 快速构建命令

### 3.2 `00-索引/标签定义.md` — 标签字典

**标签体系**：
- 主题标签：`#stm32` `#bootloader` `#app-fw` `#mota` `#uart` `#flash` `#cmake` `#qt` `#upgrade` `#linker` `#gcc` `#dma` `#freertos`
- 类型标签：`#type/ref` `#type/guide` `#type/log` `#type/plan` `#type/issue`
- 状态标签：`#status/current` `#status/completed` `#status/planned`

### 3.3 `01-核心参考/硬件参考.md`

**定位**：STM32F407ZGTx 硬件唯一参考源

**内容**：
- Flash 分区表（唯一权威定义）
- Flash Sector 0~11 详细表
- Flash 操作特性（擦除规则、写入限制）
- 串口分配（USART1 日志 / USART2 升级）
- 引脚分配表（PB8 按键、PF10 LED 等）
- APP 有效性判断方法

**来源**：整合自 `链接脚本与启动流程.md`、`mOTA移植步骤.md`、`项目笔记.md`

### 3.4 `01-核心参考/Bootloader开发指南.md`

**定位**：Bootloader 开发核心技术参考

**内容**：
- 链接脚本配置（64KB 限制）
- 启动跳转流程（8步，唯一权威描述）
- Flash 读写擦除实现要点
- mOTA Flash 调用链
- 启动决策逻辑（按键消抖、超时、分支）
- boot_config.h 与 bootloader_config.h 的区别

**来源**：整合自 `链接脚本与启动流程.md`、`mOTA移植步骤.md` 2.6~2.8 节、`开发日志.md` 2026-06-08

**Obsidian 特性**：
- `![[硬件参考#Flash 分区表]]` 嵌入分区表
- `> [!warning]` 标注 Bootloader 区保护
- `> [!tip]` 标注调试建议

### 3.5 `01-核心参考/APP工程配置.md`

**定位**：APP 工程配置参考

**内容**：
- 链接脚本修改（0x08010000, 960K）
- 向量表偏移（SCB->VTOR）
- 中断重开（__enable_irq）
- GCC 裸机链接配置（nano.specs / nosys.specs / nostartfiles）
- minimal_runtime.c 的作用
- APP main() 开头初始化序列

**来源**：整合自 `链接脚本与启动流程.md`、`mOTA移植步骤.md` 1.4 节、`踩坑记录.md`、`项目笔记.md`

### 3.6 `01-核心参考/RTOS接入注意事项.md`

**定位**：FreeRTOS 接入约束清单

**内容**：完整保留原 `项目笔记.md` 中 8 项 RTOS 注意事项

**来源**：从 `项目笔记.md` 第 223~381 行拆出

### 3.7 `02-mOTA与升级/升级协议概述.md`

**定位**：升级体系全局视图

**内容**：
- 两条路线（简化协议 vs mOTA/YModem）
- 四阶段开发路线
- 串口分配
- mOTA 层级调用链
- 升级标志机制（update_flag / .noinit 段）
- 当前工程状态

**来源**：整合自 `串口移植与升级流程规划.md`、`项目笔记.md`

### 3.8 `02-mOTA与升级/简化升级协议.md`

**定位**：当前已实现的简化升级协议完整说明

**内容**：
- 12 字节头包协议格式
- 升级时序
- 状态机（WAIT_HEAD -> RECV_APP -> DONE / ERROR）
- DMA 双 buffer 方案
- 上位机配合参数

**来源**：整合自 `README.md`、`串口移植与升级流程规划.md` 第 4 节、`踩坑记录.md`

### 3.9 `02-mOTA与升级/mOTA移植指南.md`

**定位**：mOTA 移植永久性技术参考

**内容**（从原 1238 行 `mOTA移植步骤.md` 精简）：
- 移植原则
- 目录结构与文件说明
- bootloader_config.h 配置说明
- BSP Flash 移植
- bootloader_port.c 移植
- 串口移植步骤
- 完整 mOTA/YModem 接入
- 裁剪建议

**移除的内容**：
- 一次性编译排错 → `踩坑记录.md` / `GCC裸机开发笔记.md`
- 重复分区表/跳转流程 → 用 wikilink 替代
- 验收 checklist → `mOTA移植验收清单.md`

### 3.10 `02-mOTA与升级/mOTA移植验收清单.md`

**定位**：三阶段验收标准追踪

**内容**：使用 Obsidian checkbox 格式的阶段一/二/三验收清单

### 3.11 `03-上位机/` — 基本保留原内容

- `Qt6架构设计.md`：原 `Qt6+CMake+QML工业项目架构说明.md`，加 frontmatter + wikilinks
- `XY7320Host上位机说明.md`：原 `XY7320Host串口升级上位机说明.md`，加 frontmatter + `[[简化升级协议]]` wikilink

### 3.12 `04-项目管理/` — 保留 + 增强

- `开发路线.md`：原内容 + frontmatter + wikilinks 到技术文档
- `系统架构设计.md`：改标题 + frontmatter
- `开发日志.md`：加 frontmatter，内部引用改 wikilinks
- `踩坑记录.md`：加 frontmatter，GCC 知识同步到 `GCC裸机开发笔记.md`

### 3.13 `05-工程工具/`

- `CMake与STM32工程笔记.md`：原 `STM32_CMake笔记.md`，移除链接脚本章节（已在 `Bootloader开发指南.md`）
- `GCC裸机开发笔记.md`（新）：从 `踩坑记录.md`、`项目笔记.md`、`mOTA移植步骤.md` 提取 GCC/newlib 裸机开发知识

### 3.14 `_templates/` — Obsidian 模板

- `技术参考模板.md`：标准 frontmatter + 结构
- `踩坑记录模板.md`：现象/根因/解决/经验

---

## 四、迁移映射表

| 旧文档 | 内容去向 | 操作 |
|---|---|---|
| `cmake-stm32/STM32_CMake笔记.md` | 链接脚本章节 → `Bootloader开发指南.md`；其余 → `CMake与STM32工程笔记.md` | 拆分后删除 |
| `cmake-stm32/链接脚本与启动流程.md` | Flash 分区/Sector 表 → `硬件参考.md`；链接脚本/跳转 → `Bootloader开发指南.md`；APP 链接脚本 → `APP工程配置.md` | 拆分后删除 |
| `mota/mOTA移植步骤.md` | 永久参考 → `mOTA移植指南.md`；验收标准 → `mOTA移植验收清单.md`；编译排错 → `GCC裸机开发笔记.md` / `踩坑记录.md` | 拆分后删除 |
| `mota/串口移植与升级流程规划.md` | 四阶段路线/串口分配 → `升级协议概述.md`；串口步骤 → `mOTA移植指南.md`；协议格式 → `简化升级协议.md` | 拆分后删除 |
| `project-docs/项目笔记.md` | RTOS 注意事项 → `RTOS接入注意事项.md`；GCC 配置 → `GCC裸机开发笔记.md`；其余已在各参考文档中 | 拆分后删除 |
| `project-docs/项目结构.md` | 由 `00-项目索引.md` 承担 | 删除 |
| `project-docs/上下位机综合项目开发路线.md` | → `开发路线.md` | 重命名 + 增强 |
| `project-docs/原项目系统结构与理解深度要求.md` | → `系统架构设计.md` | 重命名 + frontmatter |
| `project-docs/开发日志.md` | → `开发日志.md` | 加 frontmatter + wikilinks |
| `project-docs/踩坑记录.md` | → `踩坑记录.md` | 加 frontmatter |
| `qt/Qt6+CMake+QML工业项目架构说明.md` | → `Qt6架构设计.md` | 重命名 + frontmatter |
| `qt/XY7320Host串口升级上位机说明.md` | → `XY7320Host上位机说明.md` | 重命名 + frontmatter + wikilinks |
| `README.md`（根目录） | 保留为 Git 仓库 README（精简版），完整 MOC 在 `00-项目索引.md` | 精简 |

---

## 五、Obsidian 特性使用规范

### 5.1 Frontmatter（每个文档必须包含）

```yaml
---
title: 文档标题
module: 所属模块  # bootloader / app / mota / upgrade / qt / hardware / toolchain / project
type: 文档类型    # ref / guide / log / plan / checklist / moc
tags: [tag1, tag2]
created: YYYY-MM-DD
related: "[[文档A]], [[文档B]]"
---
```

### 5.2 Wikilinks

- 引用文档：`[[Bootloader开发指南]]`
- 引用章节：`[[Bootloader开发指南#启动跳转流程]]`
- 嵌入章节（避免重复）：`![[硬件参考#Flash 分区表]]`

### 5.3 Callouts

- `> [!warning]` 危险操作（如 Bootloader 区保护）
- `> [!tip]` 调试建议
- `> [!info]` 当前状态
- `> [!note]` 设计决策
- `> [!bug]` 已知问题

---

## 六、执行步骤

1. **创建新目录结构**（`00-索引/` ~ `05-工程工具/`、`_templates/`）
2. **创建索引文档**（`00-项目索引.md`、`标签定义.md`）
3. **创建核心参考文档**（`硬件参考.md`、`Bootloader开发指南.md`、`APP工程配置.md`、`RTOS接入注意事项.md`）— 从旧文档中提取整合内容
4. **创建 mOTA 与升级文档**（`升级协议概述.md`、`简化升级协议.md`、`mOTA移植指南.md`、`mOTA移植验收清单.md`）
5. **迁移上位机文档**（重命名 + 加 frontmatter + wikilinks）
6. **迁移项目管理文档**（重命名 + 加 frontmatter + wikilinks）
7. **创建工程工具文档**（`CMake与STM32工程笔记.md` 去重、`GCC裸机开发笔记.md` 新建）
8. **创建模板文件**
9. **更新根目录 README.md**（精简为 Git 仓库说明）
10. **验证所有 wikilinks 无断链，删除旧文件和旧目录**

---

## 七、假设与决策

- **假设**：所有现有文档内容都有保留价值，不存在应直接丢弃的内容
- **假设**：用户希望保留中文文档风格
- **决策**：根目录 `README.md` 保留为 Git 仓库 README（精简版），不删除
- **决策**：`mOTA/README.md` 和 `mOTA/document/*.pdf` 属于第三方参考，不纳入重组范围
- **决策**：`Drivers/` 目录下的 ST HAL 驱动文件为官方代码，不纳入文档范围

---

## 八、验证步骤

1. 在 Obsidian 中打开 docs/ 目录作为 Vault
2. 检查 `00-项目索引.md` 的所有 wikilinks 是否可达
3. 使用 Obsidian Graph View 查看文档关联图
4. 搜索每个旧文档中的关键知识点，确认在新文档中可找到
5. 确认无重复内容（Flash 分区表只在一个地方完整出现）
6. 确认所有 frontmatter 格式正确
