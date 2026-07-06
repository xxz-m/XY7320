/**
 * @file    adc_service.h
 * @brief   ADC 采集服务
 *
 *          责任：
 *          - 包装 BspAdc，把硬件中断数据搬运给 Oscilloscope 滤波引擎
 *          - 维护 ModeManager 状态与 Oscilloscope 模式的对应关系
 *          - 提供原始快照（m_snapshot）与滤波结果（m_filteredResult）给上层
 *
 *          典型数据流：
 *              AdcService::Update (被 ModeManager 1ms Tick 驱动)
 *                └── BspAdc_IsDataReady ? ProcessTaskA/B : return
 *                      ├── memcpy(BSP 缓冲 → m_snapshot)
 *                      ├── Oscilloscope::TickLoop400_450 / TickLoopGSM
 *                      └── LOG_Printf 周期输出
 *                └── BspAdc_Resume (启动下一轮 DMA)
 *
 * @note    服务层单例，由 App_Main::App_Main_Init 调用 Init() 初始化。
 * @note    AdcService 不感知 ModeManager；上层通过 StartTaskA/B 间接驱动。
 */

#ifndef XY7320_ADC_SERVICE_H
#define XY7320_ADC_SERVICE_H

#include <stdint.h>
#include "Oscilloscope.h"
#include <cstring>

class AdcService {
public:
    /**
     * @brief ADC 服务工作模式
     */
    enum class Mode {
        Idle,   ///< 空闲：不采集
        TaskA,  ///< 400/450 模式：4 通道（CH0~CH3）采集 + Oscilloscope 400/450 滤波
        TaskB,  ///< GSM 模式：双通道（CH4~CH5）+ idle 通道（CH0~CH1） + Oscilloscope GSM 滤波
    };
    /**
     * @brief 当前 Oscilloscope 模式（决定走 4 通道还是 2 通道）
     */
    ScopeMode_t GetScopeMode() const { return m_scopeMode; }

    /**
     * @brief 设置 Oscilloscope 模式
     *
     * 模式切换时 Oscilloscope 内部会清空抗闪烁状态机；建议在 ModeManager state
     * 的 enter() 中调用，避免瞬时跳变。
     *
     * @param mode SCOPE_MODE_400_450 / SCOPE_MODE_GSM
     */
    void SetScopeMode(ScopeMode_t mode);

    /**
     * @brief 获取单例引用
     */
    static AdcService& Instance();

    /**
     * @brief 初始化 BSP ADC 与 Oscilloscope
     *
     * 调用 BspAdc_Init 绑定 hadc1 / htim3，初始化 Oscilloscope（默认 400/450 模式），
     * 自身 mode 进入 Idle。
     */
    void Init();

    /**
     * @brief 切换到 TaskA（400/450 四通道采集）
     *
     * 重启 BSP ADC。调用方应在进入该模式前调用
     * SetScopeMode(SCOPE_MODE_400_450)。
     */
    void StartTaskA();

    /**
     * @brief 切换到 TaskB（GSM 双通道采集）
     *
     * 重启 BSP ADC。调用方应在进入该模式前调用
     * SetScopeMode(SCOPE_MODE_GSM)。
     */
    void StartTaskB();

    /**
     * @brief 停止采集，回到 Idle
     */
    void Stop();

    /**
     * @brief 周期轮询入口（由 ModeManager Task_ModeManager 每 1ms 驱动）
     *
     * 流程：
     *  1. 若 mode_ == Idle，直接返回
     *  2. 检查 BspAdc_IsDataReady，未就绪直接返回
     *  3. 根据 mode_ 分发到 ProcessTaskA / ProcessTaskB
     *  4. 调用 BspAdc_Resume 启动下一轮
     */
    void Update();

    /**
     * @brief 获取当前 AdcService 模式
     */
    Mode CurrentMode() const { return mode_; }

    /**
     * @brief 获取最近一次 Oscilloscope 滤波结果
     *
     * 在 ProcessTaskA / ProcessTaskB 中填充，调用方应在下一轮 Update 之前取走。
     */
    ScopeResult_t GetFilteredResult() const { return m_filteredResult; }

private:
    AdcService() = default;

    /** TaskA 内部处理：拷贝 CH0~CH3 → TickLoop400_450 */
    void ProcessTaskA();

    /** TaskB 内部处理：拷贝 CH4~CH5 → TickLoopGSM */
    void ProcessTaskB();

    Mode mode_ = Mode::Idle;       ///< 当前服务模式
    ScopeMode_t m_scopeMode = SCOPE_MODE_400_450;       ///< 当前 Oscilloscope 模式
    ScopeResult_t m_filteredResult{};                   ///< 最新一次 Oscilloscope 结果
    uint16_t m_snapshot[6][ADC_SIZE_MAX];               ///< 原始数据快照（Oscilloscope 会原地修改）
};

#endif /* XY7320_ADC_SERVICE_H */
