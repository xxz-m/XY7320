---
title: Oscilloscope 算法说明（waveLimitFilter）
tags:
  - docs/spec
  - filter
  - oscilloscope
status: active
---

# Oscilloscope 算法说明 — waveLimitFilter

## 1. 文档目的

帮助维护者理解 `Domain/filter/Oscilloscope.cpp` 中 `waveLimitFilter` 的真实执行流程，纠正两种常见误解：

- ❌ “先排序后丢掉小于平均的点”—— **错**。本算法不排序。
- ❌ “加权平均”—— **错**。所有点权重相同，是普通算术平均。

本文档配套 `docs/30-工程资料/STM32/waveLimitFilter动画.html` 使用。

---

## 2. 算法核心一句话

> **斩波迭代 → 排序 → 截尾 → 算术平均**

每一步对应代码里的具体函数。

---

## 3. 完整算法流程（伪代码）

```cpp
templen = len;
avg = limit;

for j in 0..N-1:                    // ← 第 1 步：斩波迭代（不排序）
    templen2 = 0
    for i in 0..templen-1:
        if in[i] >= avg:            // 砍掉小于阈值的点
            midIn[templen2++] = in[i]
    templen = templen2

    if templen < 10:                // 剩余太少 → 无信号
        return 0

    for k in 0..templen-1:
        in[k] = midIn[k]            // 收缩 in 数组
        sum += midIn[k]

    avg = sum / templen             // 阈值抬升到本轮均值

return select_sort(in, templen)     // ← 第 2+3+4 步：排序+截尾+平均
```

---

## 4. 四步详解

### 第 1 步：斩波迭代（核心：阈值逐步抬升）

**每轮做的事情**：

1. 用当前阈值 `avg` 砍一遍，**只保留 `in[i] >= avg` 的点**。
2. 计算这一轮剩余点的均值。
3. **把阈值 `avg` 抬升到刚才算出的均值**，进入下一轮。

**关键**：**不是排序，是按阈值筛**。每一轮把“低分”砍掉，门槛越抬越高，剩下的全是“稳定信号段”。

**为什么叫斩波**：

```
初始门槛 100   ─→  把 < 100 的全砍
                 剩下 8 个，平均 845
门槛抬到 845    ─→  把 < 845 的全砍
                 剩下 5 个，平均 1390
门槛抬到 1390   ─→  把 < 1390 的全砍
                 剩下 3 个，平均 1550
门槛抬到 1550   ─→  把 < 1550 的全砍
                 剩下 1 个，平均 1700
```

**为什么能去底噪**：

- 第一轮 `limit` 通常是个小值（默认 100），把底噪点全部砍掉。
- 几轮迭代 `avg` 抬到脉冲幅度附近，剩余点全是“脉冲顶部”的样本。
- **没信号时**（数据本来就小），砍完后剩余 < 10 → 返回 0 → “信号消失检测”。

### 第 2 步：排序

仅在最后一轮后做一次。

`select_sort` 把 `in[]` 用**选择排序**从大到小排序。**这一步只为第 3 步服务**，不影响滤波主流程。

### 第 3 步：截尾

去掉排序后的：

- 前 2 个最大值
- 后 2 个最小值

**为什么截尾**：脉冲顶部偶尔会冒出**尖刺 / 凹陷**（毛刺、电源干扰），截尾抑制头尾异常值。

```
原始顶部:  1500
采集:      1500 1520 1480 1510 2200 1490 1470 1500 1380
                                      ↑              ↑
                                    尖刺          凹陷
```

排序后：

```
排序:  2200 1520 1510 1500 1500 1490 1480 1470 1380
```

去头 2 + 去尾 2：

```
中间:  1520 1510 1500 1500 1490 1480 1470
```

### 第 4 步：算术平均

对截尾后的中间点求算术平均，返回 `uint16_t`。

**注意**：所有点权重相同，**不是加权平均**。

---

## 5. 算法适用场景

XY7320 是**测试仪器**（POWER / MOHMS / CALI / GSM 模式），采集的物理信号特征：

- 大部分时间是底噪（应该判为 0）
- 测试时输出**脉冲 / 矩形波**
- 脉冲顶部叠加高频噪声

