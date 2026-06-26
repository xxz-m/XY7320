//
// Created by Administrator on 2026/6/26.
//

#ifndef XY7320_ADC_SERVICE_H
#define XY7320_ADC_SERVICE_H
#include <stdint.h>

class AdcService {
public:
    enum class Mode {
        Idle,
        TaskA,
        TaskB,
    };
    static AdcService& Instance();
    void Init();
    void StartTaskA();
    void StartTaskB();
    void Stop();

    void Update();
    Mode CurrentMode() const { return mode_; }
private:
    AdcService() = default;

    void ProcessTaskA();
    void ProcessTaskB();

    Mode mode_ = Mode::Idle;
};

#endif //XY7320_ADC_SERVICE_H
