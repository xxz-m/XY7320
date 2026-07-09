/**
 * @file    protocol_gnss.cpp
 * @brief   GNSS NMEA 文本协议解析实现
 */

#include "protocol_gnss.h"

namespace {
constexpr uint32_t KNOT_TO_KMH_X1000_NUM = 1852U;
constexpr uint32_t KNOT_TO_KMH_X1000_DEN = 1000U;

bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}
} // namespace

/**
 * @brief 解析一整条 NMEA 语句
 *
 * @note 校验流程：先裁掉末尾 \r\n，然后检查首字符是否为 '$'，再算 XOR 校验。
 *       校验失败直接返回 false；校验通过后判断 talker/语句类型，分发到 GGA/RMC 解析。
 *       返回值与 out->parsed 等价：true 表示语句被识别并完成字段级解析，
 *       但仍需结合 gga.valid / rmc.valid 判定数据业务有效性。
 */
bool ProtocolGnss::DecodeLine(const char *line, uint16_t len, DecodeResult *out)
{
    if (line == nullptr || out == nullptr) {
        return false;
    }

    // 每次解析前先把结果清零，避免上一次调用残留污染当前结果
    *out = DecodeResult{};
    len = TrimLineLength(line, len);

    if (len < 7U || line[0] != '$') {
        // 至少要 "$??TXT*XX"（7 字节）才是一条最小可识别 NMEA 语句
        return false;
    }

    out->checksumOk = VerifyChecksum(line, len);
    if (!out->checksumOk) {
        // 校验失败的语句直接丢弃，避免把乱码当作合法定位数据使用
        return false;
    }

    out->talker = ParseTalker(line, len);
    out->type = ParseSentenceType(line, len);

    // 只分发到当前支持的 GGA/RMC；其他类型标记 parsed=false，
    // 但校验位已经过，调用方仍可凭 checksumOk 判断收到了合法 NMEA
    switch (out->type) {
    case SentenceType::GGA:
        out->parsed = ParseGga(line, len, out);
        break;
    case SentenceType::RMC:
        out->parsed = ParseRmc(line, len, out);
        break;
    default:
        out->parsed = false;
        break;
    }

    return out->parsed;
}

/**
 * @brief 校验 NMEA 的 *XX 校验和
 *
 * NMEA checksum 规则：对 '$' 之后、'*' 之前的所有 ASCII 字符逐字节 XOR，
 * 结果必须等于 '*' 后两个十六进制字符表示的数值。
 */
bool ProtocolGnss::VerifyChecksum(const char *line, uint16_t len)
{
    if (line == nullptr || len < 4U || line[0] != '$') {
        // 长度至少 "$X*Y" 才可能有校验位，提前返回避免越界访问
        return false;
    }

    uint16_t starIndex = 0;
    bool foundStar = false;
    for (uint16_t i = 1; i < len; ++i) {
        // 只在 '$' 之后寻找 '*'，不进入业务字段，避免误判
        if (line[i] == '*') {
            starIndex = i;
            foundStar = true;
            break;
        }
    }

    if (!foundStar || (starIndex + 2U) >= len) {
        // '*' 之后必须还有两位十六进制字符，否则校验位不完整
        return false;
    }

    uint8_t calc = 0;
    for (uint16_t i = 1; i < starIndex; ++i) {
        // 起始 i=1 跳过 '$'，到 starIndex 为止不包含 '*' 本身，
        // 正好与 NMEA 规范定义的范围一致
        calc ^= static_cast<uint8_t>(line[i]);
    }

    const uint8_t high = HexToNibble(line[starIndex + 1U]);
    const uint8_t low = HexToNibble(line[starIndex + 2U]);
    if (high > 0x0FU || low > 0x0FU) {
        // HexToNibble 失败时返回 0xFF，越界说明这一段不是合法十六进制字符
        return false;
    }

    const uint8_t expected = static_cast<uint8_t>((high << 4U) | low);
    return calc == expected;
}

