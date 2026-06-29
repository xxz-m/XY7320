/**
 * @file    Oscilloscope.h
 * @brief   多通道信号滤波引擎
 *          支持 400/450 四通道和 GSM 双通道两种互斥模式，
 *          提供截尾均值、斩波均值、抗闪烁保持等多种滤波算法。
 */

#ifndef XY7320_OSCILLOSCOPE_H
#define XY7320_OSCILLOSCOPE_H

#include <stdint.h>
#include <stdbool.h>

/** 单次采样最大点数 */
#define ADC_SIZE_MAX 512

/**
 * 滤波工作模式
 *
 * 400/450 和 GSM 两种模式互斥，同一时刻只能运行一种。
 * 切换模式时会清空所有滤波状态，避免跨模式干扰。
 */
typedef enum
{
    SCOPE_MODE_400_450 = 0,  ///< 400/450 四通道模式
    SCOPE_MODE_GSM,          ///< GSM 双通道模式
} ScopeMode_t;

/**
 * 单通道滤波状态
 *
 * 用于抗闪烁和跳变抑制。核心机制：
 * - 连续空档达到阈值才归零（短空档保持）
 * - 大跳变需要连续确认才接受（防止瞬时毛刺）
 */
typedef struct
{
    uint16_t lastNonZero;  ///< 最近一次有效非零值
    uint8_t  zeroCnt;      ///< 连续零值计数
    uint8_t  sig;          ///< 信号确认标志（0=无信号, 1=有信号）
    uint16_t cand;         ///< 跳变候选值
    uint8_t  candCnt;      ///< 候选连续出现次数
} WaveHoldState;

/**
 * 400/450 模式阈值配置
 *
 * 每个通道独立配置底噪抑制阈值，适应不同前端放大倍数。
 */
typedef struct
{
    uint16_t Threshold1;  ///< CH1 底噪阈值
    uint16_t Threshold2;  ///< CH2 底噪阈值
    uint16_t Threshold3;  ///< CH3 底噪阈值
    uint16_t Threshold4;  ///< CH4 底噪阈值
} ScopeConfig400_450_t;

/**
 * GSM 模式阈值配置
 *
 * GSM 通道噪声边界通常更高，阈值单独配置。
 */
typedef struct
{
    uint16_t Threshold5;  ///< CH5 底噪阈值
    uint16_t Threshold6;  ///< CH6 底噪阈值
} ScopeConfigGSM_t;

/**
 * 滤波器总配置
 *
 * 包含采样参数、当前模式、以及各模式的阈值配置。
 */
typedef struct
{
    uint16_t SampleNum;      ///< 采样点数（不超过 ADC_SIZE_MAX）
    uint16_t TriggerLevel;   ///< 斩波滤波初始阈值
    uint16_t WavePEPNum;     ///< 预留平均窗口（暂未使用）
    ScopeMode_t Mode;        ///< 当前工作模式

    ScopeConfig400_450_t cfg400_450;  ///< 400/450 模式阈值
    ScopeConfigGSM_t cfgGSM;          ///< GSM 模式阈值
} ScopeConfig_t;

/**
 * 滤波输出结果
 *
 * 保存最近一次滤波计算结果。
 * 400/450 模式使用 wavePEP1~4，GSM 模式使用 wavePEP5~6。
 */
typedef struct
{
    uint16_t wavePEP1_avg;  ///< CH1 滤波结果
    uint16_t wavePEP2_avg;  ///< CH2 滤波结果
    uint16_t wavePEP3_avg;  ///< CH3 滤波结果
    uint16_t wavePEP4_avg;  ///< CH4 滤波结果
    uint16_t wavePEP5_avg;  ///< CH5 滤波结果（GSM）
    uint16_t wavePEP6_avg;  ///< CH6 滤波结果（GSM）
} ScopeResult_t;

/**
 * 多通道信号滤波引擎
 *
 * 单例模式，提供多种滤波算法和模式管理。
 * 典型用法：
 * @code
 * auto& scope = Oscilloscope::getInstance();
 * scope.initOscilloscope();
 * scope.setMode(SCOPE_MODE_400_450);
 * scope.TickLoop400_450(ch1, ch2, ch3, ch4);
 * auto result = scope.getResult();
 * @endcode
 */
class Oscilloscope
{
public:
    /** 获取单例引用 */
    static Oscilloscope& getInstance();

    /**
     * @brief  初始化滤波器
     *
     * 设置默认配置（400/450 模式、512 采样点、默认阈值），
     * 清空所有滤波状态和输出结果。
     */
    void initOscilloscope();

