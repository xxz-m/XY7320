//
// Created by Administrator on 2026/6/14.
//

#ifndef XY7320_LED_SERVICE_H
#define XY7320_LED_SERVICE_H
#include "bsp_gpio.h"
#include <cstdint>
class Led_service
{
public:
    enum class Mode{OFF,ON,BLINK,BREATHE};
    static Led_service& Instance();

    void Init();
    void Set_mode(Mode mode);
    Mode Get_mode() const;
    void Update();
private:
    Led_service();
    BspGpio_t m_gpio;
    Mode m_mode;
    uint32_t m_lastToggleTick;
    uint32_t m_blinkPeriodMs;
    bool m_state;

    void TurnOn();
    void TurnOff();
    void UpdateBlink();
    void TurnToggle();
};
#endif //XY7320_LED_SERVICE_H
