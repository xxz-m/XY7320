#include "SerialDebug.h"

#include "SerialPortManager.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QVariantMap>
#include <QtGlobal>

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
QString SerialDebug::frameStatus() const { return m_frameStatus; }
QString SerialDebug::frameDirection() const { return m_frameDirection; }
QString SerialDebug::frameModel() const { return m_frameModel; }
QString SerialDebug::frameCommand() const { return m_frameCommand; }
QString SerialDebug::frameLength() const { return m_frameLength; }
QString SerialDebug::framePayloadHex() const { return m_framePayloadHex; }
QString SerialDebug::frameCrcStatus() const { return m_frameCrcStatus; }
QString SerialDebug::businessType() const { return m_businessType; }
QVariantList SerialDebug::businessFields() const { return m_businessFields; }

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
    m_protocolBuffer.clear();
    m_frameStatus = QStringLiteral("等待协议帧");
    m_frameDirection = QStringLiteral("-");
    m_frameModel = QStringLiteral("-");
    m_frameCommand = QStringLiteral("-");
    m_frameLength = QStringLiteral("0 B");
    m_framePayloadHex = QStringLiteral("-");
    m_frameCrcStatus = QStringLiteral("-");
    m_businessType = QStringLiteral("-");
    m_businessFields.clear();
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

    m_protocolBuffer.append(data);
    processProtocolStream();
}

