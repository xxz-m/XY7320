/**
 * @file    gnss_service.cpp
 * @brief   GNSS 接收服务实现
 */

#include "gnss_service.h"
#include "app_config.h"
#include "bsp_uart_rcv.h"
#include "usart.h"

namespace {
constexpr uint16_t GNSS_DMA_BUF_SIZE = 512U;
constexpr uint16_t GNSS_PROC_BUF_SIZE = 512U;
constexpr uint16_t GNSS_LOG_DIVIDER = 100U;

/* GNSS 输出为连续 NMEA 文本流，DMA chunk 可能包含半行、多行或一行半。
 * 因此 BSP 只保留原始 chunk，Service 层再按 '$' 与换行重新拼行。 */
uint8_t s_gnssDmaBuf[GNSS_DMA_BUF_SIZE];
uint8_t s_gnssProcBuf[GNSS_PROC_BUF_SIZE];

const char *TalkerName(ProtocolGnss::Talker talker)
{
    switch (talker) {
    case ProtocolGnss::Talker::GPS:
        return "GPS";
    case ProtocolGnss::Talker::BeiDou:
        return "BDS";
    case ProtocolGnss::Talker::GLONASS:
        return "GLO";
    case ProtocolGnss::Talker::Galileo:
        return "GAL";
    case ProtocolGnss::Talker::Mixed:
        return "MIX";
    default:
        return "UNK";
    }
}
} // namespace

GnssService& GnssService::Instance()
{
    static GnssService instance;
    return instance;
}

void GnssService::Init()
{
    // 把 USART3 绑定到 BSP 的 GNSS 实例；DMA/处理缓冲区各 512B，
    // 用于容纳 NMEA 单行、多行输出以及 IDLE 分段的余量。
    // Init 只配置上下文，Start 才真正打开 DMA 和 IDLE 中断。
    BspUartRcv_t *gnssRcv = BspUartRcv_GetGnss();
    BspUartRcv_Init(gnssRcv,
                    &huart3,
                    s_gnssDmaBuf,
                    GNSS_DMA_BUF_SIZE,
                    s_gnssProcBuf,
                    GNSS_PROC_BUF_SIZE);
    //BspUartRcv_Start(gnssRcv);

    // 重置内部状态：避免上一次开机残留的定位/计数器被本次开机误读为有效数据
    m_initialized = true;
    m_lineLen = 0;
    m_lineCount = 0;
    m_validGgaCount = 0;
    m_validRmcCount = 0;
    m_logDivider = 0;
    m_fix = {};

    LOG_Printf("GnssService,Init,USART3,DMA+IDLE,OK\r\n");
}
void GnssService::Start()
{
    if (!m_initialized) {
        return;
    }
    BspUartRcv_Start(BspUartRcv_GetGnss());
    // 重置内部状态：避免上一次开机残留的定位/计数器被本次开机误读为有效数据
    m_initialized = true;
    m_lineLen = 0;
    m_lineCount = 0;
    m_validGgaCount = 0;
    m_validRmcCount = 0;
    m_logDivider = 0;
    m_fix = {};
    LOG_Printf("GnssService,Start,OK\r\n");
}
void GnssService::Stop()
{
    if (!m_initialized) {
        return;
    }
    BspUartRcv_Stop(BspUartRcv_GetGnss());
    m_lineLen = 0U;
    LOG_Printf("GnssService,Stop,OK\r\n");
}
/**
 * @brief 周期轮询入口
 *
 * @warning 必须在主循环或任务上下文调用：内部调用 BspUartRcv_* 与日志接口，
 *          都不允许在中断上下文执行。每次调用最多处理一个 chunk，
 *          调用方需自行保证轮询周期能跟上 NMEA 输出节奏。
 */
void GnssService::Update()
{
    if (!m_initialized) {
        return;
    }

    BspUartRcv_t *gnssRcv = BspUartRcv_GetGnss();
    // TakeOverflow 同时返回并清零标志，必须每个周期都查询，
    // 否则溢出事件会被新的覆盖，丢失告警
    if (BspUartRcv_TakeOverflow(gnssRcv)) {
        LOG_Printf("GnssService,Rcv,Overflow\r\n");
    }

    if (!BspUartRcv_IsFrameReady(gnssRcv)) {
        return;
    }

    const uint16_t len = BspUartRcv_GetFrameLength(gnssRcv);
    if (len == 0U || len > RX_CHUNK_SIZE) {
        // 异常长度：清掉 ready 标志并跳过，避免把异常数据喂给协议层
        BspUartRcv_ClearFlag(gnssRcv);
        return;
    }

    // 顺序必须是 CopyFrame -> ClearFlag：
    // 先把 BSP procBuf 数据搬到本地 m_rxChunk，再让 BSP 继续接收下一帧，
    // 否则 ClearFlag 后再来新数据会覆盖 procBuf，本帧就丢了
    BspUartRcv_CopyFrame(gnssRcv, m_rxChunk);
    BspUartRcv_ClearFlag(gnssRcv);

    AppendInput(m_rxChunk, len);
}

/**
 * @brief 追加 UART DMA+IDLE 收到的原始 chunk，并按 NMEA 行边界拆分
 *
 * IDLE 中断只代表串口出现空闲间隔，不保证 chunk 刚好是一条 NMEA 语句。
 * 这里遇到 '$' 重新同步起点，遇到 '\n' 再把整行交给协议层解析。
 */
