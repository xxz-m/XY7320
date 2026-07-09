/**
 * @file    protocol_gnss.h
 * @brief   GNSS NMEA 文本协议解析接口
 *
 * 本模块只解析一整条 NMEA 语句，不依赖 BSP、HAL 或 Service。
 * 调用方负责从串口流中按 '$' 与换行拆出单行后传入 DecodeLine()。
 */

#ifndef XY7320_PROTOCOL_GNSS_H
#define XY7320_PROTOCOL_GNSS_H

#include <stdint.h>

/**
 * @brief GNSS NMEA 协议解析器
 *
 * 解析范围：
 * - GGA：UTC、经纬度、定位质量、卫星数、HDOP、海拔
 * - RMC：UTC、有效状态、经纬度、速度、日期
 */
class ProtocolGnss {
public:
    /** NMEA talker 标识，反映语句来源或融合类型。 */
    enum class Talker : uint8_t {
        Unknown = 0,
        GPS,
        BeiDou,
        GLONASS,
        Galileo,
        Mixed,
    };

    /** 当前支持解析的 NMEA 语句类型。 */
    enum class SentenceType : uint8_t {
        Unknown = 0,
        GGA,
        RMC,
    };

    /** GGA 定位质量字段。 */
    enum class GgaFixQuality : uint8_t {
        Invalid = 0,
        GpsFix = 1,
        DgpsFix = 2,
        PpsFix = 3,
        RtkFixed = 4,
        RtkFloat = 5,
        Estimated = 6,
        Manual = 7,
        Simulation = 8,
        Unknown = 255,
    };

    /** RMC 有效状态。 */
    enum class RmcStatus : uint8_t {
        Invalid = 0,
        Active,
    };

    /** UTC 日期时间。GGA 只有时间，RMC 同时带时间和日期。 */
    struct DateTime {
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
        uint16_t millisecond = 0;

        uint8_t day = 0;
        uint8_t month = 0;
        uint16_t year = 0;

        bool timeValid = false;
        bool dateValid = false;
    };

    /** 经纬度，单位为度 * 1e7，例如 30.6530273 度存为 306530273。 */
    struct Coordinate {
        int32_t latitudeE7 = 0;
        int32_t longitudeE7 = 0;
        bool latitudeValid = false;
        bool longitudeValid = false;

        bool IsValid() const
        {
            return latitudeValid && longitudeValid;
        }
    };

    /** GGA 语句解析结果。 */
    struct GgaInfo {
        Talker talker = Talker::Unknown;
        DateTime utc;
        Coordinate coordinate;
        GgaFixQuality fixQuality = GgaFixQuality::Invalid;
        uint8_t satelliteCount = 0;
        uint16_t hdopX100 = 0;
        int32_t altitudeCm = 0;
        bool valid = false;
    };

    /** RMC 语句解析结果。 */
    struct RmcInfo {
        Talker talker = Talker::Unknown;
        DateTime utc;
        Coordinate coordinate;
        RmcStatus status = RmcStatus::Invalid;
        uint16_t speedKnotsX1000 = 0;
        uint16_t speedKmhX1000 = 0;
        bool valid = false;
    };

    /**
     * @brief 单条 NMEA 语句解析结果。
     *
     * 字段语义：
     * - @c checksumOk：'$' 与 '*' 之间的字节 XOR 是否等于 '*' 后的两位十六进制。
     * - @c parsed    ：语句被识别并完成字段级解析，不代表数据本身有效。
     * - @c gga.valid / @c rmc.valid：在 parsed 基础上还要求关键字段非空、
     *                              坐标/速度/时间格式合法等业务有效性检查通过。
     * 调用方应优先用 @c gga.valid / @c rmc.valid 判定数据是否可用，
     * 不要只看 @c parsed。
     */
    struct DecodeResult {
        SentenceType type = SentenceType::Unknown;
        Talker talker = Talker::Unknown;
        bool checksumOk = false;
        bool parsed = false;
        GgaInfo gga;
        RmcInfo rmc;
    };

    /**
     * @brief 解析一整条 NMEA 语句
     * @param line NMEA 行，示例："$GNGGA,...*66"
     * @param len  line 长度，可包含末尾 \r\n
     * @param out  解析结果输出
     * @return true 语句已识别并完成解析；false 无效、校验失败或不支持
     */
    static bool DecodeLine(const char *line, uint16_t len, DecodeResult *out);

private:
    static bool VerifyChecksum(const char *line, uint16_t len);
    static Talker ParseTalker(const char *line, uint16_t len);
    static SentenceType ParseSentenceType(const char *line, uint16_t len);

    static bool ParseGga(const char *line, uint16_t len, DecodeResult *out);
    static bool ParseRmc(const char *line, uint16_t len, DecodeResult *out);

    static bool GetField(const char *line,
                         uint16_t len,
                         uint8_t fieldIndex,
                         const char **field,
                         uint16_t *fieldLen);

    static bool ParseUtcTime(const char *field, uint16_t fieldLen, DateTime *out);
    static bool ParseDate(const char *field, uint16_t fieldLen, DateTime *out);
    static bool ParseCoordinate(const char *value,
                                uint16_t valueLen,
                                const char *hemisphere,
                                uint16_t hemisphereLen,
                                bool isLatitude,
                                Coordinate *out);

    static uint32_t ParseUnsigned(const char *field, uint16_t fieldLen);
    static uint32_t ParseDecimalScaled(const char *field, uint16_t fieldLen, uint32_t scale);
    static int32_t ParseSignedDecimalScaled(const char *field, uint16_t fieldLen, uint32_t scale);
    static uint8_t HexToNibble(char c);
    static uint16_t TrimLineLength(const char *line, uint16_t len);
};

#endif // XY7320_PROTOCOL_GNSS_H