void SerialDebug::switchMode(const QString &mode)
{
    quint8 command = 0;
    QByteArray payload;
    if (mode.compare(QStringLiteral("idle"), Qt::CaseInsensitive) == 0) {
        command = 0x20;
    } else if (mode.compare(QStringLiteral("dmr"), Qt::CaseInsensitive) == 0) {
        command = 0x21;
        payload = QByteArray::fromHex("0000");
    } else if (mode.compare(QStringLiteral("gnss"), Qt::CaseInsensitive) == 0) {
        command = 0x22;
        payload = QByteArray::fromHex("00");
    } else if (mode.compare(QStringLiteral("gsm"), Qt::CaseInsensitive) == 0) {
        command = 0x28;
        payload = QByteArray::fromHex("0000");
    } else {
        appendLog(QStringLiteral("[模式切换失败] 未知模式：") + mode);
        return;
    }

    if (!isOpen()) {
        appendLog(QStringLiteral("[模式切换失败] 串口未打开"));
        return;
    }

    const QByteArray frame = encodeProtocolFrame(command, payload);
    const quint16 expectedInfoLength = static_cast<quint16>(8 + payload.size());
    const quint16 expectedFrameLength = static_cast<quint16>(expectedInfoLength + 6);
    appendLog(QStringLiteral("[模式切换] frame_len=%1 info_len=%2 payload_len=%3")
                  .arg(frame.size())
                  .arg(expectedInfoLength)
                  .arg(payload.size()));
    if (frame.size() != expectedFrameLength) {
        appendLog(QStringLiteral("[模式切换失败] frame length mismatch"));
        return;
    }
    if (!m_serialPortManager->write(frame, SerialPortManager::WriteTag::DebugTx)) {
        appendLog(QStringLiteral("[模式切换失败] 提交发送失败"));
        return;
    }

    appendLog(QStringLiteral("[模式切换] ") + mode + QStringLiteral(" TX: ") + frame.toHex(' ').toUpper());
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

void SerialDebug::processProtocolStream()
{
    constexpr int maxRawFrameLength = 300;
    while (!m_protocolBuffer.isEmpty()) {
        const int head = m_protocolBuffer.indexOf(QByteArray::fromHex("1002"));
        if (head < 0) {
            m_protocolBuffer.clear();
            setFrameError(QStringLiteral("等待协议帧"), QStringLiteral("未找到帧头"));
            return;
        }
        if (head > 0) {
            m_protocolBuffer.remove(0, head);
            setFrameError(QStringLiteral("已重同步"), QStringLiteral("丢弃帧头前的非协议字节"));
        }
        if (m_protocolBuffer.size() > maxRawFrameLength) {
            m_protocolBuffer.remove(0, 2);
            setFrameError(QStringLiteral("帧长度异常"), QStringLiteral("超过 300 字节"));
            continue;
        }

        int end = -1;
        for (int index = 2; index + 1 < m_protocolBuffer.size();) {
            const quint8 current = static_cast<quint8>(m_protocolBuffer.at(index));
            const quint8 next = static_cast<quint8>(m_protocolBuffer.at(index + 1));
            if (current == 0x10 && next == 0x10) {
                index += 2;
                continue;
            }
            if (current == 0x10 && next == 0x03) {
                end = index + 2;
                break;
            }
            ++index;
        }
        if (end < 0) {
            setFrameError(QStringLiteral("等待完整帧"), QStringLiteral("已收到 %1 字节").arg(m_protocolBuffer.size()));
            return;
        }

        const QByteArray rawFrame = m_protocolBuffer.left(end);
        m_protocolBuffer.remove(0, end);
        QByteArray frame;
        frame.append(rawFrame.left(2));
        for (int index = 2; index < rawFrame.size() - 2; ++index) {
            const quint8 value = static_cast<quint8>(rawFrame.at(index));
            if (value == 0x10 && index + 1 < rawFrame.size() - 2 &&
                static_cast<quint8>(rawFrame.at(index + 1)) == 0x10) {
                frame.append(char(0x10));
                ++index;
            } else {
                frame.append(char(value));
            }
        }
        frame.append(rawFrame.right(2));
        publishFrame(frame, rawFrame);
    }
}

void SerialDebug::publishFrame(const QByteArray &frame, const QByteArray &rawFrame)
{
    if (frame.size() < 10 || static_cast<quint8>(frame.at(0)) != 0x10 ||
        static_cast<quint8>(frame.at(1)) != 0x02 ||
        static_cast<quint8>(frame.at(frame.size() - 2)) != 0x10 ||
        static_cast<quint8>(frame.at(frame.size() - 1)) != 0x03) {
        setFrameError(QStringLiteral("帧格式错误"), rawFrame.toHex(' ').toUpper());
        return;
    }

    const int infoLength = (static_cast<quint8>(frame.at(2)) << 8) |
                           static_cast<quint8>(frame.at(3));
    if (infoLength < 6 || infoLength > 128 || infoLength + 8 != frame.size()) {
        setFrameError(QStringLiteral("帧长度异常"), QStringLiteral("info_len=%1, frame=%2").arg(infoLength).arg(frame.size()));
        return;
    }

    const quint16 expectedCrc = (static_cast<quint8>(frame.at(infoLength + 2)) << 8) |
                                static_cast<quint8>(frame.at(infoLength + 3));
    const quint16 actualCrc = crc16Xmodem(frame.mid(2, infoLength));
    m_frameCrcStatus = actualCrc == expectedCrc
        ? QStringLiteral("CRC 正确 0x%1").arg(expectedCrc, 4, 16, QLatin1Char('0')).toUpper()
        : QStringLiteral("CRC 错误 计算 0x%1 / 收到 0x%2")
              .arg(actualCrc, 4, 16, QLatin1Char('0')).arg(expectedCrc, 4, 16, QLatin1Char('0')).toUpper();
    m_frameStatus = actualCrc == expectedCrc ? QStringLiteral("解析成功") : QStringLiteral("CRC 校验失败");
    if (actualCrc != expectedCrc) {
        appendLog(QStringLiteral("[解析] ") + m_frameCrcStatus);
        emit frameChanged();
        return;
    }

    int offset = 4;
    if (offset + 2 > frame.size()) return;
    const quint8 originPort = static_cast<quint8>(frame.at(offset++));
    const quint8 originLength = static_cast<quint8>(frame.at(offset++));
    if (originLength > 6 || offset + originLength + 2 > infoLength + 4) {
        setFrameError(QStringLiteral("地址字段错误"), QStringLiteral("源地址长度=%1").arg(originLength));
        return;
    }
    offset += originLength;
    const quint8 goalPort = static_cast<quint8>(frame.at(offset++));
    const quint8 goalLength = static_cast<quint8>(frame.at(offset++));
    if (goalLength > 6 || offset + goalLength + 2 > infoLength + 4) {
        setFrameError(QStringLiteral("地址字段错误"), QStringLiteral("目的地址长度=%1").arg(goalLength));
        return;
    }
    offset += goalLength;
    const quint8 model = static_cast<quint8>(frame.at(offset++));
    const quint8 command = static_cast<quint8>(frame.at(offset++));
    const int payloadLength = infoLength - originLength - goalLength - 6;
    if (payloadLength < 0 || offset + payloadLength > frame.size() - 4) {
        setFrameError(QStringLiteral("数据长度错误"), QStringLiteral("payload=%1").arg(payloadLength));
        return;
    }

    const QByteArray payload = frame.mid(offset, payloadLength);
    const auto portText = [](quint8 port) {
        if (port == 0x01) return QStringLiteral("XY_PC(0x01)");
        if (port == 0x21) return QStringLiteral("XY_7000XMAIN(0x21)");
        if (port == 0x22) return QStringLiteral("XY_7320(0x22)");
        return QStringLiteral("0x%1").arg(port, 2, 16, QLatin1Char('0')).toUpper();
    };
    m_frameDirection = portText(originPort) + QStringLiteral(" -> ") + portText(goalPort);
    m_frameModel = QStringLiteral("0x%1 (%2)").arg(model, 2, 16, QLatin1Char('0')).arg(model == 0x01 ? QStringLiteral("读") : model == 0x02 ? QStringLiteral("写") : QStringLiteral("未知")).toUpper();
    m_frameCommand = QStringLiteral("0x%1").arg(command, 2, 16, QLatin1Char('0')).toUpper();
    m_frameLength = QStringLiteral("%1 B").arg(frame.size());
    m_framePayloadHex = payload.isEmpty() ? QStringLiteral("(空)") : payload.toHex(' ').toUpper();
    QString businessType;
    m_businessFields = decodeBusiness(command, payload, &businessType);
    m_businessType = businessType.isEmpty() ? QStringLiteral("通用协议帧") : businessType;
    emit frameChanged();
    appendLog(QStringLiteral("[解析] %1，%2，payload %3 B")
                  .arg(m_businessType, m_frameCrcStatus).arg(payload.size()));
}

void SerialDebug::setFrameError(const QString &status, const QString &detail)
{
    m_frameStatus = status;
    m_frameCrcStatus = detail;
    emit frameChanged();
}

quint16 SerialDebug::crc16Xmodem(const QByteArray &data)
{
    quint16 crc = 0;
    for (const char byte : data) {
        crc ^= static_cast<quint16>(static_cast<quint8>(byte)) << 8;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) ? static_cast<quint16>((crc << 1) ^ 0x1021) : static_cast<quint16>(crc << 1);
        }
    }
    return crc;
}

