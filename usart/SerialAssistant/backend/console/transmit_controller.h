/**
 * @file transmit_controller.h
 * @brief Transmit-side console backend controller.
 */

#ifndef SERIAL_ASSISTANT_TRANSMIT_CONTROLLER_H
#define SERIAL_ASSISTANT_TRANSMIT_CONTROLLER_H

#include <QObject>
#include <QTimer>

class ConsoleRecordModel;
class SerialPortController;
class SettingsManager;

/**
 * Encodes user input into serial bytes and maintains transmit state/statistics.
 */
class TransmitController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hexSend READ hexSend WRITE setHexSend NOTIFY hexSendChanged)
    Q_PROPERTY(bool loopSend READ loopSend WRITE setLoopSend NOTIFY loopSendChanged)
    Q_PROPERTY(int loopIntervalMs READ loopIntervalMs WRITE setLoopIntervalMs NOTIFY loopIntervalMsChanged)
    Q_PROPERTY(int lineEnding READ lineEnding WRITE setLineEnding NOTIFY lineEndingChanged)
    Q_PROPERTY(qulonglong txBytes READ txBytes NOTIFY statisticsChanged)
    Q_PROPERTY(qulonglong txFrames READ txFrames NOTIFY statisticsChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)

public:
    enum LineEnding {
        None = 0,
        LF = 1,
        CR = 2,
        CRLF = 3
    };
    Q_ENUM(LineEnding)

    TransmitController(SerialPortController* serialController,
                       ConsoleRecordModel* recordModel,
                       SettingsManager* settingsManager,
                       QObject* parent = nullptr);

    [[nodiscard]] bool hexSend() const;
    [[nodiscard]] bool loopSend() const;
    [[nodiscard]] int loopIntervalMs() const;
    [[nodiscard]] int lineEnding() const;
    [[nodiscard]] qulonglong txBytes() const;
    [[nodiscard]] qulonglong txFrames() const;
    [[nodiscard]] QString errorText() const;

    void setHexSend(bool hexSend);
    void setLoopSend(bool loopSend);
    void setLoopIntervalMs(int loopIntervalMs);
    void setLineEnding(int lineEnding);

    Q_INVOKABLE bool send(const QString& input);
    Q_INVOKABLE void startLoopSend(const QString& input);
    Q_INVOKABLE void stopLoopSend();

signals:
    void hexSendChanged();
    void loopSendChanged();
    void loopIntervalMsChanged();
    void lineEndingChanged();
    void statisticsChanged();
    void errorTextChanged();

private slots:
    void sendLoopPayload();
    void handleSerialError(const QString& message);

private:
    [[nodiscard]] QByteArray buildPayload(const QString& input, QString* errorText) const;
    [[nodiscard]] QByteArray lineEndingBytes() const;
    void setErrorText(const QString& errorText);
    void recordTransmit(const QByteArray& payload);

    SerialPortController* m_serialController;
    ConsoleRecordModel* m_recordModel;
    SettingsManager* m_settingsManager;
    QTimer m_loopTimer;
    QString m_loopInput;
    bool m_hexSend;
    bool m_loopSend;
    int m_loopIntervalMs;
    int m_lineEnding;
    qulonglong m_txBytes = 0;
    qulonglong m_txFrames = 0;
    QString m_errorText;
};

#endif
