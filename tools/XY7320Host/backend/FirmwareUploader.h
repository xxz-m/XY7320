#ifndef FIRMWAREUPLOADER_H
#define FIRMWAREUPLOADER_H

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QUrl>
#include <QVariant>

class FirmwareUploader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileInfoChanged)
    Q_PROPERTY(QString fileSizeText READ fileSizeText NOTIFY fileInfoChanged)
    Q_PROPERTY(QString crcHex READ crcHex NOTIFY fileInfoChanged)
    Q_PROPERTY(QString headerHex READ headerHex NOTIFY headerHexChanged)
    Q_PROPERTY(int packetSize READ packetSize WRITE setPacketSize NOTIFY packetSizeChanged)
    Q_PROPERTY(int headerDelayMs READ headerDelayMs WRITE setHeaderDelayMs NOTIFY headerDelayMsChanged)
    Q_PROPERTY(int packetDelayMs READ packetDelayMs WRITE setPacketDelayMs NOTIFY packetDelayMsChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool serialOpen READ serialOpen NOTIFY serialOpenChanged)
    Q_PROPERTY(bool manualHeaderSent READ manualHeaderSent NOTIFY manualHeaderSentChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)

public:
    explicit FirmwareUploader(QObject *parent = nullptr);

    QVariantList ports() const;
    QString portName() const;
    int baudRate() const;
    QString filePath() const;
    QString fileName() const;
    QString fileSizeText() const;
    QString crcHex() const;
    QString headerHex() const;
    int packetSize() const;
    int headerDelayMs() const;
    int packetDelayMs() const;
    double progress() const;
    bool busy() const;
    bool serialOpen() const;
    bool manualHeaderSent() const;
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
    Q_INVOKABLE void openPort();
    Q_INVOKABLE void closePort();
    Q_INVOKABLE void start();
    Q_INVOKABLE void sendHeaderManual();
    Q_INVOKABLE void sendFirmwareManual();
    Q_INVOKABLE void copyHeaderToClipboard();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void clearLog();

signals:
    void portsChanged();
    void portNameChanged();
    void baudRateChanged();
    void filePathChanged();
    void fileInfoChanged();
    void headerHexChanged();
    void packetSizeChanged();
    void headerDelayMsChanged();
    void packetDelayMsChanged();
    void progressChanged();
    void busyChanged();
    void serialOpenChanged();
    void manualHeaderSentChanged();
    void statusChanged();
    void logTextChanged();

private slots:
    void sendNextPacket();
    void checkSelectedFile();

private:
    static constexpr quint32 SimpleAppMagic = 0x41505055U;
    static constexpr int MaxPacketSize = 1024;

    QByteArray makeHeader() const;
    bool prepareFirmware();
    void updateFileInfo();
    void updateHeaderHex();
    void updateFileSnapshot();
    void resetPreparedFirmware();
    quint32 crc32(const QByteArray &data) const;
    void appendLe32(QByteArray *data, quint32 value) const;
    void setBusy(bool busy);
    void setManualHeaderSent(bool manualHeaderSent);
    void updateSerialOpenState();
    void setStatus(const QString &status);
    void appendLog(const QString &line);
    bool openSerial();
    bool writeBytes(const QByteArray &data);
    void finish(bool ok, const QString &message);
    void closeSession();

    QVariantList m_ports;
    QString m_portName;
    int m_baudRate = 115200;
    QString m_filePath;
    QString m_fileName = QStringLiteral("-");
    QString m_fileSizeText = QStringLiteral("-");
    QString m_crcHex = QStringLiteral("-");
    QString m_headerHex;
    int m_packetSize = 1024;
    int m_headerDelayMs = 3000;
    int m_packetDelayMs = 10;
    double m_progress = 0.0;
    bool m_busy = false;
    bool m_serialOpen = false;
    bool m_manualHeaderSent = false;
    QString m_status = QStringLiteral("就绪");
    QString m_logText;

    QSerialPort m_serial;
    QTimer m_sendTimer;
    QTimer m_portRefreshTimer;
    QTimer m_fileRefreshTimer;
    QByteArray m_firmware;
    qsizetype m_offset = 0;
    quint32 m_crc = 0;
    QDateTime m_fileLastModified;
    qint64 m_fileLastSize = -1;
};

#endif
