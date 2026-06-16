/**
 * @file    led_service.cpp
 * @brief   LED 服务实现
 *          通过 BspGpio_* 驱动板载 LED，支持 OFF/ON/BLINK/BREATHE 模式
 */

#include "led_service.h"
#include "bsp_tim_os.h"

LedService::LedService()
    : m_mode(Mode::OFF)
    , m_lastToggleTick(0)
    , m_blinkPeriodMs(500)
    , m_state(false)
{
    /* 硬件配置封装在 Services 层内部 */
    m_gpio.port = GPIOF;
    m_gpio.pin = GPIO_PIN_10;
    m_gpio.active_high = true;
}

LedService& LedService::Instance()
{
    static LedService instance;
    return instance;
}

void LedService::Init()
{
    BspGpio_Init(&m_gpio);
    TurnOff();
    m_mode = Mode::OFF;
    m_lastToggleTick = 0;
    m_state = false;
}

void LedService::SetMode(Mode mode)
{
    m_mode = mode;
    m_lastToggleTick = 0;

    if (mode == Mode::OFF) TurnOff();
    else if (mode == Mode::ON) TurnOn();
}

LedService::Mode LedService::GetMode() const
{
    return m_mode;
}

void LedService::Update()
{
    switch (m_mode) {
    case Mode::OFF:
        TurnOff();
        break;
    case Mode::ON:
        TurnOn();
        break;
    case Mode::BLINK:
        UpdateBlink();
        break;
    case Mode::BREATHE:
        /* TODO: 预留呼吸灯模式 */
        break;
    }
}

void LedService::TurnOn()
{
    BspGpio_On(&m_gpio);
    m_state = true;
}

void LedService::TurnOff()
{
    BspGpio_Off(&m_gpio);
    m_state = false;
}

void LedService::TurnToggle()
{
    BspGpio_Toggle(&m_gpio);
    m_state = !m_state;
}

void LedService::UpdateBlink()
{
    uint32_t now = BspTimOs_GetTick();
    if (now - m_lastToggleTick >= m_blinkPeriodMs) {
        m_lastToggleTick = now;
        BspGpio_Toggle(&m_gpio);
        m_state = !m_state;
    }
}
