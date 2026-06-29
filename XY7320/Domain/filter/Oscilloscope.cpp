/**
 * @file    Oscilloscope.cpp
 * @brief   多通道信号滤波引擎实现
 */

#include "Oscilloscope.h"

Oscilloscope& Oscilloscope::getInstance()
{
    static Oscilloscope instance;
    return instance;
}

void Oscilloscope::initOscilloscope()
{
    /* 默认 400/450 模式，避免上电后模式未定义 */
    m_config.SampleNum = 512;
    m_config.TriggerLevel = 100;
    m_config.WavePEPNum = 5;
    m_config.Mode = SCOPE_MODE_400_450;

    /* 400/450 阈值：CH1/CH3 和 CH2/CH4 前端放大倍数不同 */
    m_config.cfg400_450.Threshold1 = 300;
    m_config.cfg400_450.Threshold2 = 600;
    m_config.cfg400_450.Threshold3 = 300;
    m_config.cfg400_450.Threshold4 = 600;

    /* GSM 阈值：GSM 通道噪声边界更高 */
    m_config.cfgGSM.Threshold5 = 600;
    m_config.cfgGSM.Threshold6 = 1500;

    /* 清空结果和状态，避免残留影响首帧 */
    m_result = {0};
    m_st400_450_ch1 = {0};
    m_st400_450_ch2 = {0};
    m_stGSM_ch5 = {0};
    m_stGSM_ch6 = {0};
}

void Oscilloscope::setMode(ScopeMode_t mode)
{
    if (m_config.Mode == mode)
    {
        return;
    }

    m_config.Mode = mode;

    /* 切换模式时清空状态，防止跨模式干扰 */
    m_st400_450_ch1 = {0};
    m_st400_450_ch2 = {0};
    m_stGSM_ch5 = {0};
    m_stGSM_ch6 = {0};
    m_result = {0};
}

ScopeConfig_t Oscilloscope::getConfig() const
{
    return m_config;
}

ScopeResult_t Oscilloscope::getResult() const
{
    return m_result;
}

void Oscilloscope::preprocessChannel(uint16_t *adcData, uint16_t threshold)
{
    if (adcData == nullptr)
    {
        return;
    }

    for (uint16_t i = 0; i < m_config.SampleNum; ++i)
    {
        if (adcData[i] < threshold)
        {
            adcData[i] = 0;
        }
    }
}

uint16_t Oscilloscope::select_sort(uint16_t *a, uint16_t len)
{
    if (a == nullptr || len == 0)
    {
        return 0;
    }

    /* 选择排序：从大到小 */
    for (uint16_t i = 0; i < len; ++i)
    {
        uint16_t x = a[i];
        uint16_t l = i;
        for (uint16_t j = i; j < len; ++j)
        {
            if (a[j] > x)
            {
                x = a[j];
                l = j;
            }
        }
        a[l] = a[i];
        a[i] = x;
    }

    /* 长度 <= 4 时直接求平均 */
    if (len <= 4)
    {
        uint32_t sum = 0;
        for (uint16_t i = 0; i < len; ++i)
        {
            sum += a[i];
        }
        return (uint16_t)(sum / len);
    }

    /* 去掉前 2 和后 2 个值，对中间部分求平均 */
    uint32_t sum = 0;
    uint16_t outlen = 0;
    for (uint16_t i = 2; i < len - 2; ++i)
    {
        sum += a[i];
        ++outlen;
    }

    return outlen ? (uint16_t)(sum / outlen) : 0;
}

uint16_t Oscilloscope::waveMidFilter(uint16_t *in, uint16_t len)
{
    if (in == nullptr || len == 0)
    {
        return 0;
    }

    /* 冒泡排序：从大到小 */
    for (uint16_t i = 0; i < len - 1; ++i)
    {
        for (uint16_t j = i + 1; j < len; ++j)
        {
            if (in[j] > in[i])
            {
                uint16_t temp = in[i];
                in[i] = in[j];
                in[j] = temp;
            }
        }
    }

    /* 取中间 80% 数据求平均 */
    uint32_t sum = 0;
    uint16_t outlen = 0;
    for (uint16_t i = len * 10 / 100; i < len * 90 / 100; ++i)
    {
        sum += in[i];
        ++outlen;
    }

    return outlen ? (uint16_t)(sum / outlen) : 0;
}

