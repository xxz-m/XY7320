#include "SerialDebug.h"

#include "SerialPortManager.h"

#include <QDateTime>
#include <QRegularExpression>

SerialDebug::SerialDebug(SerialPortManager *serialPortManager, QObject *parent)
    : QObject(parent)
    , m_serialPortManager(serialPortManager)
{
    Q_ASSERT(m_serialPortManager != nullptr);

    connect(m_serialPortManager, &SerialPortManager::portsChanged, this, &SerialDebug::refreshPorts);
    connect(m_serialPortManager, &SerialPortManager::portNameChanged, this, [this]() {
        m_portName = m_serialPortManager->portName();
        emit portNameChanged();
    });
    connect(m_serialPortManager, &SerialPortManager::baudRateChanged, this, [this]() {
        m_baudRate = m_serialPortManager->baudRate();
        emit baudRateChanged();
    });
    connect(m_serialPortManager, &SerialPortManager::isOpenChanged, this, &SerialDebug::updateOpenState);
    connect(m_serialPortManager, &SerialPortManager::dataReceived, this, &SerialDebug::handleReadyRead);
    connect(m_serialPortManager, &SerialPortManager::writeFinished, this, [this](qint64 totalBytes, SerialPortManager::WriteTag tag) {
        handleWriteFinished(totalBytes, static_cast<int>(tag));
    });
    connect(m_serialPortManager, &SerialPortManager::serialErrorOccurred, this, [this](QSerialPort::SerialPortError error, const QString &message) {
        handleError(static_cast<int>(error), message);
    });

    refreshPorts();
    m_portName = m_serialPortManager->portName();
    m_baudRate = m_serialPortManager->baudRate();
}

QVariantList SerialDebug::ports() const { return m_ports; }
QString SerialDebug::portName() const { return m_portName; }
int SerialDebug::baudRate() const { return m_baudRate; }
bool SerialDebug::isOpen() const { return m_serialPortManager != nullptr && m_serialPortManager->isOpen(); }
QString SerialDebug::logText() const { return m_logText; }
bool SerialDebug::autoScroll() const { return m_autoScroll; }
bool SerialDebug::showHex() const { return m_showHex; }
qint64 SerialDebug::rxBytes() const { return m_rxBytes; }
qint64 SerialDebug::txBytes() const { return m_txBytes; }

void SerialDebug::setPortName(const QString &portName)
{
    if (m_serialPortManager == nullptr) return;
    m_serialPortManager->setPortName(portName);
}

void SerialDebug::setBaudRate(int baudRate)
{
    if (m_serialPortManager == nullptr) return;
    m_serialPortManager->setBaudRate(baudRate);
}

void SerialDebug::setAutoScroll(bool autoScroll)
{
    if (m_autoScroll == autoScroll) return;
    m_autoScroll = autoScroll;
    emit autoScrollChanged();
}

void SerialDebug::setShowHex(bool showHex)
{
    if (m_showHex == showHex) return;
    m_showHex = showHex;
    emit showHexChanged();
}

void SerialDebug::refreshPorts()
{
    if (m_serialPortManager == nullptr) {
        return;
    }

    const QVariantList newPorts = m_serialPortManager->ports();
    if (m_ports != newPorts) {
        m_ports = newPorts;
        emit portsChanged();
    }

    const QString nextPortName = m_serialPortManager->portName();
    if (m_portName != nextPortName) {
        m_portName = nextPortName;
        emit portNameChanged();
    }
}

void SerialDebug::open()
{
    if (m_serialPortManager == nullptr || m_serialPortManager->isOpen()) return;

    if (m_serialPortManager->open()) {
        appendLog(QStringLiteral("[已连接] ") + m_serialPortManager->portName() + QStringLiteral(" @ ") + QString::number(m_serialPortManager->baudRate()));
    }
}

void SerialDebug::close()
{
    if (m_serialPortManager == nullptr || !m_serialPortManager->isOpen()) return;
    m_serialPortManager->close();
    appendLog(QStringLiteral("[已断开]"));
}

void SerialDebug::send(const QString &data)
{
    if (!isOpen()) {
        appendLog(QStringLiteral("[发送失败] 串口未打开"));
        return;
    }

    if (m_showHex) {
        sendHex(data);
        return;
    }

    const QByteArray bytes = data.toUtf8();

    if (bytes.isEmpty()) {
        appendLog(QStringLiteral("[发送失败] 数据为空"));
        return;
    }

    if (!m_serialPortManager->write(bytes, SerialPortManager::WriteTag::DebugTx)) {
        appendLog(QStringLiteral("[发送失败] 提交发送失败"));
        return;
    }

    appendLog(QStringLiteral("[TX-QUEUED] ") + data);
}

void SerialDebug::sendHex(const QString &data)
{
    if (!isOpen()) {
        appendLog(QStringLiteral("[发送失败] 串口未打开"));
        return;
    }

    const QByteArray bytes = parseHexString(data);

    if (bytes.isEmpty()) {
        appendLog(QStringLiteral("[发送失败] 数据为空"));
        return;
    }

    if (!m_serialPortManager->write(bytes, SerialPortManager::WriteTag::DebugTx)) {
        appendLog(QStringLiteral("[发送失败] 提交发送失败"));
        return;
    }

    appendLog(QStringLiteral("[TX-QUEUED] ") + bytes.toHex(' ').toUpper());
}

void SerialDebug::clear()
{
    m_logText.clear();
    m_rxBytes = 0;
    m_txBytes = 0;
    emit logTextChanged();
    emit rxBytesChanged();
    emit txBytesChanged();
}

void SerialDebug::handleReadyRead(const QByteArray &data)
{
    if (data.isEmpty()) return;

    m_rxBytes += data.size();
    emit rxBytesChanged();

    QString text;
    if (m_showHex) {
        text = data.toHex(' ').toUpper();
    } else {
        text = QString::fromUtf8(data);
    }

    appendLog(QStringLiteral("[RX] ") + text);
}

void SerialDebug::handleWriteFinished(qint64 totalBytes, int tag)
{
    if (static_cast<SerialPortManager::WriteTag>(tag) != SerialPortManager::WriteTag::DebugTx) {
        return;
    }

    m_txBytes += totalBytes;
    emit txBytesChanged();
}

void SerialDebug::handleError(int error, const QString &message)
{
    if (error == static_cast<int>(QSerialPort::NoError)) return;
    if (error == static_cast<int>(QSerialPort::ResourceError)) {
        appendLog(QStringLiteral("[错误] 串口被断开"));
        return;
    }

    appendLog(QStringLiteral("[错误] ") + message);
}

void SerialDebug::appendLog(const QString &text)
{
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"));
    QString line = QStringLiteral("[%1] %2\n").arg(timestamp, text);

    // 限制日志长度，防止内存溢出
    const int maxLogLength = 100000;
    if (m_logText.length() + line.length() > maxLogLength) {
        m_logText = m_logText.right(maxLogLength / 2);
    }

    m_logText += line;
    emit logTextChanged();
}

void SerialDebug::updateOpenState()
{
    emit isOpenChanged();
}

QByteArray SerialDebug::parseHexString(const QString &data) const
{
    QByteArray bytes;
    const QStringList parts = data.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        bool ok = false;
        const quint8 byte = part.toUInt(&ok, 16);
        if (!ok) {
            return QByteArray();
        }
        bytes.append(static_cast<char>(byte));
    }
    return bytes;
}