void GnssService::AppendInput(const uint8_t *data, uint16_t len)
{
    if (data == nullptr || len == 0U) {
        return;
    }

    for (uint16_t i = 0; i < len; ++i) {
        const char ch = static_cast<char>(data[i]);

        if (ch == '$') {
            // 把 '$' 当成重同步锚点：
            // 1) 上一次是中途被截断的半行，新 '$' 来了直接丢弃旧缓冲；
            // 2) chunk 起头就是 '$'，相当于新一轮行起点。
            m_lineLen = 0;
        }

        if (m_lineLen < LINE_BUF_SIZE) {
            m_lineBuf[m_lineLen++] = ch;
        } else {
            // 行缓冲已满（正常 NMEA 不会发生，多半是链路异常/乱码）。
            // 静默丢弃当前行，并清零等待下一个 '$' 重新同步；
            // continue 跳过本字符的 '\n' 解析，避免在污染缓冲上触发解析。
            m_lineLen = 0;
            continue;
        }

        if (ch == '\n') {
            // '\n' 是 NMEA 行结束标志；同步喂给协议层解析，
            // 处理完成后清空行缓冲等待下一行。
            HandleLine(m_lineBuf, m_lineLen);
            m_lineLen = 0;
        }
    }
}

/**
 * @brief 解析一整条 NMEA 行，并把 GGA/RMC 结果合并到最新定位状态
 */
void GnssService::HandleLine(const char *line, uint16_t len)
{
    ProtocolGnss::DecodeResult result{};
    if (!ProtocolGnss::DecodeLine(line, len, &result)) {
        // 解码失败（不支持的句型、校验不过、字段缺失等）直接丢弃，
        // 不计入 m_lineCount，避免日志统计被乱码拉高
        return;
    }

    ++m_lineCount;

    // 用 gga.valid / rmc.valid 判定而不是 parsed：
    // parsed=true 仅代表字段级解析完成，valid 才代表业务有效数据
    if (result.type == ProtocolGnss::SentenceType::GGA && result.gga.valid) {
        ApplyGga(result.gga);
        ++m_validGgaCount;
    } else if (result.type == ProtocolGnss::SentenceType::RMC && result.rmc.valid) {
        ApplyRmc(result.rmc);
        ++m_validRmcCount;
    }

    RefreshFixState();
    PrintFixBrief();
}

void GnssService::ApplyGga(const ProtocolGnss::GgaInfo& gga)
{
    // GGA 不带日期，所以只复制时间字段；日期由 RMC 覆盖。
    // 一旦写过 hasGga 就不再清回 false，避免后续短暂无 GGA 时把状态打回冷启动。
    m_fix.talker = gga.talker;
    m_fix.utc.hour = gga.utc.hour;
    m_fix.utc.minute = gga.utc.minute;
    m_fix.utc.second = gga.utc.second;
    m_fix.utc.millisecond = gga.utc.millisecond;
    m_fix.utc.timeValid = gga.utc.timeValid;
    m_fix.fixQuality = gga.fixQuality;
    m_fix.latitudeE7 = gga.coordinate.latitudeE7;
    m_fix.longitudeE7 = gga.coordinate.longitudeE7;
    m_fix.satelliteCount = gga.satelliteCount;
    m_fix.hdopX100 = gga.hdopX100;
    m_fix.altitudeCm = gga.altitudeCm;
    m_fix.hasGga = true;
}

void GnssService::ApplyRmc(const ProtocolGnss::RmcInfo& rmc)
{
    // RMC 携带完整时间+日期，所以直接整体覆盖 utc；
    // 经纬度/速度是 RMC 主字段，坐标和 GGA 取最新一条即可。
    m_fix.talker = rmc.talker;
    m_fix.utc = rmc.utc;
    m_fix.latitudeE7 = rmc.coordinate.latitudeE7;
    m_fix.longitudeE7 = rmc.coordinate.longitudeE7;
    m_fix.speedKnotsX1000 = rmc.speedKnotsX1000;
    m_fix.speedKmhX1000 = rmc.speedKmhX1000;
    m_fix.rmcActive = rmc.status == ProtocolGnss::RmcStatus::Active;
    m_fix.hasRmc = true;
}

void GnssService::RefreshFixState()
{
    // hasFix 的判定要兼顾 GGA 和 RMC 双方：
    // - GGA 提供定位质量/卫星数，但 RMC 才是 NMEA 标准里的“有效定位”位
    // - 任一方尚未到达或 RMC 未激活都视为不可用
    // - fixQuality 不能是 Invalid/Unknown，避免把"未定位"误判为有效
    m_fix.hasFix = m_fix.hasGga &&
                   m_fix.hasRmc &&
                   m_fix.rmcActive &&
                   m_fix.fixQuality != ProtocolGnss::GgaFixQuality::Invalid &&
                   m_fix.fixQuality != ProtocolGnss::GgaFixQuality::Unknown;
}

void GnssService::PrintFixBrief()
{
    // 用分频器避免每条 NMEA 都打印：1Hz 输出大致对应 GNSS_LOG_DIVIDER=100，
    // 实际节流比例随调度周期而变，符合“关键字段 1s 一次概要”的定位
    if (++m_logDivider < GNSS_LOG_DIVIDER) {
        return;
    }
    m_logDivider = 0;

    LOG_Printf("GNSS,%s,fix,%u,gga,%lu,rmc,%lu,latE7,%ld,lonE7,%ld,sat,%u,hdopX100,%u,altCm,%ld,kmhX1000,%u\r\n",
               TalkerName(m_fix.talker),
               m_fix.hasFix ? 1U : 0U,
               static_cast<unsigned long>(m_validGgaCount),
               static_cast<unsigned long>(m_validRmcCount),
               static_cast<long>(m_fix.latitudeE7),
               static_cast<long>(m_fix.longitudeE7),
               m_fix.satelliteCount,
               m_fix.hdopX100,
               static_cast<long>(m_fix.altitudeCm),
               m_fix.speedKmhX1000);
}
