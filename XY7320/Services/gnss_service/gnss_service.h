/**
 * @file    gnss_service.h
 * @brief   GNSS 接收服务
 *
 *          负责从 BSP UART DMA+IDLE 接收层取出原始 chunk，拼接 NMEA 行，
 *          调用 ProtocolGnss 解析 GGA/RMC，并维护最新定位状态。
 */

#ifndef XY7320_GNSS_SERVICE_H
#define XY7320_GNSS_SERVICE_H

#include <stdint.h>
#include "protocol_gnss.h"

/**
 * @brief GNSS 服务层最新定位状态
 */
struct GnssFix {
    ProtocolGnss::Talker talker = ProtocolGnss::Talker::Unknown;                 /**< 最近一次有效语句来源 */
    ProtocolGnss::DateTime utc;                                                  /**< UTC 日期时间 */
    ProtocolGnss::GgaFixQuality fixQuality = ProtocolGnss::GgaFixQuality::Invalid; /**< GGA 定位质量 */

    int32_t latitudeE7 = 0;       /**< 纬度，单位：度 * 1e7 */
    int32_t longitudeE7 = 0;      /**< 经度，单位：度 * 1e7 */
    uint8_t satelliteCount = 0;   /**< 卫星数 */
    uint16_t hdopX100 = 0;        /**< HDOP * 100 */
    int32_t altitudeCm = 0;       /**< 海拔，单位：cm */
    uint16_t speedKnotsX1000 = 0; /**< 地速，单位：节 * 1000 */
    uint16_t speedKmhX1000 = 0;   /**< 地速，单位：km/h * 1000 */

    bool rmcActive = false;       /**< RMC 状态是否为 A */
    bool hasGga = false;          /**< 是否收到过有效 GGA */
    bool hasRmc = false;          /**< 是否收到过有效 RMC */
    bool hasFix = false;          /**< 是否具备可用定位状态 */
};

/**
 * @brief GNSS 接收服务
 */
class GnssService {
public:
    /** 获取单例。 */
    static GnssService& Instance();

    /**
     * @brief 初始化 USART3 GNSS 接收上下文。
     *
     * 只绑定 UART、DMA 缓冲区和处理缓冲区并清零服务状态，
     * 不启动 DMA；进入 GNSS 状态后由 Start() 启动接收。
     */
    void Init();

    /**
     * @brief 周期轮询入口：取 chunk、拼行、解析并更新状态。
     *
     * @warning 必须在主循环或任务上下文调用，禁止在中断里调用。
     *          必须在 Init() 之后调用；未初始化时直接返回。
     *          当前由 TaskstateGnss::tick() 周期调用。
     */
    void Update();

    /**
     * @brief 启动 USART3 的 DMA+IDLE 接收。
     *
     * Init() 完成后才能调用；重复启动前应先调用 Stop()。
     * 进入 GNSS FSM 状态时由 TaskstateGnss::entry() 调用。
     */
    void Start();

    /**
     * @brief 停止 USART3 的 DMA+IDLE 接收。
     *
     * 保留已绑定的 UART 和缓冲区配置，后续可再次调用 Start()。
     * 离开 GNSS FSM 状态时由 TaskstateGnss::exit() 调用。
     */
    void Stop();

    /** @brief 获取最新定位状态。 */
    const GnssFix& GetFix() const { return m_fix; }

    /** 是否已经初始化。 */
    bool IsInitialized() const { return m_initialized; }

private:
    GnssService() = default;

    /**
     * @brief 追加 UART DMA+IDLE 收到的原始 chunk，并按 NMEA 行边界拆分。
     *
     * 当缓冲区装不下当前行（>= LINE_BUF_SIZE）时，会静默丢弃当前行并
     * 重新从下一个 '$' 开始同步。理论上 NMEA 单行不会超长，这里主要
     * 是异常乱码场景下的保护。
     */
    void AppendInput(const uint8_t *data, uint16_t len);

    void HandleLine(const char *line, uint16_t len);
    void ApplyGga(const ProtocolGnss::GgaInfo& gga);
    void ApplyRmc(const ProtocolGnss::RmcInfo& rmc);
    void RefreshFixState();
    void PrintFixBrief();

    static constexpr uint16_t RX_CHUNK_SIZE = 512U;
    static constexpr uint16_t LINE_BUF_SIZE = 192U;

    uint8_t m_rxChunk[RX_CHUNK_SIZE]{};
    char m_lineBuf[LINE_BUF_SIZE]{};
    uint16_t m_lineLen = 0;
    uint32_t m_lineCount = 0;
    uint32_t m_validGgaCount = 0;
    uint32_t m_validRmcCount = 0;
    uint16_t m_logDivider = 0;
    bool m_initialized = false;
    GnssFix m_fix{};
};

#endif /* XY7320_GNSS_SERVICE_H */