/**
 * @brief 从 NMEA talker 两字符识别卫星系统来源
 *
 * GP=GPS，GB/BD=北斗，GL=GLONASS，GA=Galileo，GN=多星座融合。
 */
ProtocolGnss::Talker ProtocolGnss::ParseTalker(const char *line, uint16_t len)
{
    if (line == nullptr || len < 6U || line[0] != '$') {
        return Talker::Unknown;
    }

    const char a = line[1];
    const char b = line[2];

    if (a == 'G' && b == 'P') {
        return Talker::GPS;
    }
    if ((a == 'G' && b == 'B') || (a == 'B' && b == 'D')) {
        return Talker::BeiDou;
    }
    if (a == 'G' && b == 'L') {
        return Talker::GLONASS;
    }
    if (a == 'G' && b == 'A') {
        return Talker::Galileo;
    }
    if (a == 'G' && b == 'N') {
        return Talker::Mixed;
    }

    return Talker::Unknown;
}

/**
 * @brief 从 NMEA 语句头识别当前支持的语句类型
 */
ProtocolGnss::SentenceType ProtocolGnss::ParseSentenceType(const char *line, uint16_t len)
{
    if (line == nullptr || len < 6U || line[0] != '$') {
        return SentenceType::Unknown;
    }

    if (line[3] == 'G' && line[4] == 'G' && line[5] == 'A') {
        return SentenceType::GGA;
    }
    if (line[3] == 'R' && line[4] == 'M' && line[5] == 'C') {
        return SentenceType::RMC;
    }

    return SentenceType::Unknown;
}

/**
 * @brief 解析 GGA：定位质量、卫星数、HDOP、海拔和当前位置
 */
bool ProtocolGnss::ParseGga(const char *line, uint16_t len, DecodeResult *out)
{
    if (line == nullptr || out == nullptr) {
        return false;
    }

    GgaInfo gga{};
    gga.talker = out->talker;

    const char *field = nullptr;
    uint16_t fieldLen = 0;

    if (GetField(line, len, 1U, &field, &fieldLen)) {
        (void)ParseUtcTime(field, fieldLen, &gga.utc);
    }

    const char *lat = nullptr;
    const char *latHemi = nullptr;
    const char *lon = nullptr;
    const char *lonHemi = nullptr;
    uint16_t latLen = 0;
    uint16_t latHemiLen = 0;
    uint16_t lonLen = 0;
    uint16_t lonHemiLen = 0;

    const bool hasLat = GetField(line, len, 2U, &lat, &latLen);
    const bool hasLatHemi = GetField(line, len, 3U, &latHemi, &latHemiLen);
    const bool hasLon = GetField(line, len, 4U, &lon, &lonLen);
    const bool hasLonHemi = GetField(line, len, 5U, &lonHemi, &lonHemiLen);

    if (hasLat && hasLatHemi) {
        (void)ParseCoordinate(lat, latLen, latHemi, latHemiLen, true, &gga.coordinate);
    }
    if (hasLon && hasLonHemi) {
        (void)ParseCoordinate(lon, lonLen, lonHemi, lonHemiLen, false, &gga.coordinate);
    }

    if (GetField(line, len, 6U, &field, &fieldLen)) {
        const uint32_t quality = ParseUnsigned(field, fieldLen);
        gga.fixQuality = (quality <= 8U) ? static_cast<GgaFixQuality>(quality) : GgaFixQuality::Unknown;
    }

    if (GetField(line, len, 7U, &field, &fieldLen)) {
        const uint32_t count = ParseUnsigned(field, fieldLen);
        gga.satelliteCount = static_cast<uint8_t>(count > 255U ? 255U : count);
    }

    if (GetField(line, len, 8U, &field, &fieldLen)) {
        gga.hdopX100 = static_cast<uint16_t>(ParseDecimalScaled(field, fieldLen, 100U));
    }

    if (GetField(line, len, 9U, &field, &fieldLen)) {
        gga.altitudeCm = ParseSignedDecimalScaled(field, fieldLen, 100U);
    }

    gga.valid = gga.utc.timeValid &&
                gga.coordinate.IsValid() &&
                gga.fixQuality != GgaFixQuality::Invalid &&
                gga.fixQuality != GgaFixQuality::Unknown;

    out->gga = gga;
    return true;
}

