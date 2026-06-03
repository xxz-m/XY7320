/**
 * ============================================================================
 *  XY7320 Bootloader - APP 跳转模块（实现）
 * ============================================================================
 */

#include "boot_jump.h"

/**
 * ----------------------------------------------------------------------------
 *  Boot_IsValidApp - 检查 APP 固件是否合法
 * ----------------------------------------------------------------------------
 *
 *  判断原理：
 *
 *  APP 的 bin 文件布局：
 *  ┌──────────────────────┐ app_addr + 0x0000
 *  │   初始 MSP（栈顶）      │  ← 4 字节，必须落在 SRAM 内
 *  ├──────────────────────┤ app_addr + 0x0004
 *  │   Reset_Handler 地址  │  ← 4 字节，指向 APP 第一条指令
 *  ├──────────────────────┤
 *  │   ... 其他向量 ...     │
 *  └──────────────────────┘
 *
 *  例如一个合法的 APP：
 *    app_addr[0x00] = 0x20020000  ← MSP 指向 SRAM 顶部，合法
 *    app_addr[0x04] = 0x08010100  ← Reset_Handler，指向 APP 代码
 *
 *  空的 Flash（全 0xFF）：
 *    app_addr[0x00] = 0xFFFFFFFF  ← 不在 SRAM 范围内，非法
 *    app_addr[0x04] = 0xFFFFFFFF
 *
 *  注意：这里只检查 MSP 范围，不检查 Reset_Handler 是否合法。
 *        如果要更严格，可以额外检查 Reset_Handler 地址的 bit0 = 1（Thumb 模式）。
 */
int Boot_IsValidApp(uint32_t app_addr)
{
    /* 读取 APP 首地址的前 4 字节，即 APP 的初始主栈指针 */
    uint32_t stack_addr = *(volatile uint32_t *)app_addr;

    /* 初始 MSP 必须落在 SRAM 范围内 */
    if ((stack_addr >= SRAM_START_ADDR) && (stack_addr <= SRAM_END_ADDR))
    {
        return 1;   /* APP 有效 */
    }

    return 0;       /* APP 不存在或已损坏 */
}

/* ========================================================================
 *  Boot_JumpToApp - 跳转到 APP 运行
 * ========================================================================
 *
 *  跳转流程：
 *
 *  步骤 1: 读取 APP 入口信息
 *    - app_stack: APP 的初始栈顶地址（APP bin 前 4 字节）
 *    - app_reset: APP 的 Reset_Handler 地址（APP bin 第 5~8 字节）
 *
 *  步骤 2: 关闭所有中断
 *    - __disable_irq() → 关闭全局中断
 *    - 避免跳转过程中被中断打断导致状态混乱
 *
 *  步骤 3: 恢复外设到默认状态
 *    - HAL_RCC_DeInit() → 复位 RCC（HSI 作为系统时钟，关闭 HSE/PLL）
 *    - SysTick 清零     → 停止 SysTick 定时器
 *    - NVIC 清除        → 清除所有中断使能和挂起标志
 *    - HAL_DeInit()     → 反初始化所有 HAL 外设（GPIO、UART、DMA 等）
 *
 *  步骤 4: 设置 APP 运行环境
 *    - SCB->VTOR = app_addr  → 重定位中断向量表到 APP 地址
 *                              SVCall/PendSV/SysTick 等中断会使用 APP 的向量表
 *    - __set_MSP(app_stack)  → 设置主栈指针为 APP 初始栈顶
 *    - __set_CONTROL(0)      → 确保 CPU 处于特权模式 + 使用 MSP
 *
 *  步骤 5: 跳转执行
 *    - 将 app_reset 强制转换为函数指针
 *    - 调用该函数 → CPU 跳转到 APP 的 Reset_Handler
 *    - 从此 bootloader 不再运行，控制权完全交给 APP
 *
 *  为什么不需要 __enable_irq()？
 *    因为 APP 的 Reset_Handler 会自己重新初始化一切并开启中断。
 *    APP 端需要做：SCB->VTOR = 0x08010000; __enable_irq();
 * ======================================================================== */

typedef void (*pFunction)(void);

void Boot_JumpToApp(uint32_t app_addr)
{
    /* ---- 步骤 1: 读取 APP 入口信息 ---- */
    /* APP 首地址前 4 字节 = 初始 MSP */
    uint32_t app_stack = *(volatile uint32_t *)app_addr;
    /* APP 首地址第 5~8 字节 = Reset_Handler */
    uint32_t app_reset = *(volatile uint32_t *)(app_addr + 4);

    /* ---- 步骤 2: 关闭全局中断 ---- */
    __disable_irq();

    /* ---- 步骤 3: 恢复外设到默认状态 ---- */

    /* 复位 RCC：关闭 HSE、PLL，切换回 HSI */
    HAL_RCC_DeInit();

    /* 停止 SysTick 定时器，避免跳转过程中触发 SysTick 中断 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* 清除所有 NVIC 中断：ICER 关闭使能，ICPR 清除挂起 */
    for (uint8_t i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;   /* 关闭所有中断使能 */
        NVIC->ICPR[i] = 0xFFFFFFFF;   /* 清除所有中断挂起 */
    }

    /* 反初始化所有 HAL 外设 */
    HAL_DeInit();

    /* ---- 步骤 4: 设置 APP 运行环境 ---- */

    /* 重定位中断向量表到 APP 起始地址 */
    SCB->VTOR = app_addr;

    /* 设置主栈指针为 APP 初始栈顶 */
    __set_MSP(app_stack);

    /* 确保 CPU 处于特权线程模式 + 使用 MSP */
    __set_CONTROL(0);

    /* 内存屏障：确保以上设置生效后再跳转 */
    __ISB();
    __DSB();

    /* ---- 步骤 5: 跳转到 APP ---- */
    pFunction app_entry = (pFunction)app_reset;
    app_entry();

    /* 理论上不会执行到这里，如果到了说明 APP 跳转失败 */
    while (1) {}
}