    /**
     * @brief  切换工作模式
     * @param  mode  目标模式（SCOPE_MODE_400_450 或 SCOPE_MODE_GSM）
     *
     * 切换时会清空所有滤波状态，确保两种模式互不干扰。
     */
    void setMode(ScopeMode_t mode);

    /**
     * @brief  400/450 四通道滤波
     * @param  adcCH1  CH1 采样数据（长度 = SampleNum）
     * @param  adcCH2  CH2 采样数据
     * @param  adcCH3  CH3 采样数据
     * @param  adcCH4  CH4 采样数据
     *
     * 前置条件：Mode 必须为 SCOPE_MODE_400_450，否则直接返回。
     *
     * 处理流程：
     * 1. 按通道阈值清零底噪
     * 2. CH1 使用增强滤波（抗闪烁），CH2~4 使用普通斩波滤波
     * 3. 结果写入 m_result.wavePEP1~4
     */
    void TickLoop400_450(uint16_t *adcCH1,
                         uint16_t *adcCH2,
                         uint16_t *adcCH3,
                         uint16_t *adcCH4);

    /**
     * @brief  GSM 双通道滤波
     * @param  adcCH5  CH5 采样数据（长度 = SampleNum）
     * @param  adcCH6  CH6 采样数据
     *
     * 前置条件：Mode 必须为 SCOPE_MODE_GSM，否则直接返回。
     *
     * 处理流程：
     * 1. 按通道阈值清零底噪
     * 2. CH5 使用增强滤波，CH6 使用普通斩波滤波
     * 3. 结果写入 m_result.wavePEP5~6
     */
    void TickLoopGSM(uint16_t *adcCH5,
                     uint16_t *adcCH6);

    /** @brief  获取当前配置 */
    ScopeConfig_t getConfig() const;

    /** @brief  获取最近一次滤波结果 */
    ScopeResult_t getResult() const;

private:
    Oscilloscope() = default;

    /**
     * @brief  通道预处理：低于阈值的采样点清零
     * @param  adcData    采样数据（原地修改）
     * @param  threshold  底噪阈值
     */
    void preprocessChannel(uint16_t *adcData, uint16_t threshold);

    /**
     * @brief  截尾均值
     * @param  a    输入数组（会被排序）
     * @param  len  数组长度
     * @return 去掉两端异常值后的均值
     *
     * 排序后去掉前 2 和后 2 个值，对中间部分求平均。
     * 若长度 <= 4，则直接求平均。
     */
    uint16_t select_sort(uint16_t *a, uint16_t len);

    /**
     * @brief  中间 80% 均值
     * @param  in   输入数组（会被排序）
     * @param  len  数组长度
     * @return 中间 80% 数据的均值
     *
     * 丢弃两端各 10% 的极值点。
     */
    uint16_t waveMidFilter(uint16_t *in, uint16_t len);

    /**
     * @brief  斩波均值滤波
     * @param  in     输入数组（会被修改）
     * @param  len    数组长度
     * @param  limit  初始阈值
     * @param  N      迭代轮数
     * @return 滤波后的均值
     *
     * 每轮将阈值抬升到当前均值，只保留高于阈值的数据。
     * 迭代 N 轮后，对剩余数据做截尾均值。
     * 若某轮剩余数据 < 10，返回 0。
     */
    uint16_t waveLimitFilter(uint16_t *in, uint16_t len, uint16_t limit, uint16_t N);

    /**
     * @brief  增强滤波（带保持和抗跳变）
     * @param  in     输入数组
     * @param  len    数组长度
     * @param  limit  初始阈值
     * @param  N      迭代轮数
     * @param  st     通道状态（跨帧保持）
     * @return 滤波后的稳定值
     *
     * 在 waveLimitFilter 基础上增加：
     * - 短空档保持：连续零值达到阈值才归零
     * - 跳变确认：大跳变需要连续出现才接受
     */
    uint16_t waveLimitFilter_x(uint16_t *in, uint16_t len, uint16_t limit, uint16_t N, WaveHoldState *st);

private:
    ScopeConfig_t m_config;   ///< 当前配置
    ScopeResult_t m_result;   ///< 最近一次结果

    WaveHoldState m_st400_450_ch1;  ///< 400/450 CH1 状态
    WaveHoldState m_st400_450_ch2;  ///< 400/450 CH2 状态（预留）

    WaveHoldState m_stGSM_ch5;      ///< GSM CH5 状态
    WaveHoldState m_stGSM_ch6;      ///< GSM CH6 状态（预留）
};

#endif // XY7320_OSCILLOSCOPE_H