/**
 * @brief 解析 RMC：有效状态、日期、速度和当前位置
 */
bool ProtocolGnss::ParseRmc(const char *line, uint16_t len, DecodeResult *out)
{
    if (line == nullptr || out == nullptr) {
        return false;
    }

    RmcInfo rmc{};
    rmc.talker = out->talker;

    const char *field = nullptr;
    uint16_t fieldLen = 0;

    if (GetField(line, len, 1U, &field, &fieldLen)) {
        (void)ParseUtcTime(field, fieldLen, &rmc.utc);
    }

    if (GetField(line, len, 2U, &field, &fieldLen)) {
        rmc.status = (fieldLen > 0U && field[0] == 'A') ? RmcStatus::Active : RmcStatus::Invalid;
    }

    const char *lat = nullptr;
    const char *latHemi = nullptr;
    const char *lon = nullptr;
    const char *lonHemi = nullptr;
    uint16_t latLen = 0;
    uint16_t latHemiLen = 0;
    uint16_t lonLen = 0;
    uint16_t lonHemiLen = 0;

    const bool hasLat = GetField(line, len, 3U, &lat, &latLen);
    const bool hasLatHemi = GetField(line, len, 4U, &latHemi, &latHemiLen);
    const bool hasLon = GetField(line, len, 5U, &lon, &lonLen);
    const bool hasLonHemi = GetField(line, len, 6U, &lonHemi, &lonHemiLen);

    if (hasLat && hasLatHemi) {
        (void)ParseCoordinate(lat, latLen, latHemi, latHemiLen, true, &rmc.coordinate);
    }
    if (hasLon && hasLonHemi) {
        (void)ParseCoordinate(lon, lonLen, lonHemi, lonHemiLen, false, &rmc.coordinate);
    }

    if (GetField(line, len, 7U, &field, &fieldLen)) {
        const uint32_t speedKnots = ParseDecimalScaled(field, fieldLen, 1000U);
        rmc.speedKnotsX1000 = static_cast<uint16_t>(speedKnots > 65535U ? 65535U : speedKnots);
        const uint32_t speedKmh = (speedKnots * KNOT_TO_KMH_X1000_NUM) / KNOT_TO_KMH_X1000_DEN;
        rmc.speedKmhX1000 = static_cast<uint16_t>(speedKmh > 65535U ? 65535U : speedKmh);
    }

    if (GetField(line, len, 9U, &field, &fieldLen)) {
        (void)ParseDate(field, fieldLen, &rmc.utc);
    }

    rmc.valid = rmc.utc.timeValid &&
                rmc.utc.dateValid &&
                rmc.coordinate.IsValid() &&
                rmc.status == RmcStatus::Active;

    out->rmc = rmc;
    return true;
}

/**
 * @brief 获取逗号分隔字段的起始指针和长度
 *
 * 不使用 strtok，避免修改原字符串和引入不可重入的静态状态；遇到 '*' 后停止，
 * 防止把 NMEA 校验字段当作普通数据字段。
 */
bool ProtocolGnss::GetField(const char *line,
                            uint16_t len,
                            uint8_t fieldIndex,
                            const char **field,
                            uint16_t *fieldLen)
{
    if (line == nullptr || field == nullptr || fieldLen == nullptr || len == 0U) {
        return false;
    }

    uint8_t current = 0;
    uint16_t start = 0;

    // 用 i <= len 是为了把“字符串结尾”也当作一个隐式分隔符，
    // 这样最后一个字段不需要末尾 ',' 也能正确返回
    for (uint16_t i = 0; i <= len; ++i) {
        const bool isEnd = (i == len);
        const bool isSeparator = (!isEnd && (line[i] == ',' || line[i] == '*'));

        if (isEnd || isSeparator) {
            if (current == fieldIndex) {
                // 返回的指针/长度是原缓冲区视图，避免 strtok 复制和静态状态
                *field = &line[start];
                *fieldLen = static_cast<uint16_t>(i - start);
                return true;
            }
            current++;
            start = static_cast<uint16_t>(i + 1U);
        }

        if (!isEnd && line[i] == '*') {
            // '*' 是校验字段的起点，其后内容不再属于业务字段
            break;
        }
    }

    return false;
}