uint16_t Oscilloscope::waveLimitFilter(uint16_t *in, uint16_t len, uint16_t limit, uint16_t N)
{
    if (in == nullptr || len == 0)
    {
        return 0;
    }

    uint16_t templen = len;
    uint16_t avg = limit;
    uint16_t midIn[ADC_SIZE_MAX] = {0};

    /* 迭代 N 轮，每轮将阈值抬升到当前均值 */
    for (uint16_t j = 0; j < N; ++j)
    {
        uint16_t templen2 = 0;
        for (uint16_t i = 0; i < templen; ++i)
        {
            if (in[i] >= avg)
            {
                midIn[templen2++] = in[i];
            }
        }

        templen = templen2;

        /* 剩余数据太少，认为无有效信号 */
        if (templen < 10)
        {
            return 0;
        }

        uint32_t sum = 0;
        for (uint16_t k = 0; k < templen; ++k)
        {
            in[k] = midIn[k];
            sum += midIn[k];
        }

        avg = (uint16_t)(sum / templen);
    }

    return select_sort(in, templen);
}

uint16_t Oscilloscope::waveLimitFilter_x(uint16_t *in,
                                         uint16_t len,
                                         uint16_t limit,
                                         uint16_t N,
                                         WaveHoldState *st)
{
    if (in == nullptr || st == nullptr || len == 0)
    {
        return 0;
    }

    /* 抗闪烁参数 */
    const uint8_t ZERO_CONFIRM = 10;    /* 连续 10 帧零值才归零 */
    const uint16_t TH_ON = 500;         /* 无信号时的有效门限 */
    const uint16_t TH_KEEP = 450;       /* 有信号时的保持门限（更低） */
    const uint16_t JUMP_DELTA = 500;    /* 跳变阈值 */
    const uint8_t JUMP_CONFIRM = 2;     /* 跳变确认次数 */

    uint16_t templen = len;
    uint16_t avg = limit;
    uint16_t midIn[ADC_SIZE_MAX] = {0};
    uint16_t raw = 0;

    /* 斩波均值滤波 */
    for (uint16_t j = 0; j < N; ++j)
    {
        uint16_t templen2 = 0;
        for (uint16_t i = 0; i < templen; ++i)
        {
            if (in[i] >= avg)
            {
                midIn[templen2++] = in[i];
            }
        }

        templen = templen2;

        if (templen < 10)
        {
            raw = 0;
            goto POST_PROCESS;
        }

        uint32_t sum = 0;
        for (uint16_t k = 0; k < templen; ++k)
        {
            in[k] = midIn[k];
            sum += midIn[k];
        }

        avg = (uint16_t)(sum / templen);
    }

    raw = select_sort(in, templen);

POST_PROCESS:
    {
        /* 有信号时用更低的保持门限，避免信号短暂波动导致误判 */
        uint16_t th = st->sig ? TH_KEEP : TH_ON;
        bool valid = (raw >= th);

        if (valid)
        {
            st->zeroCnt = 0;

            /* 检查是否有大跳变 */
            if (st->sig && st->lastNonZero > 0)
            {
                uint16_t diff = (raw > st->lastNonZero) ? (raw - st->lastNonZero)
                                                        : (st->lastNonZero - raw);

                if (diff > JUMP_DELTA)
                {
                    /* 第一次看到大跳变：先暂存候选，不立即接受 */
                    if (st->candCnt == 0)
                    {
                        st->cand = raw;
                        st->candCnt = 1;
                        return st->lastNonZero;
                    }
                    else
                    {
                        /* 检查候选值是否与当前值相近 */
                        uint16_t cdiff = (raw > st->cand) ? (raw - st->cand)
                                                          : (st->cand - raw);

                        if (cdiff <= JUMP_DELTA)
                        {
                            /* 连续出现相近跳变，计数加 1 */
                            if (st->candCnt < 255)
                            {
                                ++st->candCnt;
                            }

                            /* 未达到确认次数，继续返回旧值 */
                            if (st->candCnt < JUMP_CONFIRM)
                            {
                                return st->lastNonZero;
                            }
                        }
                        else
                        {
                            /* 跳变方向改变，重新暂存 */
                            st->cand = raw;
                            st->candCnt = 1;
                            return st->lastNonZero;
                        }
                    }
                }
            }

            /* 无跳变或跳变已确认，接受新值 */
            st->sig = 1;
            st->lastNonZero = raw;
            st->candCnt = 0;
            st->cand = 0;

            return raw;
        }
        else
        {
            /* 无效值：清空候选，开始计数零值 */
            st->candCnt = 0;
            st->cand = 0;

            if (st->zeroCnt < 255)
            {
                ++st->zeroCnt;
            }

            /* 连续零值达到阈值，认为信号消失 */
            if (st->zeroCnt >= ZERO_CONFIRM)
            {
                st->sig = 0;
                st->lastNonZero = 0;
                return 0;
            }

            /* 短空档内保持旧值 */
            return st->sig ? st->lastNonZero : 0;
        }
    }
}

