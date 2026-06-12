---
title: RTOS 接入注意事项
module: app
type: ref
tags: [freertos, bootloader, jump, type/ref, status/planned]
created: 2026-06-11
related: "[[Bootloader开发指南]], [[APP工程配置]]"
---

# RTOS 接入注意事项

本文档记录后续 APP 可能接入 RTOS 时需要遵守的启动和跳转约束。

> [!info] 当前状态
> 本阶段尚未接入 RTOS，本文档为规划性参考。后续接入 FreeRTOS 时应逐项验证。

---

## 1. APP 启动链路要保持完整

APP 后续如果会上 RTOS，建议保留启动文件中的：

```asm
bl __libc_init_array
```

原因是 RTOS 项目后续可能会引入：

```text
C++ 全局对象
静态对象
驱动框架初始化
库组件初始化
```

如果长期注释 `__libc_init_array`，这些对象的构造函数不会执行，后续问题会比较隐蔽。

当前 GCC/CLion 工程已经在 `XY7320/CMakeLists_template.txt` 中维护裸机参数：

```cmake
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
add_link_options(--specs=nano.specs --specs=nosys.specs -nostartfiles)
```

并通过 `minimal_runtime.c` 提供：

```c
void _init(void)
{
}

void _fini(void)
{
}
```

这样可以保留 `__libc_init_array`，同时避免 GCC 默认启动文件和复杂退出/析构注册逻辑带来的裸机启动问题。

> [!tip] 详见 [[APP工程配置#minimal_runtime.c 的作用]]

---

## 2. APP 入口必须重设向量表

APP 从 Bootloader 跳转进入，不等同于芯片真正复位。APP 入口建议保留：

```c
SCB->VTOR = 0x08010000U;
__enable_irq();
```

`SCB->VTOR` 用于确保中断向量表指向 APP 自己。

`__enable_irq()` 用于恢复 Bootloader 跳转前关闭的全局中断。

如果后续使用 FreeRTOS，`SysTick`、`PendSV`、`SVC` 都必须来自 APP 的向量表，否则调度器可能无法启动或直接 HardFault。

> [!tip] 详见 [[APP工程配置#向量表偏移]]

---

## 3. Bootloader 跳转前必须清场

Bootloader 跳转 APP 前应尽量清理自己留下的运行状态：

```text
关闭全局中断
停止 SysTick
清除 SysTick / PendSV pending
关闭所有 NVIC 中断
清除所有 NVIC pending
停止 USART DMA
反初始化 UART / DMA
设置 VTOR 到 APP 地址
设置 MSP 到 APP 栈顶
跳转 APP Reset_Handler
```

RTOS 对 `SysTick`、`PendSV`、`SVC` 很敏感，因此这部分比裸机 APP 更重要。

> [!tip] 详见 [[Bootloader开发指南#启动跳转流程]]

---

## 4. APP 不要假设硬件处于复位默认状态

从 Bootloader 跳进 APP 时，外设不一定处于真正上电复位状态。APP 应像独立程序一样完整初始化：

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_DMA_Init();
MX_USARTx_UART_Init();
MX_FREERTOS_Init();
vTaskStartScheduler();
```

不要依赖 Bootloader 中已经打开过的时钟、GPIO、USART 或 DMA。

> [!important] 核心原则
> APP 必须像"刚复位启动"一样完整初始化自己。
> Bootloader 必须在跳转前把中断、SysTick、DMA 清干净。

---

## 5. FreeRTOS 中断优先级要重新确认

STM32 + FreeRTOS 常见问题集中在中断优先级配置：

```c
configPRIO_BITS
configLIBRARY_LOWEST_INTERRUPT_PRIORITY
configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
configKERNEL_INTERRUPT_PRIORITY
configMAX_SYSCALL_INTERRUPT_PRIORITY
```

如果中断优先级配置错误，可能出现：

```text
HardFault
任务不调度
串口中断卡死
FreeRTOS 断言失败
```

尤其是 HAL UART DMA 中断里使用队列、信号量、通知等 RTOS API 时，该中断优先级不能高于 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 允许的范围。

> [!warning] 常见陷阱
> 在 DMA 中断服务函数中调用 FreeRTOS API（如 `xQueueSendFromISR`、`xSemaphoreGiveFromISR`）时，
> 该 DMA 中断的优先级必须在 FreeRTOS 可管理的范围内。
> 优先级数值（STM32 中数值越大优先级越低）不能小于 `configMAX_SYSCALL_INTERRUPT_PRIORITY`。

---

## 6. 栈和堆要按 RTOS 重新规划

RTOS APP 比裸机闪灯程序复杂得多，需要重新评估 RAM：

```text
启动栈 MSP
FreeRTOS heap
各任务栈
中断栈
HAL / 驱动缓冲区
```

链接脚本中当前类似：

```ld
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;
```

后续接入 RTOS 后可能需要调整，或者根据 FreeRTOS heap 实现方式重新规划。

> [!note] RAM 分配建议
> FreeRTOS 通常使用 `heap_4.c` 或 `configTOTAL_HEAP_SIZE` 管理动态内存。
> 需要确保 `_Min_Stack_Size` 足够支撑 MSP 启动栈（中断上下文），
> 同时 FreeRTOS heap 足够支撑所有任务的 TCB 和栈。

---

## 7. SysTick 归属要统一

FreeRTOS 通常会接管 `SysTick`。

Bootloader 跳 APP 前必须停止并清除自己的 SysTick 状态。

APP 中 `HAL_Init()` 会初始化 HAL tick；FreeRTOS 启动后，通常由 FreeRTOS tick 驱动。建议按 CubeMX 生成的 FreeRTOS 适配方式处理，不要手动混用两套 tick 机制。

> [!warning] 不要混用两套 tick
> HAL 默认使用 SysTick 作为时基（`HAL_Delay`、`HAL_GetTick`）。
> FreeRTOS 也需要 SysTick 作为调度时基。
> 两者不能同时使用 SysTick。
> CubeMX 生成 FreeRTOS 配置时通常会自动处理（将 HAL tick 改为其他定时器），需要确认。

---

## 8. 推荐验证路线

后续接入 RTOS 时，建议按下面顺序验证：

```text
1. 保持当前 Bootloader 跳最小 APP 稳定。
2. APP 保留 __libc_init_array，并使用维护后的 CMake 模板。
3. 在 APP 工程中接入 FreeRTOS。
4. 先确认 APP 直接 ST-LINK 烧到 0x08010000 能跑。
5. 再通过 Bootloader 升级这个 RTOS APP。
6. 如果 Bootloader 跳进去不跑，优先查 VTOR、SysTick、PendSV、SVC、NVIC pending。
```

> [!tip] 分步验证
> 不要一次性完成所有改动。每完成一步都要验证通过后再进行下一步。
> 特别是第 4 步（ST-LINK 直接烧录验证）必须先通过，再验证 Bootloader 跳转。
> 如果第 5 步失败但第 4 步通过，问题一定出在 Bootloader 跳转清场或 APP 启动链路。

---

## 核心原则总结

```text
APP 必须像"刚复位启动"一样完整初始化自己。
Bootloader 必须在跳转前把中断、SysTick、DMA 清干净。
```

---

## 相关文档

- [[Bootloader开发指南]] -- 跳转流程、清场步骤
- [[APP工程配置]] -- 向量表偏移、中断重开、GCC 裸机配置