bool ProtocolGnss::ParseUtcTime(const char *field, uint16_t fieldLen, DateTime *out)
{
    if (field == nullptr || out == nullptr || fieldLen < 6U) {
        return false;
    }

    for (uint8_t i = 0; i < 6U; ++i) {
        if (!IsDigit(field[i])) {
            return false;
        }
    }

    out->hour = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
    out->minute = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
    out->second = static_cast<uint8_t>((field[4] - '0') * 10 + (field[5] - '0'));
    out->millisecond = 0;

    if (fieldLen > 7U && field[6] == '.') {
        uint16_t ms = 0;
        uint16_t scale = 100;
        for (uint16_t i = 7U; i < fieldLen && scale > 0U; ++i) {
            if (!IsDigit(field[i])) {
                break;
            }
            ms = static_cast<uint16_t>(ms + static_cast<uint16_t>(field[i] - '0') * scale);
            scale = static_cast<uint16_t>(scale / 10U);
        }
        out->millisecond = ms;
    }

    out->timeValid = true;
    return true;
}

bool ProtocolGnss::ParseDate(const char *field, uint16_t fieldLen, DateTime *out)
{
    if (field == nullptr || out == nullptr || fieldLen < 6U) {
        return false;
    }

    for (uint8_t i = 0; i < 6U; ++i) {
        if (!IsDigit(field[i])) {
            return false;
        }
    }

    out->day = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
    out->month = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
    const uint8_t year2 = static_cast<uint8_t>((field[4] - '0') * 10 + (field[5] - '0'));
    out->year = static_cast<uint16_t>(2000U + year2);
    out->dateValid = true;
    return true;
}

/**
 * @brief 将 NMEA ddmm.mmmmm / dddmm.mmmmm 坐标转成 E7 定点十进制度
 *
 * NMEA 坐标采用“度 + 分”格式，业务侧更适合使用十进制度。这里使用 E7
 * 定点数避免在裸机协议层引入浮点依赖。
 */
bool ProtocolGnss::ParseCoordinate(const char *value,
                                   uint16_t valueLen,
                                   const char *hemisphere,
                                   uint16_t hemisphereLen,
                                   bool isLatitude,
                                   Coordinate *out)
{
    if (value == nullptr || hemisphere == nullptr || out == nullptr ||
        valueLen == 0U || hemisphereLen == 0U) {
        return false;
    }

    // NMEA 纬度用 dd（2 位），经度用 ddd（3 位），先按是否经纬度切分度的位数
    const uint8_t degreeDigits = isLatitude ? 2U : 3U;
    if (valueLen <= degreeDigits) {
        // 必须还有小数点+分秒部分，单独只有度数视为格式不合法
        return false;
    }

    uint32_t degrees = 0;
    for (uint8_t i = 0; i < degreeDigits; ++i) {
        if (!IsDigit(value[i])) {
            return false;
        }
        degrees = degrees * 10U + static_cast<uint32_t>(value[i] - '0');
    }

    const char *minuteField = value + degreeDigits;
    const uint16_t minuteLen = static_cast<uint16_t>(valueLen - degreeDigits);
    // 分钟部分统一按 1e5 倍率解析，与下面的度→E7 转换保持同一个数量级
    const uint32_t minutesX100000 = ParseDecimalScaled(minuteField, minuteLen, 100000U);

    // 度部分直接乘 1e7 得到 E7；分钟部分要做单位换算：
    //   minutes * (1e7 / 60) = minutes * 1e7 / 60
    // 这里用 64 位避免 degrees/minutes 较大时 int32 溢出
    const int32_t degreeE7 = static_cast<int32_t>(degrees * 10000000U);
    const int32_t minuteE7 = static_cast<int32_t>((static_cast<uint64_t>(minutesX100000) * 10000000ULL) /
                                                  (60ULL * 100000ULL));
    int32_t result = degreeE7 + minuteE7;

    const char hemi = hemisphere[0];
    // N/E 取正，S/W 取负；出现其他字符视为 NMEA 字段异常
    if (hemi == 'S' || hemi == 'W') {
        result = -result;
    } else if (hemi != 'N' && hemi != 'E') {
        return false;
    }

    if (isLatitude) {
        out->latitudeE7 = result;
        out->latitudeValid = true;
    } else {
        out->longitudeE7 = result;
        out->longitudeValid = true;
    }

    return true;
}

