/**
 * @file settings_manager.cpp
 * @brief SettingsManager implementation.
 */

#include "settings_manager.h"

#include <QSerialPort>
#include <QSettings>
#include <QVariant>

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
constexpr auto kTimestampFormatKey = "console/timestampFormat";
constexpr auto kThemeModeKey = "appearance/themeMode";
constexpr auto kAccentColorKey = "appearance/accentColor";
constexpr auto kUiDensityKey = "appearance/uiDensity";
constexpr auto kTerminalFontKey = "appearance/terminalFont";
constexpr auto kHexSendKey = "transmit/hexSend";
constexpr auto kLineEndingKey = "transmit/lineEnding";
constexpr auto kLegacyLineEndingKey = "serial/lineEnding";
constexpr auto kLoopSendKey = "transmit/loopSend";
constexpr auto kLoopIntervalMsKey = "transmit/loopIntervalMs";
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
    , m_timestampFormat(QStringLiteral("HH:mm:ss.SSS"))
    , m_themeMode(0)
    , m_accentColor(QStringLiteral("#00AFA0"))
    , m_uiDensity(1)
    , m_terminalFont(QStringLiteral("Consolas"))
    , m_hexSend(false)
    , m_lineEnding(0)
    , m_loopSend(false)
    , m_loopIntervalMs(200)
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
QString SettingsManager::timestampFormat() const { return m_timestampFormat; }
bool SettingsManager::hexSend() const { return m_hexSend; }
int SettingsManager::lineEnding() const { return m_lineEnding; }
bool SettingsManager::loopSend() const { return m_loopSend; }
int SettingsManager::loopIntervalMs() const { return m_loopIntervalMs; }

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
    if (m_baudRate == baudRate)
        return;
    m_baudRate = baudRate;
    saveValue(QString::fromLatin1(kBaudRateKey), baudRate);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setDataBits(int dataBits)
{
    if (m_dataBits == dataBits)
        return;
    m_dataBits = dataBits;
    saveValue(QString::fromLatin1(kDataBitsKey), dataBits);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setParity(int parity)
{
    if (m_parity == parity)
        return;
    m_parity = parity;
    saveValue(QString::fromLatin1(kParityKey), parity);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setStopBits(int stopBits)
{
    if (m_stopBits == stopBits)
        return;
    m_stopBits = stopBits;
    saveValue(QString::fromLatin1(kStopBitsKey), stopBits);
    Q_EMIT serialSettingsChanged();
}

void SettingsManager::setFlowControl(int flowControl)
{
    if (m_flowControl == flowControl)
        return;
    m_flowControl = flowControl;
    saveValue(QString::fromLatin1(kFlowControlKey), flowControl);
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
    if (m_uiDensity == uiDensity)
        return;
    m_uiDensity = uiDensity;
    saveValue(QString::fromLatin1(kUiDensityKey), uiDensity);
    Q_EMIT uiDensityChanged();
}

void SettingsManager::setTerminalFont(const QString& terminalFont)
{
    if (m_terminalFont == terminalFont)
        return;
    m_terminalFont = terminalFont;
    saveValue(QString::fromLatin1(kTerminalFontKey), terminalFont);
    Q_EMIT terminalFontChanged();
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
    setTimestampFormat(QStringLiteral("HH:mm:ss.SSS"));
    setThemeMode(0);
    setAccentColor(QStringLiteral("#00AFA0"));
    setUiDensity(1);
    setTerminalFont(QStringLiteral("Consolas"));
    setHexSend(false);
    setLineEnding(0);
    setLoopSend(false);
    setLoopIntervalMs(200);
}

void SettingsManager::load()
{
    const QSettings settings;
    m_portName = settings.value(QString::fromLatin1(kPortNameKey)).toString();
    m_baudRate = settings.value(QString::fromLatin1(kBaudRateKey), m_baudRate).toInt();
    m_dataBits = settings.value(QString::fromLatin1(kDataBitsKey), m_dataBits).toInt();
    m_parity = settings.value(QString::fromLatin1(kParityKey), m_parity).toInt();
    m_stopBits = settings.value(QString::fromLatin1(kStopBitsKey), m_stopBits).toInt();
    m_flowControl = settings.value(QString::fromLatin1(kFlowControlKey), m_flowControl).toInt();
    m_autoScroll = settings.value(QString::fromLatin1(kAutoScrollKey), m_autoScroll).toBool();
    m_hexDisplay = settings.value(QString::fromLatin1(kHexDisplayKey), m_hexDisplay).toBool();
    m_timestampEnabled = settings.value(QString::fromLatin1(kTimestampEnabledKey), m_timestampEnabled).toBool();
    m_timestampFormat = settings.value(QString::fromLatin1(kTimestampFormatKey), m_timestampFormat).toString();
    m_themeMode = settings.value(QString::fromLatin1(kThemeModeKey), m_themeMode).toInt();
    m_accentColor = settings.value(QString::fromLatin1(kAccentColorKey), m_accentColor).toString();
    m_uiDensity = settings.value(QString::fromLatin1(kUiDensityKey), m_uiDensity).toInt();
    m_terminalFont = settings.value(QString::fromLatin1(kTerminalFontKey), m_terminalFont).toString();
    m_hexSend = settings.value(QString::fromLatin1(kHexSendKey), m_hexSend).toBool();
    m_lineEnding = settings.value(QString::fromLatin1(kLineEndingKey),
                                  settings.value(QString::fromLatin1(kLegacyLineEndingKey), m_lineEnding))
                     .toInt();
    m_loopSend = settings.value(QString::fromLatin1(kLoopSendKey), m_loopSend).toBool();
    m_loopIntervalMs = qBound(10,
                              settings.value(QString::fromLatin1(kLoopIntervalMsKey), m_loopIntervalMs).toInt(),
                              60000);
}

void SettingsManager::saveValue(const QString& key, const QVariant& value) const
{
    QSettings settings;
    settings.setValue(key, value);
}