quint16 SerialDebug::readUint16(const QByteArray &data, int offset)
{
    return static_cast<quint16>((static_cast<quint8>(data.at(offset)) << 8) |
                                static_cast<quint8>(data.at(offset + 1)));
}

quint32 SerialDebug::readUint32(const QByteArray &data, int offset)
{
    return (static_cast<quint32>(static_cast<quint8>(data.at(offset))) << 24) |
           (static_cast<quint32>(static_cast<quint8>(data.at(offset + 1))) << 16) |
           (static_cast<quint32>(static_cast<quint8>(data.at(offset + 2))) << 8) |
           static_cast<quint32>(static_cast<quint8>(data.at(offset + 3)));
}

QString SerialDebug::hexByte(quint8 value)
{
    return QStringLiteral("0x%1").arg(value, 2, 16, QLatin1Char('0')).toUpper();
}

QVariantMap SerialDebug::field(const QString &name, const QString &value)
{
    return {{QStringLiteral("name"), name}, {QStringLiteral("value"), value}};
}

QVariantList SerialDebug::decodeBusiness(quint8 command, const QByteArray &payload, QString *type) const
{
    QVariantList fields;
    if (command == 0x21 && payload.size() == 26) {
        *type = QStringLiteral("DMR 测量");
        fields << field(QStringLiteral("频率模式"), hexByte(static_cast<quint8>(payload.at(0))))
               << field(QStringLiteral("捕获模式"), hexByte(static_cast<quint8>(payload.at(1))))
               << field(QStringLiteral("413MHz dbm1"), QString::number(readUint16(payload, 2)))
               << field(QStringLiteral("413MHz dbm2"), QString::number(readUint16(payload, 4)))
               << field(QStringLiteral("413MHz p1v"), QString::number(readUint16(payload, 6)))
               << field(QStringLiteral("413MHz p2v"), QString::number(readUint16(payload, 8)))
               << field(QStringLiteral("457MHz dbm1"), QString::number(readUint16(payload, 10)))
               << field(QStringLiteral("457MHz dbm2"), QString::number(readUint16(payload, 12)))
               << field(QStringLiteral("457MHz p1v"), QString::number(readUint16(payload, 14)))
               << field(QStringLiteral("457MHz p2v"), QString::number(readUint16(payload, 16)))
               << field(QStringLiteral("脉宽"), QString::number(readUint32(payload, 18)))
               << field(QStringLiteral("周期"), QString::number(readUint32(payload, 22)));
    } else if (command == 0x28 && payload.size() == 18) {
        *type = QStringLiteral("GSM 测量");
        fields << field(QStringLiteral("字段0"), hexByte(static_cast<quint8>(payload.at(0))))
               << field(QStringLiteral("字段1"), hexByte(static_cast<quint8>(payload.at(1))))
               << field(QStringLiteral("dbm1"), QString::number(readUint16(payload, 2)))
               << field(QStringLiteral("dbm2"), QString::number(readUint16(payload, 4)))
               << field(QStringLiteral("p1v"), QString::number(readUint16(payload, 6)))
               << field(QStringLiteral("p2v"), QString::number(readUint16(payload, 8)))
               << field(QStringLiteral("脉宽"), QString::number(readUint32(payload, 10)))
               << field(QStringLiteral("周期"), QString::number(readUint32(payload, 14)));
    } else if (command == 0x22 && payload.size() == 36) {
        *type = QStringLiteral("GNSS 数据");
        fields << field(QStringLiteral("定位质量"), QString::number(static_cast<quint8>(payload.at(9))))
               << field(QStringLiteral("经度方向"), QString(QChar(payload.at(10))))
               << field(QStringLiteral("经度原值"), QString::number(readUint32(payload, 11)))
               << field(QStringLiteral("纬度方向"), QString(QChar(payload.at(15))))
               << field(QStringLiteral("纬度原值"), QString::number(readUint32(payload, 16)))
               << field(QStringLiteral("总卫星数"), QString::number(static_cast<quint8>(payload.at(20))))
               << field(QStringLiteral("GPS卫星数"), QString::number(static_cast<quint8>(payload.at(21))))
               << field(QStringLiteral("北斗卫星数"), QString::number(static_cast<quint8>(payload.at(22))))
               << field(QStringLiteral("GPS最大SNR"), QString::number(static_cast<quint8>(payload.at(23))))
               << field(QStringLiteral("北斗最大SNR"), QString::number(static_cast<quint8>(payload.at(24))))
               << field(QStringLiteral("速度原值"), QString::number(readUint16(payload, 25)))
               << field(QStringLiteral("高度原值"), QString::number(readUint16(payload, 27)))
               << field(QStringLiteral("UTC"), QStringLiteral("%1-%2-%3 %4:%5:%6")
                   .arg(readUint16(payload, 29), 4, 10, QLatin1Char('0'))
                   .arg(static_cast<quint8>(payload.at(31)), 2, 10, QLatin1Char('0'))
                   .arg(static_cast<quint8>(payload.at(32)), 2, 10, QLatin1Char('0'))
                   .arg(static_cast<quint8>(payload.at(33)), 2, 10, QLatin1Char('0'))
                   .arg(static_cast<quint8>(payload.at(34)), 2, 10, QLatin1Char('0'))
                   .arg(static_cast<quint8>(payload.at(35)), 2, 10, QLatin1Char('0')));
    }
    return fields;
}

