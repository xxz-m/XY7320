/**
 * @file settings_manager.cpp
 * @brief SettingsManager implementation.
 */

#include "settings_manager.h"

#include <QSerialPort>
#include <QSettings>
#include <QVariant>
#include <QtGlobal>

namespace {
constexpr auto kPortNameKey = "serial/portName";
constexpr auto kBaudRateKey = "serial/baudRate";
constexpr auto kDataBitsKey = "serial/dataBits";
constexpr auto kParityKey = "serial/parity";
constexpr auto kStopBitsKey = "serial/stopBits";
constexpr auto kFlowControlKey = "serial/flowControl";
constexpr auto kAutoScrollKey = "console/autoScroll";
constexpr auto kHexDisplayKey = "console/hexDisplay";
constexpr auto kTimestampEnabledKey = "console/timestampEnabled";
constexpr auto kTimestampScopeKey = "console/timestampScope";
constexpr auto kTimestampFormatKey = "console/timestampFormat";
constexpr auto kThemeModeKey = "appearance/themeMode";
constexpr auto kAccentColorKey = "appearance/accentColor";
constexpr auto kUiDensityKey = "appearance/uiDensity";
constexpr auto kTerminalFontKey = "appearance/terminalFont";
constexpr auto kTerminalFontSizeKey = "appearance/terminalFontSize";
constexpr auto kHighlightEnabledKey = "highlight/enabled";
constexpr auto kHighlightCaseSensitiveKey = "highlight/caseSensitive";
constexpr auto kHighlightRegexKey = "highlight/regex";
constexpr auto kHexSendKey = "transmit/hexSend";
constexpr auto kLineEndingKey = "transmit/lineEnding";
constexpr auto kLegacyLineEndingKey = "serial/lineEnding";
constexpr auto kLoopSendKey = "transmit/loopSend";
constexpr auto kLoopIntervalMsKey = "transmit/loopIntervalMs";
constexpr auto kDtrEnabledKey = "serial/dtrEnabled";
constexpr auto kRtsEnabledKey = "serial/rtsEnabled";
constexpr auto kAutoOpenKey = "serial/autoOpen";
constexpr auto kRememberPortKey = "serial/rememberPort";
constexpr auto kReceiveEncodingKey = "encoding/receiveEncoding";
constexpr auto kSendEncodingKey = "encoding/sendEncoding";
constexpr auto kInvalidBytePolicyKey = "encoding/invalidBytePolicy";
constexpr auto kEnterToSendKey = "input/enterToSend";
constexpr auto kAppendNewlineKey = "input/appendNewline";
constexpr auto kMultilineInputKey = "input/multilineInput";

int normalizeBaudRate(int baudRate)
{
    switch (baudRate) {
    case QSerialPort::Baud1200:
    case QSerialPort::Baud2400:
    case QSerialPort::Baud4800:
    case QSerialPort::Baud9600:
    case QSerialPort::Baud19200:
    case QSerialPort::Baud38400:
    case QSerialPort::Baud57600:
    case QSerialPort::Baud115200:
    case 230400:
    case 460800:
    case 921600:
        return baudRate;
    default:
        return QSerialPort::Baud115200;
    }
}

int normalizeDataBits(int dataBits)
{
    switch (dataBits) {
    case QSerialPort::Data5:
    case QSerialPort::Data6:
    case QSerialPort::Data7:
    case QSerialPort::Data8:
        return dataBits;
    default:
        return QSerialPort::Data8;
    }
}

int normalizeParity(int parity)
{
    switch (parity) {
    case QSerialPort::NoParity:
    case QSerialPort::EvenParity:
    case QSerialPort::OddParity:
    case QSerialPort::SpaceParity:
    case QSerialPort::MarkParity:
        return parity;
    default:
        return QSerialPort::NoParity;
    }
}

int normalizeStopBits(int stopBits)
{
    switch (stopBits) {
    case QSerialPort::OneStop:
    case QSerialPort::OneAndHalfStop:
    case QSerialPort::TwoStop:
        return stopBits;
    default:
        return QSerialPort::OneStop;
    }
}

int normalizeFlowControl(int flowControl)
{
    switch (flowControl) {
    case QSerialPort::NoFlowControl:
    case QSerialPort::HardwareControl:
    case QSerialPort::SoftwareControl:
        return flowControl;
    default:
        return QSerialPort::NoFlowControl;
    }
}

int normalizeTimestampScope(int timestampScope)
{
    return timestampScope == 1 ? 1 : 0;
}

QString normalizeEncoding(QString encoding)
{
    encoding = encoding.trimmed();
    if (encoding.compare(QStringLiteral("GBK"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("GBK");
    if (encoding.compare(QStringLiteral("ASCII"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("ASCII");
    if (encoding.compare(QStringLiteral("Latin-1"), Qt::CaseInsensitive) == 0
        || encoding.compare(QStringLiteral("Latin1"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("Latin-1");
    return QStringLiteral("UTF-8");
}

int normalizeInvalidBytePolicy(int invalidBytePolicy)
{
    return qBound(0, invalidBytePolicy, 2);
}
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_baudRate(QSerialPort::Baud115200)
    , m_dataBits(QSerialPort::Data8)
    , m_parity(QSerialPort::NoParity)
    , m_stopBits(QSerialPort::OneStop)
    , m_flowControl(QSerialPort::NoFlowControl)
    , m_autoScroll(true)
    , m_hexDisplay(false)
    , m_timestampEnabled(true)
    , m_timestampScope(0)
    , m_timestampFormat(QStringLiteral("HH:mm:ss.SSS"))
    , m_themeMode(0)
    , m_accentColor(QStringLiteral("#00AFA0"))
    , m_uiDensity(1)
    , m_terminalFont(QStringLiteral("Consolas"))
    , m_terminalFontSize(12)
    , m_highlightEnabled(true)
    , m_highlightCaseSensitive(false)
    , m_highlightRegex(false)
    , m_hexSend(false)
    , m_lineEnding(0)
    , m_loopSend(false)
    , m_loopIntervalMs(200)
    , m_dtrEnabled(false)
    , m_rtsEnabled(false)
    , m_autoOpen(false)
    , m_rememberPort(true)
    , m_receiveEncoding(QStringLiteral("UTF-8"))
    , m_sendEncoding(QStringLiteral("UTF-8"))
    , m_invalidBytePolicy(0)
    , m_enterToSend(false)
    , m_appendNewline(false)
    , m_multilineInput(true)
{
    load();
}

QString SettingsManager::portName() const { return m_portName; }
int SettingsManager::baudRate() const { return m_baudRate; }
int SettingsManager::dataBits() const { return m_dataBits; }
int SettingsManager::parity() const { return m_parity; }
int SettingsManager::stopBits() const { return m_stopBits; }
int SettingsManager::flowControl() const { return m_flowControl; }
bool SettingsManager::autoScroll() const { return m_autoScroll; }
bool SettingsManager::hexDisplay() const { return m_hexDisplay; }
bool SettingsManager::timestampEnabled() const { return m_timestampEnabled; }
int SettingsManager::timestampScope() const { return m_timestampScope; }
QString SettingsManager::timestampFormat() const { return m_timestampFormat; }
bool SettingsManager::hexSend() const { return m_hexSend; }
int SettingsManager::lineEnding() const { return m_lineEnding; }
bool SettingsManager::loopSend() const { return m_loopSend; }
int SettingsManager::loopIntervalMs() const { return m_loopIntervalMs; }
bool SettingsManager::dtrEnabled() const { return m_dtrEnabled; }
bool SettingsManager::rtsEnabled() const { return m_rtsEnabled; }
bool SettingsManager::autoOpen() const { return m_autoOpen; }
bool SettingsManager::rememberPort() const { return m_rememberPort; }
QString SettingsManager::receiveEncoding() const { return m_receiveEncoding; }
QString SettingsManager::sendEncoding() const { return m_sendEncoding; }
int SettingsManager::invalidBytePolicy() const { return m_invalidBytePolicy; }
bool SettingsManager::enterToSend() const { return m_enterToSend; }
bool SettingsManager::appendNewline() const { return m_appendNewline; }
bool SettingsManager::multilineInput() const { return m_multilineInput; }
int SettingsManager::terminalFontSize() const { return m_terminalFontSize; }
bool SettingsManager::highlightEnabled() const { return m_highlightEnabled; }
bool SettingsManager::highlightCaseSensitive() const { return m_highlightCaseSensitive; }
bool SettingsManager::highlightRegex() const { return m_highlightRegex; }

void SettingsManager::setPortName(const QString& portName)
{
    if (m_portName == portName)
        return;
    m_portName = portName;
    saveValue(QString::fromLatin1(kPortNameKey), portName);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setBaudRate(int baudRate)
{
    const int normalizedBaudRate = normalizeBaudRate(baudRate);
    if (m_baudRate == normalizedBaudRate)
        return;
    m_baudRate = normalizedBaudRate;
    saveValue(QString::fromLatin1(kBaudRateKey), normalizedBaudRate);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setDataBits(int dataBits)
{
    const int normalizedDataBits = normalizeDataBits(dataBits);
    if (m_dataBits == normalizedDataBits)
        return;
    m_dataBits = normalizedDataBits;
    saveValue(QString::fromLatin1(kDataBitsKey), normalizedDataBits);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setParity(int parity)
{
    const int normalizedParity = normalizeParity(parity);
    if (m_parity == normalizedParity)
        return;
    m_parity = normalizedParity;
    saveValue(QString::fromLatin1(kParityKey), normalizedParity);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setStopBits(int stopBits)
{
    const int normalizedStopBits = normalizeStopBits(stopBits);
    if (m_stopBits == normalizedStopBits)
        return;
    m_stopBits = normalizedStopBits;
    saveValue(QString::fromLatin1(kStopBitsKey), normalizedStopBits);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setFlowControl(int flowControl)
{
    const int normalizedFlowControl = normalizeFlowControl(flowControl);
    if (m_flowControl == normalizedFlowControl)
        return;
    m_flowControl = normalizedFlowControl;
    saveValue(QString::fromLatin1(kFlowControlKey), normalizedFlowControl);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setAutoScroll(bool autoScroll)
{
    if (m_autoScroll == autoScroll)
        return;
    m_autoScroll = autoScroll;
    saveValue(QString::fromLatin1(kAutoScrollKey), autoScroll);
    Q_EMIT autoScrollChanged();
}

void SettingsManager::setHexDisplay(bool hexDisplay)
{
    if (m_hexDisplay == hexDisplay)
        return;
    m_hexDisplay = hexDisplay;
    saveValue(QString::fromLatin1(kHexDisplayKey), hexDisplay);
    Q_EMIT hexDisplayChanged();
}

void SettingsManager::setTimestampEnabled(bool timestampEnabled)
{
    if (m_timestampEnabled == timestampEnabled)
        return;
    m_timestampEnabled = timestampEnabled;
    saveValue(QString::fromLatin1(kTimestampEnabledKey), timestampEnabled);
    Q_EMIT timestampEnabledChanged();
}

void SettingsManager::setTimestampScope(int timestampScope)
{
    const int normalizedScope = normalizeTimestampScope(timestampScope);
    if (m_timestampScope == normalizedScope)
        return;
    m_timestampScope = normalizedScope;
    saveValue(QString::fromLatin1(kTimestampScopeKey), normalizedScope);
    Q_EMIT timestampScopeChanged();
}

void SettingsManager::setTimestampFormat(const QString& timestampFormat)
{
    if (m_timestampFormat == timestampFormat)
        return;
    m_timestampFormat = timestampFormat;
    saveValue(QString::fromLatin1(kTimestampFormatKey), timestampFormat);
    Q_EMIT timestampFormatChanged();
}

int SettingsManager::themeMode() const { return m_themeMode; }
QString SettingsManager::accentColor() const { return m_accentColor; }
int SettingsManager::uiDensity() const { return m_uiDensity; }
QString SettingsManager::terminalFont() const { return m_terminalFont; }

void SettingsManager::setThemeMode(int themeMode)
{
    if (m_themeMode == themeMode)
        return;
    m_themeMode = themeMode;
    saveValue(QString::fromLatin1(kThemeModeKey), themeMode);
    Q_EMIT themeModeChanged();
}

void SettingsManager::setAccentColor(const QString& accentColor)
{
    if (m_accentColor == accentColor)
        return;
    m_accentColor = accentColor;
    saveValue(QString::fromLatin1(kAccentColorKey), accentColor);
    Q_EMIT accentColorChanged();
}

void SettingsManager::setUiDensity(int uiDensity)
{
    const int boundedDensity = qBound(0, uiDensity, 2);
    if (m_uiDensity == boundedDensity)
        return;
    m_uiDensity = boundedDensity;
    saveValue(QString::fromLatin1(kUiDensityKey), boundedDensity);
    Q_EMIT uiDensityChanged();
    Q_EMIT displaySettingsChanged();
}

void SettingsManager::setTerminalFont(const QString& terminalFont)
{
    const QString nextFont = terminalFont.trimmed().isEmpty() ? QStringLiteral("Consolas") : terminalFont.trimmed();
    if (m_terminalFont == nextFont)
        return;
    m_terminalFont = nextFont;
    saveValue(QString::fromLatin1(kTerminalFontKey), nextFont);
    Q_EMIT terminalFontChanged();
    Q_EMIT displaySettingsChanged();
}

void SettingsManager::setTerminalFontSize(int terminalFontSize)
{
    const int boundedSize = qBound(9, terminalFontSize, 24);
    if (m_terminalFontSize == boundedSize)
        return;
    m_terminalFontSize = boundedSize;
    saveValue(QString::fromLatin1(kTerminalFontSizeKey), boundedSize);
    Q_EMIT displaySettingsChanged();
}

void SettingsManager::setHighlightEnabled(bool highlightEnabled)
{
    if (m_highlightEnabled == highlightEnabled)
        return;
    m_highlightEnabled = highlightEnabled;
    saveValue(QString::fromLatin1(kHighlightEnabledKey), highlightEnabled);
    Q_EMIT highlightSettingsChanged();
}

void SettingsManager::setHighlightCaseSensitive(bool highlightCaseSensitive)
{
    if (m_highlightCaseSensitive == highlightCaseSensitive)
        return;
    m_highlightCaseSensitive = highlightCaseSensitive;
    saveValue(QString::fromLatin1(kHighlightCaseSensitiveKey), highlightCaseSensitive);
    Q_EMIT highlightSettingsChanged();
}

void SettingsManager::setHighlightRegex(bool highlightRegex)
{
    if (m_highlightRegex == highlightRegex)
        return;
    m_highlightRegex = highlightRegex;
    saveValue(QString::fromLatin1(kHighlightRegexKey), highlightRegex);
    Q_EMIT highlightSettingsChanged();
}

void SettingsManager::setHexSend(bool hexSend)
{
    if (m_hexSend == hexSend)
        return;
    m_hexSend = hexSend;
    saveValue(QString::fromLatin1(kHexSendKey), hexSend);
    Q_EMIT transmitSettingsChanged();
}

void SettingsManager::setLineEnding(int lineEnding)
{
    if (m_lineEnding == lineEnding)
        return;
    m_lineEnding = lineEnding;
    saveValue(QString::fromLatin1(kLineEndingKey), lineEnding);
    Q_EMIT transmitSettingsChanged();
}

void SettingsManager::setLoopSend(bool loopSend)
{
    if (m_loopSend == loopSend)
        return;
    m_loopSend = loopSend;
    saveValue(QString::fromLatin1(kLoopSendKey), loopSend);
    Q_EMIT transmitSettingsChanged();
}

void SettingsManager::setLoopIntervalMs(int loopIntervalMs)
{
    const int boundedInterval = qBound(10, loopIntervalMs, 60000);
    if (m_loopIntervalMs == boundedInterval)
        return;
    m_loopIntervalMs = boundedInterval;
    saveValue(QString::fromLatin1(kLoopIntervalMsKey), boundedInterval);
    Q_EMIT transmitSettingsChanged();
}

void SettingsManager::setDtrEnabled(bool dtrEnabled)
{
    if (m_dtrEnabled == dtrEnabled)
        return;
    m_dtrEnabled = dtrEnabled;
    saveValue(QString::fromLatin1(kDtrEnabledKey), dtrEnabled);
    Q_EMIT serialLineSettingsChanged();
}

void SettingsManager::setRtsEnabled(bool rtsEnabled)
{
    if (m_rtsEnabled == rtsEnabled)
        return;
    m_rtsEnabled = rtsEnabled;
    saveValue(QString::fromLatin1(kRtsEnabledKey), rtsEnabled);
    Q_EMIT serialLineSettingsChanged();
}

void SettingsManager::setAutoOpen(bool autoOpen)
{
    if (m_autoOpen == autoOpen)
        return;
    m_autoOpen = autoOpen;
    saveValue(QString::fromLatin1(kAutoOpenKey), autoOpen);
    Q_EMIT serialStartupSettingsChanged();
}

void SettingsManager::setRememberPort(bool rememberPort)
{
    if (m_rememberPort == rememberPort)
        return;
    m_rememberPort = rememberPort;
    saveValue(QString::fromLatin1(kRememberPortKey), rememberPort);
    Q_EMIT serialStartupSettingsChanged();
}

void SettingsManager::setReceiveEncoding(const QString& receiveEncoding)
{
    const QString normalizedEncoding = normalizeEncoding(receiveEncoding);
    if (m_receiveEncoding == normalizedEncoding)
        return;
    m_receiveEncoding = normalizedEncoding;
    saveValue(QString::fromLatin1(kReceiveEncodingKey), normalizedEncoding);
    Q_EMIT encodingSettingsChanged();
}

void SettingsManager::setSendEncoding(const QString& sendEncoding)
{
    const QString normalizedEncoding = normalizeEncoding(sendEncoding);
    if (m_sendEncoding == normalizedEncoding)
        return;
    m_sendEncoding = normalizedEncoding;
    saveValue(QString::fromLatin1(kSendEncodingKey), normalizedEncoding);
    Q_EMIT encodingSettingsChanged();
}

void SettingsManager::setInvalidBytePolicy(int invalidBytePolicy)
{
    const int normalizedPolicy = normalizeInvalidBytePolicy(invalidBytePolicy);
    if (m_invalidBytePolicy == normalizedPolicy)
        return;
    m_invalidBytePolicy = normalizedPolicy;
    saveValue(QString::fromLatin1(kInvalidBytePolicyKey), normalizedPolicy);
    Q_EMIT encodingSettingsChanged();
}

void SettingsManager::setEnterToSend(bool enterToSend)
{
    if (m_enterToSend == enterToSend)
        return;
    m_enterToSend = enterToSend;
    saveValue(QString::fromLatin1(kEnterToSendKey), enterToSend);
    Q_EMIT inputBehaviorChanged();
}

void SettingsManager::setAppendNewline(bool appendNewline)
{
    if (m_appendNewline == appendNewline)
        return;
    m_appendNewline = appendNewline;
    saveValue(QString::fromLatin1(kAppendNewlineKey), appendNewline);
    Q_EMIT inputBehaviorChanged();
}

void SettingsManager::setMultilineInput(bool multilineInput)
{
    if (m_multilineInput == multilineInput)
        return;
    m_multilineInput = multilineInput;
    saveValue(QString::fromLatin1(kMultilineInputKey), multilineInput);
    Q_EMIT inputBehaviorChanged();
}

void SettingsManager::restoreDefaults()
{
    setPortName(QString());
    setBaudRate(QSerialPort::Baud115200);
    setDataBits(QSerialPort::Data8);
    setParity(QSerialPort::NoParity);
    setStopBits(QSerialPort::OneStop);
    setFlowControl(QSerialPort::NoFlowControl);
    setAutoScroll(true);
    setHexDisplay(false);
    setTimestampEnabled(true);
    setTimestampScope(0);
    setTimestampFormat(QStringLiteral("HH:mm:ss.SSS"));
    setThemeMode(0);
    setAccentColor(QStringLiteral("#00AFA0"));
    setUiDensity(1);
    setTerminalFont(QStringLiteral("Consolas"));
    setTerminalFontSize(12);
    setHighlightEnabled(true);
    setHighlightCaseSensitive(false);
    setHighlightRegex(false);
    setHexSend(false);
    setLineEnding(0);
    setLoopSend(false);
    setLoopIntervalMs(200);
    setDtrEnabled(false);
    setRtsEnabled(false);
    setAutoOpen(false);
    setRememberPort(true);
    setReceiveEncoding(QStringLiteral("UTF-8"));
    setSendEncoding(QStringLiteral("UTF-8"));
    setInvalidBytePolicy(0);
    setEnterToSend(false);
    setAppendNewline(false);
    setMultilineInput(true);
}

void SettingsManager::load()
{
    const QSettings settings;
    m_portName = settings.value(QString::fromLatin1(kPortNameKey)).toString();
    m_baudRate = normalizeBaudRate(settings.value(QString::fromLatin1(kBaudRateKey), m_baudRate).toInt());
    m_dataBits = normalizeDataBits(settings.value(QString::fromLatin1(kDataBitsKey), m_dataBits).toInt());
    m_parity = normalizeParity(settings.value(QString::fromLatin1(kParityKey), m_parity).toInt());
    m_stopBits = normalizeStopBits(settings.value(QString::fromLatin1(kStopBitsKey), m_stopBits).toInt());
    m_flowControl = normalizeFlowControl(settings.value(QString::fromLatin1(kFlowControlKey), m_flowControl).toInt());
    m_autoScroll = settings.value(QString::fromLatin1(kAutoScrollKey), m_autoScroll).toBool();
    m_hexDisplay = settings.value(QString::fromLatin1(kHexDisplayKey), m_hexDisplay).toBool();
    m_timestampEnabled = settings.value(QString::fromLatin1(kTimestampEnabledKey), m_timestampEnabled).toBool();
    m_timestampScope = normalizeTimestampScope(settings.value(QString::fromLatin1(kTimestampScopeKey), m_timestampScope).toInt());
    m_timestampFormat = settings.value(QString::fromLatin1(kTimestampFormatKey), m_timestampFormat).toString();
    m_themeMode = settings.value(QString::fromLatin1(kThemeModeKey), m_themeMode).toInt();
    m_accentColor = settings.value(QString::fromLatin1(kAccentColorKey), m_accentColor).toString();
    m_uiDensity = qBound(0, settings.value(QString::fromLatin1(kUiDensityKey), m_uiDensity).toInt(), 2);
    m_terminalFont = settings.value(QString::fromLatin1(kTerminalFontKey), m_terminalFont).toString().trimmed();
    if (m_terminalFont.isEmpty())
        m_terminalFont = QStringLiteral("Consolas");
    m_terminalFontSize = qBound(9, settings.value(QString::fromLatin1(kTerminalFontSizeKey), m_terminalFontSize).toInt(), 24);
    m_highlightEnabled = settings.value(QString::fromLatin1(kHighlightEnabledKey), m_highlightEnabled).toBool();
    m_highlightCaseSensitive = settings.value(QString::fromLatin1(kHighlightCaseSensitiveKey), m_highlightCaseSensitive).toBool();
    m_highlightRegex = settings.value(QString::fromLatin1(kHighlightRegexKey), m_highlightRegex).toBool();
    m_hexSend = settings.value(QString::fromLatin1(kHexSendKey), m_hexSend).toBool();
    m_lineEnding = settings.value(QString::fromLatin1(kLineEndingKey),
                                  settings.value(QString::fromLatin1(kLegacyLineEndingKey), m_lineEnding))
                     .toInt();
    m_loopSend = settings.value(QString::fromLatin1(kLoopSendKey), m_loopSend).toBool();
    m_loopIntervalMs = qBound(10,
                              settings.value(QString::fromLatin1(kLoopIntervalMsKey), m_loopIntervalMs).toInt(),
                              60000);
    m_dtrEnabled = settings.value(QString::fromLatin1(kDtrEnabledKey), m_dtrEnabled).toBool();
    m_rtsEnabled = settings.value(QString::fromLatin1(kRtsEnabledKey), m_rtsEnabled).toBool();
    m_autoOpen = settings.value(QString::fromLatin1(kAutoOpenKey), m_autoOpen).toBool();
    m_rememberPort = settings.value(QString::fromLatin1(kRememberPortKey), m_rememberPort).toBool();
    m_receiveEncoding = normalizeEncoding(settings.value(QString::fromLatin1(kReceiveEncodingKey), m_receiveEncoding).toString());
    m_sendEncoding = normalizeEncoding(settings.value(QString::fromLatin1(kSendEncodingKey), m_sendEncoding).toString());
    m_invalidBytePolicy = normalizeInvalidBytePolicy(settings.value(QString::fromLatin1(kInvalidBytePolicyKey), m_invalidBytePolicy).toInt());
    m_enterToSend = settings.value(QString::fromLatin1(kEnterToSendKey), m_enterToSend).toBool();
    m_appendNewline = settings.value(QString::fromLatin1(kAppendNewlineKey), m_appendNewline).toBool();
    m_multilineInput = settings.value(QString::fromLatin1(kMultilineInputKey), m_multilineInput).toBool();
}

void SettingsManager::saveValue(const QString& key, const QVariant& value) const
{
    QSettings settings;
    settings.setValue(key, value);
}
