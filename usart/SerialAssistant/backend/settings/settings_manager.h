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
    Q_PROPERTY(int timestampScope READ timestampScope WRITE setTimestampScope NOTIFY timestampScopeChanged)
    Q_PROPERTY(QString timestampFormat READ timestampFormat WRITE setTimestampFormat NOTIFY timestampFormatChanged)
    Q_PROPERTY(int themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int uiDensity READ uiDensity WRITE setUiDensity NOTIFY displaySettingsChanged)
    Q_PROPERTY(QString terminalFont READ terminalFont WRITE setTerminalFont NOTIFY displaySettingsChanged)
    Q_PROPERTY(int terminalFontSize READ terminalFontSize WRITE setTerminalFontSize NOTIFY displaySettingsChanged)
    Q_PROPERTY(bool highlightEnabled READ highlightEnabled WRITE setHighlightEnabled NOTIFY highlightSettingsChanged)
    Q_PROPERTY(bool highlightCaseSensitive READ highlightCaseSensitive WRITE setHighlightCaseSensitive NOTIFY highlightSettingsChanged)
    Q_PROPERTY(bool highlightRegex READ highlightRegex WRITE setHighlightRegex NOTIFY highlightSettingsChanged)
    Q_PROPERTY(bool dtrEnabled READ dtrEnabled WRITE setDtrEnabled NOTIFY serialLineSettingsChanged)
    Q_PROPERTY(bool rtsEnabled READ rtsEnabled WRITE setRtsEnabled NOTIFY serialLineSettingsChanged)
    Q_PROPERTY(bool autoOpen READ autoOpen WRITE setAutoOpen NOTIFY serialStartupSettingsChanged)
    Q_PROPERTY(bool rememberPort READ rememberPort WRITE setRememberPort NOTIFY serialStartupSettingsChanged)
    Q_PROPERTY(QString receiveEncoding READ receiveEncoding WRITE setReceiveEncoding NOTIFY encodingSettingsChanged)
    Q_PROPERTY(QString sendEncoding READ sendEncoding WRITE setSendEncoding NOTIFY encodingSettingsChanged)
    Q_PROPERTY(int invalidBytePolicy READ invalidBytePolicy WRITE setInvalidBytePolicy NOTIFY encodingSettingsChanged)
    Q_PROPERTY(bool enterToSend READ enterToSend WRITE setEnterToSend NOTIFY inputBehaviorChanged)
    Q_PROPERTY(bool appendNewline READ appendNewline WRITE setAppendNewline NOTIFY inputBehaviorChanged)
    Q_PROPERTY(bool multilineInput READ multilineInput WRITE setMultilineInput NOTIFY inputBehaviorChanged)

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
    [[nodiscard]] int timestampScope() const;
    [[nodiscard]] QString timestampFormat() const;
    [[nodiscard]] int themeMode() const;
    [[nodiscard]] QString accentColor() const;
    [[nodiscard]] int uiDensity() const;
    [[nodiscard]] QString terminalFont() const;
    [[nodiscard]] int terminalFontSize() const;
    [[nodiscard]] bool highlightEnabled() const;
    [[nodiscard]] bool highlightCaseSensitive() const;
    [[nodiscard]] bool highlightRegex() const;
    [[nodiscard]] bool hexSend() const;
    [[nodiscard]] int lineEnding() const;
    [[nodiscard]] bool loopSend() const;
    [[nodiscard]] int loopIntervalMs() const;
    [[nodiscard]] bool dtrEnabled() const;
    [[nodiscard]] bool rtsEnabled() const;
    [[nodiscard]] bool autoOpen() const;
    [[nodiscard]] bool rememberPort() const;
    [[nodiscard]] QString receiveEncoding() const;
    [[nodiscard]] QString sendEncoding() const;
    [[nodiscard]] int invalidBytePolicy() const;
    [[nodiscard]] bool enterToSend() const;
    [[nodiscard]] bool appendNewline() const;
    [[nodiscard]] bool multilineInput() const;

    void setPortName(const QString& portName);
    void setBaudRate(int baudRate);
    void setDataBits(int dataBits);
    void setParity(int parity);
    void setStopBits(int stopBits);
    void setFlowControl(int flowControl);
    void setAutoScroll(bool autoScroll);
    void setHexDisplay(bool hexDisplay);
    void setTimestampEnabled(bool timestampEnabled);
    void setTimestampScope(int timestampScope);
    void setTimestampFormat(const QString& timestampFormat);
    void setThemeMode(int themeMode);
    void setAccentColor(const QString& accentColor);
    void setUiDensity(int uiDensity);
    void setTerminalFont(const QString& terminalFont);
    void setTerminalFontSize(int terminalFontSize);
    void setHighlightEnabled(bool highlightEnabled);
    void setHighlightCaseSensitive(bool highlightCaseSensitive);
    void setHighlightRegex(bool highlightRegex);
    void setHexSend(bool hexSend);
    void setLineEnding(int lineEnding);
    void setLoopSend(bool loopSend);
    void setLoopIntervalMs(int loopIntervalMs);
    void setDtrEnabled(bool dtrEnabled);
    void setRtsEnabled(bool rtsEnabled);
    void setAutoOpen(bool autoOpen);
    void setRememberPort(bool rememberPort);
    void setReceiveEncoding(const QString& receiveEncoding);
    void setSendEncoding(const QString& sendEncoding);
    void setInvalidBytePolicy(int invalidBytePolicy);
    void setEnterToSend(bool enterToSend);
    void setAppendNewline(bool appendNewline);
    void setMultilineInput(bool multilineInput);

public slots:
    void restoreDefaults();

signals:
    void serialSettingsChanged();
    void autoScrollChanged();
    void hexDisplayChanged();
    void timestampEnabledChanged();
    void timestampScopeChanged();
    void timestampFormatChanged();
    void themeModeChanged();
    void accentColorChanged();
    void uiDensityChanged();
    void terminalFontChanged();
    void displaySettingsChanged();
    void highlightSettingsChanged();
    void transmitSettingsChanged();
    void serialLineSettingsChanged();
    void serialStartupSettingsChanged();
    void encodingSettingsChanged();
    void inputBehaviorChanged();

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
    int m_timestampScope;
    QString m_timestampFormat;
    int m_themeMode;
    QString m_accentColor;
    int m_uiDensity;
    QString m_terminalFont;
    int m_terminalFontSize;
    bool m_highlightEnabled;
    bool m_highlightCaseSensitive;
    bool m_highlightRegex;
    bool m_hexSend;
    int m_lineEnding;
    bool m_loopSend;
    int m_loopIntervalMs;
    bool m_dtrEnabled;
    bool m_rtsEnabled;
    bool m_autoOpen;
    bool m_rememberPort;
    QString m_receiveEncoding;
    QString m_sendEncoding;
    int m_invalidBytePolicy;
    bool m_enterToSend;
    bool m_appendNewline;
    bool m_multilineInput;
};

#endif
