#ifndef FIRMWAREUPLOADER_H
#define FIRMWAREUPLOADER_H

#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QStringList>
#include <QTimer>
#include <QUrl>

class FirmwareUploader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString headerHex READ headerHex NOTIFY headerHexChanged)
    Q_PROPERTY(int packetSize READ packetSize WRITE setPacketSize NOTIFY packetSizeChanged)
    Q_PROPERTY(int headerDelayMs READ headerDelayMs WRITE setHeaderDelayMs NOTIFY headerDelayMsChanged)
    Q_PROPERTY(int packetDelayMs READ packetDelayMs WRITE setPacketDelayMs NOTIFY packetDelayMsChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)

public:
    explicit FirmwareUploader(QObject *parent = nullptr);

    QStringList ports() const;
    QString portName() const;
    int baudRate() const;
    QString filePath() const;
    QString headerHex() const;
    int packetSize() const;
    int headerDelayMs() const;
    int packetDelayMs() const;
    double progress() const;
    bool busy() const;
    QString status() const;
    QString logText() const;

    void setPortName(const QString &portName);
    void setBaudRate(int baudRate);
    void setFilePath(const QString &filePath);
    void setPacketSize(int packetSize);
    void setHeaderDelayMs(int headerDelayMs);
    void setPacketDelayMs(int packetDelayMs);

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void setFileUrl(const QUrl &url);
    Q_INVOKABLE void start();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void clearLog();

signals:
    void portsChanged();
    void portNameChanged();
    void baudRateChanged();
    void filePathChanged();
    void headerHexChanged();
    void packetSizeChanged();
    void headerDelayMsChanged();
    void packetDelayMsChanged();
    void progressChanged();
    void busyChanged();
    void statusChanged();
    void logTextChanged();

private slots:
    void sendNextPacket();

private:
    static constexpr quint32 SimpleAppMagic = 0x41505055U;
    static constexpr int MaxPacketSize = 1024;

    QByteArray makeHeader() const;
    void updateHeaderHex();
    quint32 crc32(const QByteArray &data) const;
    void appendLe32(QByteArray *data, quint32 value) const;
    void setBusy(bool busy);
    void setStatus(const QString &status);
    void appendLog(const QString &line);
    bool writeBytes(const QByteArray &data);
    void finish(bool ok, const QString &message);

    QStringList m_ports;
    QString m_portName;
    int m_baudRate = 115200;
    QString m_filePath;
    QString m_headerHex;
    int m_packetSize = 1024;
    int m_headerDelayMs = 3000;
    int m_packetDelayMs = 20;
    double m_progress = 0.0;
    bool m_busy = false;
    QString m_status = QStringLiteral("就绪");
    QString m_logText;

    QSerialPort m_serial;
    QTimer m_sendTimer;
    QByteArray m_firmware;
    qsizetype m_offset = 0;
    quint32 m_crc = 0;
};

#endif
