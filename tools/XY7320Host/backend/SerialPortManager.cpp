#include "SerialPortManager.h"

#include <QSerialPortInfo>
#include <QSet>
#include <QSettings>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead, this, &SerialPortManager::handleReadyRead);
    connect(&m_serial, &QSerialPort::bytesWritten, this, &SerialPortManager::handleBytesWritten);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &SerialPortManager::handleError);

    m_portRefreshTimer.setInterval(1500);
    connect(&m_portRefreshTimer, &QTimer::timeout, this, &SerialPortManager::refreshPorts);
    m_portRefreshTimer.start();

    m_writeTimeoutTimer.setSingleShot(true);
    connect(&m_writeTimeoutTimer, &QTimer::timeout, this, &SerialPortManager::handleWriteTimeout);

    refreshPorts();
}

QVariantList SerialPortManager::ports() const { return m_ports; }
QString SerialPortManager::portName() const { return m_portName; }
int SerialPortManager::baudRate() const { return m_baudRate; }
bool SerialPortManager::isOpen() const { return m_isOpen; }

void SerialPortManager::setPortName(const QString &portName)
{
    if (m_portName == portName) {
        return;
    }

    if (m_serial.isOpen()) {
        close();
    }

    m_portName = portName;
    emit portNameChanged();
}

void SerialPortManager::setBaudRate(int baudRate)
{
    if (m_baudRate == baudRate) {
        return;
    }

    if (m_serial.isOpen()) {
        close();
    }

    m_baudRate = baudRate;
    emit baudRateChanged();
}

QString SerialPortManager::formatPortText(const QString &portName, const QString &description, const QString &manufacturer)
{
    QString text = portName;
    if (!description.isEmpty()) {
        text += QStringLiteral(" - %1").arg(description);
    }
    if (!manufacturer.isEmpty()) {
        text += QStringLiteral(" %1").arg(manufacturer);
    }
    return text;
}

void SerialPortManager::refreshPorts()
{
    QVariantList ports;
    QSet<QString> seenPorts;

    auto addPort = [&ports, &seenPorts](const QString &portName, const QString &description, const QString &manufacturer) {
        if (portName.isEmpty() || seenPorts.contains(portName)) {
            return;
        }

        QVariantMap item;
        item.insert(QStringLiteral("text"), formatPortText(portName, description, manufacturer));
        item.insert(QStringLiteral("portName"), portName);
        ports.append(item);
        seenPorts.insert(portName);
    };

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        addPort(info.portName(), info.description(), info.manufacturer());
    }

#ifdef Q_OS_WIN
    QSettings serialComm(QStringLiteral("HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                         QSettings::NativeFormat);
    for (const QString &key : serialComm.allKeys()) {
        const QString portName = serialComm.value(key).toString();
        const QSerialPortInfo info(portName);
        addPort(portName, info.description(), info.manufacturer());
    }
#endif

    if (m_ports != ports) {
        m_ports = ports;
        emit portsChanged();
    }

    bool currentExists = false;
    for (const QVariant &port : std::as_const(ports)) {
        if (port.toMap().value(QStringLiteral("portName")).toString() == m_portName) {
            currentExists = true;
            break;
        }
    }

    if ((m_portName.isEmpty() || !currentExists) && !ports.isEmpty()) {
        setPortName(ports.first().toMap().value(QStringLiteral("portName")).toString());
    } else if (!m_portName.isEmpty() && ports.isEmpty()) {
        setPortName(QString());
    }
}

bool SerialPortManager::open()
{
    if (m_serial.isOpen()) {
        return true;
    }

    if (m_portName.isEmpty()) {
        emit serialErrorOccurred(QSerialPort::OpenError, QStringLiteral("No serial port selected."));
        return false;
    }

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(m_baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        emit serialErrorOccurred(m_serial.error(), m_serial.errorString());
        updateOpenState();
        return false;
    }

    updateOpenState();
    return true;
}

void SerialPortManager::close()
{
    m_writeTimeoutTimer.stop();
    clearPendingWrites();
    if (m_serial.isOpen()) {
        m_serial.close();
    }
    updateOpenState();
}

void SerialPortManager::clearRxBuffer()
{
    if (m_serial.isOpen()) {
        m_serial.clear(QSerialPort::Input);
    }
}

bool SerialPortManager::write(const QByteArray &data, WriteTag tag, int timeoutMs)
{
    if (!m_serial.isOpen()) {
        emit serialErrorOccurred(QSerialPort::WriteError, QStringLiteral("Serial port is not open."));
        return false;
    }

    if (data.isEmpty()) {
        emit serialErrorOccurred(QSerialPort::WriteError, QStringLiteral("Write data is empty."));
        return false;
    }

    const qint64 written = m_serial.write(data);
    if (written < 0) {
        emit serialErrorOccurred(m_serial.error(), m_serial.errorString());
        return false;
    }

    WriteRequest request;
    request.payload = data;
    request.bytesQueued = data.size();
    request.bytesCommitted = 0;
    request.tag = tag;
    request.timeoutMs = timeoutMs;
    m_pendingWrites.enqueue(request);

    if (!m_writeTimeoutTimer.isActive()) {
        m_writeTimeoutTimer.start(timeoutMs);
    }

    return true;
}

void SerialPortManager::handleReadyRead()
{
    const QByteArray data = m_serial.readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialPortManager::handleBytesWritten(qint64 bytes)
{
    qint64 remaining = bytes;

    while (remaining > 0 && !m_pendingWrites.isEmpty()) {
        WriteRequest request = m_pendingWrites.dequeue();
        const qint64 chunk = qMin(remaining, request.bytesQueued - request.bytesCommitted);
        request.bytesCommitted += chunk;
        remaining -= chunk;

        if (request.bytesCommitted >= request.bytesQueued) {
            emit writeFinished(request.bytesQueued, request.tag);
        } else {
            m_pendingWrites.prepend(request);
            break;
        }
    }

    if (m_pendingWrites.isEmpty()) {
        m_writeTimeoutTimer.stop();
    } else {
        m_writeTimeoutTimer.start(m_pendingWrites.head().timeoutMs);
    }
}

void SerialPortManager::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    const QString message = m_serial.errorString();

    if (error == QSerialPort::ResourceError) {
        m_writeTimeoutTimer.stop();
        clearPendingWrites();
        if (m_serial.isOpen()) {
            m_serial.close();
        }
        updateOpenState();
    }

    emit serialErrorOccurred(error, message);
}

void SerialPortManager::handleWriteTimeout()
{
    if (m_pendingWrites.isEmpty()) {
        return;
    }

    const WriteTag tag = m_pendingWrites.head().tag;
    clearPendingWrites();
    emit writeTimeout(tag);
}

void SerialPortManager::updateOpenState()
{
    const bool open = m_serial.isOpen();
    if (m_isOpen == open) {
        return;
    }

    m_isOpen = open;
    emit isOpenChanged();
}

void SerialPortManager::clearPendingWrites()
{
    m_pendingWrites.clear();
}
