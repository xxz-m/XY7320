#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QQueue>
#include <QSerialPort>
#include <QTimer>
#include <QVariant>

class SerialPortManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY isOpenChanged)

public:
    enum class WriteTag {
        DebugTx,
        UpgradeHandshake,
        UpgradeHeader,
        UpgradePacket,
        UpgradeFinish,
        Unknown,
    };
    Q_ENUM(WriteTag)

    explicit SerialPortManager(QObject *parent = nullptr);

    QVariantList ports() const;
    QString portName() const;
    int baudRate() const;
    bool isOpen() const;

    void setPortName(const QString &portName);
    void setBaudRate(int baudRate);

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE bool open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void clearRxBuffer();

    bool write(const QByteArray &data, WriteTag tag, int timeoutMs = 3000);

signals:
    void portsChanged();
    void portNameChanged();
    void baudRateChanged();
    void isOpenChanged();
    void dataReceived(const QByteArray &data);
    void writeFinished(qint64 totalBytes, SerialPortManager::WriteTag tag);
    void writeTimeout(SerialPortManager::WriteTag tag);
    void serialErrorOccurred(QSerialPort::SerialPortError error, const QString &message);

private slots:
    void handleReadyRead();
    void handleBytesWritten(qint64 bytes);
    void handleError(QSerialPort::SerialPortError error);
    void handleWriteTimeout();

private:
    struct WriteRequest {
        QByteArray payload;
        qint64 bytesQueued = 0;
        qint64 bytesCommitted = 0;
        WriteTag tag = WriteTag::Unknown;
        int timeoutMs = 3000;
    };

    void updateOpenState();
    void clearPendingWrites();
    static QString formatPortText(const QString &portName, const QString &description, const QString &manufacturer);

    QVariantList m_ports;
    QString m_portName;
    int m_baudRate = 115200;
    bool m_isOpen = false;

    QSerialPort m_serial;
    QTimer m_portRefreshTimer;
    QTimer m_writeTimeoutTimer;
    QQueue<WriteRequest> m_pendingWrites;
};

#endif // SERIALPORTMANAGER_H
