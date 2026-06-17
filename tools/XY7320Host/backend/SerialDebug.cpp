#include "SerialDebug.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSerialPortInfo>

SerialDebug::SerialDebug(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead, this, &SerialDebug::handleReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &SerialDebug::handleError);

    m_portRefreshTimer.setInterval(1500);
    connect(&m_portRefreshTimer, &QTimer::timeout, this, &SerialDebug::refreshPorts);
    m_portRefreshTimer.start();

    refreshPorts();
}

QVariantList SerialDebug::ports() const { return m_ports; }
QString SerialDebug::portName() const { return m_portName; }
int SerialDebug::baudRate() const { return m_baudRate; }
bool SerialDebug::isOpen() const { return m_serial.isOpen(); }
QString SerialDebug::logText() const { return m_logText; }
bool SerialDebug::autoScroll() const { return m_autoScroll; }
bool SerialDebug::showHex() const { return m_showHex; }
qint64 SerialDebug::rxBytes() const { return m_rxBytes; }
qint64 SerialDebug::txBytes() const { return m_txBytes; }

void SerialDebug::setPortName(const QString &portName)
{
    if (m_portName == portName) return;
    m_portName = portName;
    emit portNameChanged();
}

void SerialDebug::setBaudRate(int baudRate)
{
    if (m_baudRate == baudRate) return;
    m_baudRate = baudRate;
    emit baudRateChanged();
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
    QVariantList newPorts;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const auto &info : infos) {
        QVariantMap item;
        item[QStringLiteral("text")] = info.portName() + QStringLiteral(" - ") + info.description();
        item[QStringLiteral("portName")] = info.portName();
        newPorts.append(item);
    }

    if (m_ports.size() != newPorts.size()) {
        m_ports = newPorts;
        emit portsChanged();
    }
}

void SerialDebug::open()
{
    if (m_serial.isOpen()) return;
    if (m_portName.isEmpty()) return;

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(m_baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial.open(QIODevice::ReadWrite)) {
        updateOpenState();
        appendLog(QStringLiteral("[已连接] ") + m_portName + QStringLiteral(" @ ") + QString::number(m_baudRate));
    } else {
        appendLog(QStringLiteral("[连接失败] ") + m_serial.errorString());
    }
}

void SerialDebug::close()
{
    if (!m_serial.isOpen()) return;
    m_serial.close();
    updateOpenState();
    appendLog(QStringLiteral("[已断开]"));
}

void SerialDebug::send(const QString &data)
{
    if (!m_serial.isOpen()) {
        appendLog(QStringLiteral("[发送失败] 串口未打开"));
        return;
    }

    QByteArray bytes;
    if (m_showHex) {
        // 解析 HEX 字符串
        QStringList parts = data.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        for (const auto &part : parts) {
            bool ok;
            quint8 byte = part.toUInt(&ok, 16);
            if (ok) bytes.append(static_cast<char>(byte));
        }
    } else {
        bytes = data.toUtf8();
    }

    if (bytes.isEmpty()) {
        appendLog(QStringLiteral("[发送失败] 数据为空"));
        return;
    }

    qint64 written = m_serial.write(bytes);
    if (written > 0) {
        m_txBytes += written;
        emit txBytesChanged();
        appendLog(QStringLiteral("[TX] ") + (m_showHex ? bytes.toHex(' ').toUpper() : data));
    } else {
        appendLog(QStringLiteral("[发送失败] ") + m_serial.errorString());
    }
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

void SerialDebug::handleReadyRead()
{
    QByteArray data = m_serial.readAll();
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

void SerialDebug::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;
    if (error == QSerialPort::ResourceError) {
        m_serial.close();
        updateOpenState();
        appendLog(QStringLiteral("[错误] 串口被断开"));
    }
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