QByteArray SerialDebug::encodeProtocolFrame(quint8 command, const QByteArray &payload) const
{
    const quint16 infoLength = static_cast<quint16>(8 + payload.size());
    QByteArray content;
    content.reserve(infoLength);
    content.append(static_cast<char>((infoLength >> 8) & 0xFFU));
    content.append(static_cast<char>(infoLength & 0xFFU));
    content.append(static_cast<char>(0x01));
    content.append(static_cast<char>(0x00));
    content.append(static_cast<char>(0x21));
    content.append(static_cast<char>(0x00));
    content.append(static_cast<char>(0x02));
    content.append(static_cast<char>(command));
    content.append(payload.constData(), payload.size());

    const quint16 crc = crc16Xmodem(content);
    QByteArray frame;
    frame.reserve(static_cast<int>(infoLength) + 6);
    frame.append(static_cast<char>(0x10));
    frame.append(static_cast<char>(0x02));

    const auto appendEscaped = [&frame](char byte) {
        frame.append(byte);
        if (static_cast<quint8>(byte) == 0x10U) {
            frame.append(static_cast<char>(0x10));
        }
    };

    for (const char byte : content) {
        appendEscaped(byte);
    }
    appendEscaped(static_cast<char>((crc >> 8) & 0xFFU));
    appendEscaped(static_cast<char>(crc & 0xFFU));
    frame.append(static_cast<char>(0x10));
    frame.append(static_cast<char>(0x03));
    return frame;
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
