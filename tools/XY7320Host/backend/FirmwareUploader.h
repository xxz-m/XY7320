#ifndef FIRMWAREUPLOADER_H
#define FIRMWAREUPLOADER_H

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVariant>

class SerialPortManager;

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
    Q_PROPERTY(QString versionText READ versionText WRITE setVersionText NOTIFY versionInfoChanged)
    Q_PROPERTY(int versionFlag READ versionFlag WRITE setVersionFlag NOTIFY versionInfoChanged)
    Q_PROPERTY(QString versionFrameHex READ versionFrameHex NOTIFY versionInfoChanged)
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
    explicit FirmwareUploader(SerialPortManager *serialPortManager, QObject *parent = nullptr);

    QVariantList ports() const;
    QString portName() const;
    int baudRate() const;
    QString filePath() const;
    QString fileName() const;
    QString fileSizeText() const;
    QString crcHex() const;
    QString versionText() const;
    int versionFlag() const;
    QString versionFrameHex() const;
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
    void setVersionText(const QString &versionText);
    void setVersionFlag(int versionFlag);
    void setPacketSize(int packetSize);
    void setHeaderDelayMs(int headerDelayMs);
    void setPacketDelayMs(int packetDelayMs);

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void setFileUrl(const QUrl &url);
    Q_INVOKABLE void openPort();
    Q_INVOKABLE void closePort();
    Q_INVOKABLE void start();
    Q_INVOKABLE void sendVersionFrameManual();
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
    void versionInfoChanged();
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
    void handleSerialReadyRead(const QByteArray &data);
    void handleHandshakeTimeout();
    void handleWriteFinished(qint64 totalBytes, int tag);
    void handleSerialError(int error, const QString &message);
    void handleWriteTimeout(int tag);

private:
    static constexpr quint32 SimpleAppMagic = 0x41505055U;
    static constexpr int MaxPacketSize = 1024;
    static constexpr int VersionTextLength = 12;
    static constexpr int HandshakeTimeoutMs = 8000;
    static constexpr quint8 ProtocolHead1 = 0x10;
    static constexpr quint8 ProtocolHead2 = 0x02;
    static constexpr quint8 ProtocolEnd1 = 0x10;
    static constexpr quint8 ProtocolEnd2 = 0x03;
    static constexpr quint8 ProtocolModelWrite = 0x02;
    static constexpr quint8 ProtocolCmdUpgradeHandshake = 0xF0;
    static constexpr quint8 ProtocolPortPc = 0x01;
    static constexpr quint8 ProtocolPortDevice = 0x22;

    enum class AutoStage {
        Idle,
        WaitAppVersionAck,
        WaitBootReady,
        WaitHeaderAck,
        SendingFirmware,
        WaitFinishAck
    };

    QByteArray makeHeader() const;
    QByteArray makeVersionFrame() const;
    bool validateVersionFrameInput();
    quint16 protocolCrc16(const QByteArray &data) const;
    QByteArray makeProtocolUpgradeHandshake() const;
    bool tryConsumeProtocolUpgradeAck();
    bool prepareFirmware();
    void updateFileInfo();
    void updateVersionInfo();
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
    bool writeBytes(const QByteArray &data, int timeoutMs, int tag);
    void finish(bool ok, const QString &message);
    void closeSession();
    void startHandshakeTimer();
    void stopHandshakeTimer();
    void consumeRxBuffer();
    void handleHandshakeCode(const QByteArray &code);
    QString stageName(AutoStage stage) const;

    QVariantList m_ports;
    SerialPortManager *m_serialPortManager = nullptr;
    QString m_portName;
    int m_baudRate = 115200;
    QString m_filePath;
    QString m_fileName = QStringLiteral("-");
    QString m_fileSizeText = QStringLiteral("-");
    QString m_crcHex = QStringLiteral("-");
    QString m_versionText;
    int m_versionFlag = 0;
    QString m_versionFrameHex;
    QString m_headerHex;
    int m_packetSize = 1024;
    int m_headerDelayMs = 3000;
    int m_packetDelayMs = 10;
    double m_progress = 0.0;
    bool m_busy = false;
    bool m_serialOpen = false;
    bool m_manualHeaderSent = false;
    QString m_status = QStringLiteral("Ready");
    QString m_logText;

    QTimer m_sendTimer;
    QTimer m_handshakeTimer;
    QTimer m_fileRefreshTimer;
    AutoStage m_autoStage = AutoStage::Idle;
    QByteArray m_rxBuffer;
    QByteArray m_firmware;
    qsizetype m_offset = 0;
    quint32 m_crc = 0;
    QDateTime m_fileLastModified;
    qint64 m_fileLastSize = -1;
};

#endif