void Oscilloscope::TickLoop400_450(uint16_t *adcCH1,
                                   uint16_t *adcCH2,
                                   uint16_t *adcCH3,
                                   uint16_t *adcCH4)
{
    if (adcCH1 == nullptr || adcCH2 == nullptr || adcCH3 == nullptr || adcCH4 == nullptr)
    {
        return;
    }

    if (m_config.Mode != SCOPE_MODE_400_450)
    {
        return;
    }

    /* 底噪清零 */
    preprocessChannel(adcCH1, m_config.cfg400_450.Threshold1);
    preprocessChannel(adcCH2, m_config.cfg400_450.Threshold2);
    preprocessChannel(adcCH3, m_config.cfg400_450.Threshold3);
    preprocessChannel(adcCH4, m_config.cfg400_450.Threshold4);

    /* CH1 用增强滤波，CH2~4 用普通滤波 */
    m_result.wavePEP1_avg = waveLimitFilter_x(adcCH1, m_config.SampleNum, m_config.TriggerLevel, 4, &m_st400_450_ch1);
    m_result.wavePEP2_avg = waveLimitFilter(adcCH2, m_config.SampleNum, m_config.TriggerLevel, 4);
    m_result.wavePEP3_avg = waveLimitFilter(adcCH3, m_config.SampleNum, m_config.TriggerLevel, 4);
    m_result.wavePEP4_avg = waveLimitFilter(adcCH4, m_config.SampleNum, m_config.TriggerLevel, 4);
}

void Oscilloscope::TickLoopGSM(uint16_t *adcCH5,
                               uint16_t *adcCH6)
{
    if (adcCH5 == nullptr || adcCH6 == nullptr)
    {
        return;
    }

    if (m_config.Mode != SCOPE_MODE_GSM)
    {
        return;
    }

    /* 底噪清零 */
    preprocessChannel(adcCH5, m_config.cfgGSM.Threshold5);
    preprocessChannel(adcCH6, m_config.cfgGSM.Threshold6);

    /* CH5 用增强滤波，CH6 用普通滤波 */
    m_result.wavePEP5_avg = waveLimitFilter_x(adcCH5, m_config.SampleNum, m_config.TriggerLevel, 4, &m_stGSM_ch5);
    m_result.wavePEP6_avg = waveLimitFilter(adcCH6, m_config.SampleNum, m_config.TriggerLevel, 4);
}
