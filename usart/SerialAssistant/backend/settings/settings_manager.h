/**
 * @file settings_manager.h
 * @brief Centralized persistent settings for SerialAssistant.
 */

#ifndef SERIAL_ASSISTANT_SETTINGS_MANAGER_H
#define SERIAL_ASSISTANT_SETTINGS_MANAGER_H

#include <QObject>
#include <QString>

/**
 * Provides typed access to persistent application settings.
 */
class SettingsManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)
    Q_PROPERTY(bool hexDisplay READ hexDisplay WRITE setHexDisplay NOTIFY hexDisplayChanged)
    Q_PROPERTY(bool timestampEnabled READ timestampEnabled WRITE setTimestampEnabled NOTIFY timestampEnabledChanged)
    Q_PROPERTY(QString timestampFormat READ timestampFormat WRITE setTimestampFormat NOTIFY timestampFormatChanged)
    Q_PROPERTY(int themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int uiDensity READ uiDensity WRITE setUiDensity NOTIFY uiDensityChanged)
    Q_PROPERTY(QString terminalFont READ terminalFont WRITE setTerminalFont NOTIFY terminalFontChanged)

public:
    explicit SettingsManager(QObject* parent = nullptr);

    [[nodiscard]] QString portName() const;
    [[nodiscard]] int baudRate() const;
    [[nodiscard]] int dataBits() const;
    [[nodiscard]] int parity() const;
    [[nodiscard]] int stopBits() const;
    [[nodiscard]] int flowControl() const;
    [[nodiscard]] bool autoScroll() const;
    [[nodiscard]] bool hexDisplay() const;
    [[nodiscard]] bool timestampEnabled() const;
    [[nodiscard]] QString timestampFormat() const;
    [[nodiscard]] int themeMode() const;
    [[nodiscard]] QString accentColor() const;
    [[nodiscard]] int uiDensity() const;
    [[nodiscard]] QString terminalFont() const;
    [[nodiscard]] bool hexSend() const;
    [[nodiscard]] int lineEnding() const;
    [[nodiscard]] bool loopSend() const;
    [[nodiscard]] int loopIntervalMs() const;

    void setPortName(const QString& portName);
    void setBaudRate(int baudRate);
    void setDataBits(int dataBits);
    void setParity(int parity);
    void setStopBits(int stopBits);
    void setFlowControl(int flowControl);
    void setAutoScroll(bool autoScroll);
    void setHexDisplay(bool hexDisplay);
    void setTimestampEnabled(bool timestampEnabled);
    void setTimestampFormat(const QString& timestampFormat);
    void setThemeMode(int themeMode);
    void setAccentColor(const QString& accentColor);
    void setUiDensity(int uiDensity);
    void setTerminalFont(const QString& terminalFont);
    void setHexSend(bool hexSend);
    void setLineEnding(int lineEnding);
    void setLoopSend(bool loopSend);
    void setLoopIntervalMs(int loopIntervalMs);

public slots:
    void restoreDefaults();

signals:
    void serialSettingsChanged();
    void autoScrollChanged();
    void hexDisplayChanged();
    void timestampEnabledChanged();
    void timestampFormatChanged();
    void themeModeChanged();
    void accentColorChanged();
    void uiDensityChanged();
    void terminalFontChanged();
    void transmitSettingsChanged();

private:
    void load();
    void saveValue(const QString& key, const QVariant& value) const;

    QString m_portName;
    int m_baudRate;
    int m_dataBits;
    int m_parity;
    int m_stopBits;
    int m_flowControl;
    bool m_autoScroll;
    bool m_hexDisplay;
    bool m_timestampEnabled;
    QString m_timestampFormat;
    int m_themeMode;
    QString m_accentColor;
    int m_uiDensity;
    QString m_terminalFont;
    bool m_hexSend;
    int m_lineEnding;
    bool m_loopSend;
    int m_loopIntervalMs;
};

#endif
