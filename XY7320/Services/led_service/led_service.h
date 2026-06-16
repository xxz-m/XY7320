/**
 * @file    led_service.h
 * @brief   LED 服务
 *          管理板载 LED 的开关、闪烁、呼吸等工作模式
 */

#ifndef XY7320_LED_SERVICE_H
#define XY7320_LED_SERVICE_H

#include "bsp_gpio.h"
#include <cstdint>

/**
 * LED 服务
 *
 * 对外提供 LED 模式切换接口（OFF / ON / BLINK / BREATHE），
 * 内部通过 BspGpio_* 驱动硬件，调用方无需关心 GPIO 细节。
 *
 * @note 单例模式，通过 Instance() 获取实例
 */
class LedService
{
public:
    /** LED 工作模式 */
    enum class Mode { OFF, ON, BLINK, BREATHE };

    static LedService& Instance();

    /** 初始化 LED 硬件（关闭输出、清零状态） */
    void Init();

    /** 设置 LED 工作模式 */
    void SetMode(Mode mode);

    /** 获取当前 LED 工作模式 */
    Mode GetMode() const;

    /** 周期调用，驱动 LED 状态更新（闪烁等） */
    void Update();

private:
    LedService();

    BspGpio_t  m_gpio;
    Mode       m_mode;
    uint32_t   m_lastToggleTick;
    uint32_t   m_blinkPeriodMs;
    bool       m_state;

    void TurnOn();
    void TurnOff();
    void UpdateBlink();
    void TurnToggle();
};

#endif /* XY7320_LED_SERVICE_H */
