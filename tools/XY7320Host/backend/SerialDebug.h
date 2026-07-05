#ifndef SERIALDEBUG_H
#define SERIALDEBUG_H

#include <QObject>
#include <QVariant>

class SerialPortManager;

class SerialDebug : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY isOpenChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)
    Q_PROPERTY(bool showHex READ showHex WRITE setShowHex NOTIFY showHexChanged)
    Q_PROPERTY(qint64 rxBytes READ rxBytes NOTIFY rxBytesChanged)
    Q_PROPERTY(qint64 txBytes READ txBytes NOTIFY txBytesChanged)

public:
    explicit SerialDebug(SerialPortManager *serialPortManager, QObject *parent = nullptr);

    QVariantList ports() const;
    QString portName() const;
    int baudRate() const;
    bool isOpen() const;
    QString logText() const;
    bool autoScroll() const;
    bool showHex() const;
    qint64 rxBytes() const;
    qint64 txBytes() const;

    void setPortName(const QString &portName);
    void setBaudRate(int baudRate);
    void setAutoScroll(bool autoScroll);
    void setShowHex(bool showHex);

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void send(const QString &data);
    Q_INVOKABLE void sendHex(const QString &hex);
    Q_INVOKABLE void clear();

signals:
    void portsChanged();
    void portNameChanged();
    void baudRateChanged();
    void isOpenChanged();
    void logTextChanged();
    void autoScrollChanged();
    void showHexChanged();
    void rxBytesChanged();
    void txBytesChanged();

private slots:
    void handleReadyRead(const QByteArray &data);
    void handleWriteFinished(qint64 totalBytes, int tag);
    void handleError(int error, const QString &message);

private:
    void appendLog(const QString &text);
    void updateOpenState();
    QByteArray parseHexString(const QString &data) const;

    SerialPortManager *m_serialPortManager = nullptr;

    QVariantList m_ports;
    QString m_portName;
    int m_baudRate = 115200;
    QString m_logText;
    bool m_autoScroll = true;
    bool m_showHex = false;
    qint64 m_rxBytes = 0;
    qint64 m_txBytes = 0;
};

#endif // SERIALDEBUG_H