uint32_t ProtocolGnss::ParseUnsigned(const char *field, uint16_t fieldLen)
{
    if (field == nullptr || fieldLen == 0U) {
        return 0;
    }

    uint32_t value = 0;
    for (uint16_t i = 0; i < fieldLen; ++i) {
        if (!IsDigit(field[i])) {
            break;
        }
        value = value * 10U + static_cast<uint32_t>(field[i] - '0');
    }

    return value;
}

/**
 * @brief 将 ASCII 小数字段转换为指定倍率的无符号定点数
 *
 * 例如 "0.72" 按 scale=100 转为 72；"0.013" 按 scale=1000 转为 13。
 */
uint32_t ProtocolGnss::ParseDecimalScaled(const char *field, uint16_t fieldLen, uint32_t scale)
{
    if (field == nullptr || fieldLen == 0U || scale == 0U) {
        return 0;
    }

    uint32_t integerPart = 0;
    uint32_t decimalPart = 0;
    // decimalScale 表示当前小数部分已经积累到了 10 的几次幂，
    // 用于和目标 scale 对齐做右填充
    uint32_t decimalScale = 1;
    bool afterDot = false;

    for (uint16_t i = 0; i < fieldLen; ++i) {
        const char c = field[i];
        if (c == '.') {
            afterDot = true;
            continue;
        }
        if (!IsDigit(c)) {
            // 非数字视为字段结束，避免污染结果
            break;
        }

        if (!afterDot) {
            integerPart = integerPart * 10U + static_cast<uint32_t>(c - '0');
        } else if (decimalScale < scale) {
            // 只取到目标精度为止，避免小数位超过 scale 后被截断
            decimalPart = decimalPart * 10U + static_cast<uint32_t>(c - '0');
            decimalScale *= 10U;
        }
    }

    // 字段中提供的位数不足时（如 "0.7" 按 scale=100），右侧补 0 凑齐 scale
    while (decimalScale < scale) {
        decimalPart *= 10U;
        decimalScale *= 10U;
    }

    // 整数部分乘 scale 再加上小数部分，得到统一精度的定点值
    return integerPart * scale + decimalPart;
}

int32_t ProtocolGnss::ParseSignedDecimalScaled(const char *field, uint16_t fieldLen, uint32_t scale)
{
    if (field == nullptr || fieldLen == 0U) {
        return 0;
    }

    bool negative = false;
    uint16_t offset = 0;
    if (field[0] == '-') {
        negative = true;
        offset = 1;
    } else if (field[0] == '+') {
        offset = 1;
    }

    const uint32_t value = ParseDecimalScaled(field + offset,
                                              static_cast<uint16_t>(fieldLen - offset),
                                              scale);
    return negative ? -static_cast<int32_t>(value) : static_cast<int32_t>(value);
}

uint8_t ProtocolGnss::HexToNibble(char c)
{
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return static_cast<uint8_t>(c - 'A' + 10);
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<uint8_t>(c - 'a' + 10);
    }
    return 0xFFU;
}

uint16_t ProtocolGnss::TrimLineLength(const char *line, uint16_t len)
{
    if (line == nullptr) {
        return 0;
    }

    while (len > 0U && (line[len - 1U] == '\r' || line[len - 1U] == '\n' || line[len - 1U] == '\0')) {
        --len;
    }

    return len;
}