| 场景 | 本算法效果 |
|------|-----------|
| 长时间底噪 | 第一轮砍光 → 返回 0（信号消失） |
| 脉冲信号 + 噪声 | 斩波砍底噪 → 排序去头尾毛刺 → 返回稳定脉冲顶部代表值 |
| 偶发尖刺 | select_sort 截尾剔除 |
| 偶发凹陷 | select_sort 截尾剔除 |

---

## 6. 与其他常见均值类算法对比

| 算法 | 原理 | 矩形信号上效果 |
|------|------|---------------|
| 算术平均 | 所有点直接求平均 | 底噪拉低脉冲值，结果偏低 |
| 滑动平均 | 滑动窗口求平均 | 边沿模糊，脉冲顶部被平均拉低 |
| 中位值 | 排序取中位 | 抗尖刺好，但不抗持续噪声 |
| **加权平均** | 不同点不同权重 | 权重设计难，**本项目不用** |
| **斩波 + 截尾均值** | 砍点 + 抬阈值 + 排序去头尾 | **本项目用法**：砍底噪、抗毛刺、稳定脉冲代表值 |

---

## 7. 关键工程结论

### 7.1 `waveLimitFilter` 返回值

| 输入情况 | 返回值 |
|---------|--------|
| `in == nullptr` 或 `len == 0` | `0` |
| 砍完剩余 < 10 | `0`（**信号消失**） |
| `len <= 4` | 全部点的算术平均 |
| `len > 4` | 排序去头尾各 2 后中间点算术平均 |

**返回 `0` 不一定是错误，可能是“无信号”，调用方需要结合业务语义判断**。

### 7.2 与 `waveLimitFilter_x` 的区别

`waveLimitFilter_x` = `waveLimitFilter` + **抗闪烁保持 + 跳变确认**：

- 斩波 + 截尾后多一段后处理
- 连续 10 帧零值才归零（短空档保持）
- 大跳变（>500）需要连续 2 帧才接受（防瞬时毛刺）
- 维护跨帧状态 `WaveHoldState`

CH1 / CH5 用升级版，因为这两个通道最容易出瞬时跳变（档位切换 / 接触电阻变化 / 电源瞬态）。

### 7.3 阈值选型经验

| 参数 | 默认值 | 调整方向 |
|------|--------|---------|
| `limit`（`TriggerLevel`） | 100 | 提高可加速砍底噪；过低会保留噪声 |
| `Threshold1..4`（400/450） | 300/600/300/600 | 对应不同前端放大倍数；CH2/CH4 放大更高，阈值更高 |
| `Threshold5/6`（GSM） | 600/1500 | GSM 信号噪声边界高，CH6 阈值更高 |

**注意**：阈值**调太高**会让真实信号被砍 → `templen < 10` → 返回 0。**这是工程上最容易踩的坑**。

### 7.4 CPU 开销

单次 `TickLoop400_450` 调用：

- 4 × `preprocessChannel`（O(len)）
- 1 × `waveLimitFilter_x`（斩波 + select_sort + 后处理）
- 3 × `waveLimitFilter`（斩波 + select_sort）

每轮斩波 + 排序在 512 点上约 1.5M 比较，STM32F407 @168MHz 上约几 ms。

**建议**：

- 1ms Tick 调用 Update 时，Oscilloscope 实际跑 1kHz。
- 实测 CPU 占用偏高时，给 Oscilloscope 加 100Hz 分频。

---

## 8. 调用路径速查

```text
ModeManager::Task_ModeManager (1ms Tick)
  └── ModeManager::Tick
        └── TaskStateAdcA::tick() / TaskStateAdcB::tick()
              └── AdcService::Update
                    ├── memcpy BSP → AdcService::m_snapshot[6][512]
                    ├── Oscilloscope::TickLoop400_450 / TickLoopGSM
                    │     ├── preprocessChannel × N
                    │     ├── waveLimitFilter_x (CH1/CH5)
                    │     │     ├── 斩波迭代
                    │     │     └── select_sort (排序 + 截尾 + 平均)
                    │     └── waveLimitFilter (其他通道)
                    │           ├── 斩波迭代
                    │           └── select_sort
                    └── LOG_Printf 周期输出
```

---

## 9. 相关文档

- `Domain/filter/Oscilloscope.h` — 接口定义
- `Domain/filter/Oscilloscope.cpp` — 算法实现
- `docs/30-工程资料/STM32/Oscilloscope接入方案.md` — AdcService 接入方案
- `docs/30-工程资料/STM32/waveLimitFilter动画.html` — 算法可视化动画