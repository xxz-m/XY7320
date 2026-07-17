#ifndef SERIALDEBUG_H
#define SERIALDEBUG_H

#include <QObject>
#include <QVariant>
#include <QByteArray>

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
    Q_PROPERTY(QString frameStatus READ frameStatus NOTIFY frameChanged)
    Q_PROPERTY(QString frameDirection READ frameDirection NOTIFY frameChanged)
    Q_PROPERTY(QString frameModel READ frameModel NOTIFY frameChanged)
    Q_PROPERTY(QString frameCommand READ frameCommand NOTIFY frameChanged)
    Q_PROPERTY(QString frameLength READ frameLength NOTIFY frameChanged)
    Q_PROPERTY(QString framePayloadHex READ framePayloadHex NOTIFY frameChanged)
    Q_PROPERTY(QString frameCrcStatus READ frameCrcStatus NOTIFY frameChanged)
    Q_PROPERTY(QString businessType READ businessType NOTIFY frameChanged)
    Q_PROPERTY(QVariantList businessFields READ businessFields NOTIFY frameChanged)

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
    QString frameStatus() const;
    QString frameDirection() const;
    QString frameModel() const;
    QString frameCommand() const;
    QString frameLength() const;
    QString framePayloadHex() const;
    QString frameCrcStatus() const;
    QString businessType() const;
    QVariantList businessFields() const;

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
    Q_INVOKABLE void switchMode(const QString &mode);

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
    void frameChanged();

private slots:
    void handleReadyRead(const QByteArray &data);
    void handleWriteFinished(qint64 totalBytes, int tag);
    void handleError(int error, const QString &message);

private:
    void appendLog(const QString &text);
    void updateOpenState();
    QByteArray parseHexString(const QString &data) const;
    void processProtocolStream();
    void publishFrame(const QByteArray &frame, const QByteArray &rawFrame);
    void setFrameError(const QString &status, const QString &detail);
    static quint16 crc16Xmodem(const QByteArray &data);
    static quint16 readUint16(const QByteArray &data, int offset);
    static quint32 readUint32(const QByteArray &data, int offset);
    static QString hexByte(quint8 value);
    static QVariantMap field(const QString &name, const QString &value);
    QVariantList decodeBusiness(quint8 command, const QByteArray &payload, QString *type) const;
    QByteArray encodeProtocolFrame(quint8 command, const QByteArray &payload) const;

    SerialPortManager *m_serialPortManager = nullptr;

    QVariantList m_ports;
    QString m_portName;
    int m_baudRate = 115200;
    QString m_logText;
    bool m_autoScroll = true;
    bool m_showHex = false;
    qint64 m_rxBytes = 0;
    qint64 m_txBytes = 0;

    QByteArray m_protocolBuffer;
    QString m_frameStatus = QStringLiteral("等待协议帧");
    QString m_frameDirection = QStringLiteral("-");
    QString m_frameModel = QStringLiteral("-");
    QString m_frameCommand = QStringLiteral("-");
    QString m_frameLength = QStringLiteral("0 B");
    QString m_framePayloadHex = QStringLiteral("-");
    QString m_frameCrcStatus = QStringLiteral("-");
    QString m_businessType = QStringLiteral("-");
    QVariantList m_businessFields;
};

#endif // SERIALDEBUG_H
