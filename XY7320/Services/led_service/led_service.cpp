//
// Created by Administrator on 2026/6/14.
//

#include "led_service.h"
#include "stm32f4xx_hal.h"

Led_service::Led_service()
    : m_mode(Mode::OFF)
    , m_lastToggleTick(0)
    , m_blinkPeriodMs(500)
    , m_state(false)
{
    // 硬件配置封装在 Services 层内部
    m_gpio.port = GPIOF;
    m_gpio.pin = GPIO_PIN_10;
    m_gpio.active_high = true;
}
Led_service& Led_service::Instance()
{
    static Led_service instance;
    return instance;
}
void Led_service::Init()
{
    BSP_GPIO_Init(&m_gpio);
    TurnOff();
    m_mode = Mode::OFF;
    m_lastToggleTick = 0;
    m_state = false;
}

void Led_service::Set_mode(Mode mode) {
    m_mode = mode;
    m_lastToggleTick = 0;

    if (mode == Mode::OFF) TurnOff();
    else if (mode == Mode::ON) TurnOn();
}

Led_service::Mode Led_service::Get_mode() const
{
    return m_mode;
}

void Led_service::Update()
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
        // 预留
        break;
    }
}

void Led_service::TurnOn()
{
    BSP_GPIO_On(&m_gpio);
    m_state = true;
}

void Led_service::TurnOff()
{
    BSP_GPIO_Off(&m_gpio);
    m_state = false;
}

void Led_service::UpdateBlink() {
    uint32_t now = HAL_GetTick();
    if (now - m_lastToggleTick >= m_blinkPeriodMs) {
        m_lastToggleTick = now;
        BSP_GPIO_Toggle(&m_gpio);
        m_state = !m_state;
    }
}