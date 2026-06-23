#include "FirmwareUploader.h"

#include <QClipboard>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QSerialPortInfo>
#include <QSet>
#include <QSettings>

#include <utility>

namespace {
constexpr char kBootReadyAck[] = "XYB1";
constexpr char kBootHeaderAck[] = "XYB2";
constexpr char kBootFinishAck[] = "XYB3";
}

FirmwareUploader::FirmwareUploader(QObject *parent)
    : QObject(parent)
{
    m_sendTimer.setSingleShot(true);
    connect(&m_sendTimer, &QTimer::timeout, this, &FirmwareUploader::sendNextPacket);

    m_handshakeTimer.setSingleShot(true);
    m_handshakeTimer.setInterval(HandshakeTimeoutMs);
    connect(&m_handshakeTimer, &QTimer::timeout, this, &FirmwareUploader::handleHandshakeTimeout);

    connect(&m_serial, &QSerialPort::readyRead, this, &FirmwareUploader::handleSerialReadyRead);

    m_portRefreshTimer.setInterval(1500);
    connect(&m_portRefreshTimer, &QTimer::timeout, this, &FirmwareUploader::refreshPorts);
    m_portRefreshTimer.start();

    m_fileRefreshTimer.setInterval(1000);
    connect(&m_fileRefreshTimer, &QTimer::timeout, this, &FirmwareUploader::checkSelectedFile);
    m_fileRefreshTimer.start();

    refreshPorts();
}

QVariantList FirmwareUploader::ports() const { return m_ports; }
QString FirmwareUploader::portName() const { return m_portName; }
int FirmwareUploader::baudRate() const { return m_baudRate; }
QString FirmwareUploader::filePath() const { return m_filePath; }
QString FirmwareUploader::fileName() const { return m_fileName; }
QString FirmwareUploader::fileSizeText() const { return m_fileSizeText; }
QString FirmwareUploader::crcHex() const { return m_crcHex; }
QString FirmwareUploader::versionText() const { return m_versionText; }
int FirmwareUploader::versionFlag() const { return m_versionFlag; }
QString FirmwareUploader::versionFrameHex() const { return m_versionFrameHex; }
QString FirmwareUploader::headerHex() const { return m_headerHex; }
int FirmwareUploader::packetSize() const { return m_packetSize; }
int FirmwareUploader::headerDelayMs() const { return m_headerDelayMs; }
int FirmwareUploader::packetDelayMs() const { return m_packetDelayMs; }
double FirmwareUploader::progress() const { return m_progress; }
bool FirmwareUploader::busy() const { return m_busy; }
bool FirmwareUploader::serialOpen() const { return m_serialOpen; }
bool FirmwareUploader::manualHeaderSent() const { return m_manualHeaderSent; }
QString FirmwareUploader::status() const { return m_status; }
QString FirmwareUploader::logText() const { return m_logText; }

void FirmwareUploader::setPortName(const QString &portName)
{
    if (m_portName == portName) {
        return;
    }

    if (m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("Ready"));
        appendLog(QStringLiteral("Port changed, manual session reset."));
    } else if (m_serial.isOpen()) {
        m_serial.close();
        updateSerialOpenState();
        setStatus(QStringLiteral("Closed"));
        appendLog(QStringLiteral("Port changed, serial closed."));
    }

    m_portName = portName;
    emit portNameChanged();
}

void FirmwareUploader::setBaudRate(int baudRate)
{
    if (m_baudRate == baudRate) {
        return;
    }

    if (m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("Ready"));
        appendLog(QStringLiteral("Baud rate changed, manual session reset."));
    } else if (m_serial.isOpen()) {
        m_serial.close();
        updateSerialOpenState();
        setStatus(QStringLiteral("Closed"));
        appendLog(QStringLiteral("Baud rate changed, serial closed."));
    }

    m_baudRate = baudRate;
    emit baudRateChanged();
}

void FirmwareUploader::setFilePath(const QString &filePath)
{
    const bool samePath = (m_filePath == filePath);

    if (m_filePath == filePath) {
        resetPreparedFirmware();
        updateFileSnapshot();
        return;
    }

    if (m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("Ready"));
        appendLog(QStringLiteral("Firmware changed, manual session reset."));
    }

    m_filePath = filePath;
    if (!samePath) {
        emit filePathChanged();
    }
    resetPreparedFirmware();
    updateFileSnapshot();
}

void FirmwareUploader::setVersionText(const QString &versionText)
{
    QString normalized;
    for (const QChar ch : versionText) {
        if (ch.isDigit()) {
            normalized.append(ch);
        }
        if (normalized.size() >= VersionTextLength) {
            break;
        }
    }

    if (m_versionText == normalized) {
        return;
    }

    m_versionText = normalized;
    updateVersionInfo();
}

void FirmwareUploader::setVersionFlag(int versionFlag)
{
    const int clamped = qBound(0, versionFlag, 255);
    if (m_versionFlag == clamped) {
        return;
    }

    m_versionFlag = clamped;
    updateVersionInfo();
}

void FirmwareUploader::setPacketSize(int packetSize)
{
    const int clamped = qBound(1, packetSize, MaxPacketSize);
    if (m_packetSize == clamped) {
        return;
    }

    m_packetSize = clamped;
    emit packetSizeChanged();
}

void FirmwareUploader::setHeaderDelayMs(int headerDelayMs)
{
    const int clamped = qMax(0, headerDelayMs);
    if (m_headerDelayMs == clamped) {
        return;
    }

    m_headerDelayMs = clamped;
    emit headerDelayMsChanged();
}

void FirmwareUploader::setPacketDelayMs(int packetDelayMs)
{
    const int clamped = qMax(1, packetDelayMs);
    if (m_packetDelayMs == clamped) {
        return;
    }

    m_packetDelayMs = clamped;
    emit packetDelayMsChanged();
}

void FirmwareUploader::refreshPorts()
{
    QVariantList ports;
    QSet<QString> seenPorts;

    auto addPort = [&ports, &seenPorts](const QString &portName, const QString &description, const QString &manufacturer) {
        if (portName.isEmpty() || seenPorts.contains(portName)) {
            return;
        }

        QString text = portName;
        if (!description.isEmpty()) {
            text += QStringLiteral("(%1)").arg(description);
        }
        if (!manufacturer.isEmpty()) {
            text += QStringLiteral(" %1").arg(manufacturer);
        }

        QVariantMap item;
        item.insert(QStringLiteral("text"), text);
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

    if (m_busy || m_manualHeaderSent) {
        return;
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

void FirmwareUploader::setFileUrl(const QUrl &url)
{
    setFilePath(url.isLocalFile() ? url.toLocalFile() : url.toString());
}

void FirmwareUploader::openPort()
{
    if (m_busy || m_serial.isOpen()) {
        return;
    }

    if (openSerial()) {
        setStatus(QStringLiteral("Serial open"));
        appendLog(QStringLiteral("Serial %1 opened, baud %2.").arg(m_portName).arg(m_baudRate));
    }
}

void FirmwareUploader::closePort()
{
    if (m_busy) {
        return;
    }

    if (m_serial.isOpen() || m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("Closed"));
        appendLog(QStringLiteral("Serial closed."));
    }
}

void FirmwareUploader::start()
{
    if (m_busy) {
        return;
    }

    if (!prepareFirmware()) {
        return;
    }

    if (!openSerial()) {
        return;
    }

    if (!validateVersionFrameInput()) {
        return;
    }

    setBusy(true);
    setManualHeaderSent(false);
    m_rxBuffer.clear();
    m_serial.clear(QSerialPort::Input);
    m_autoStage = AutoStage::WaitAppVersionAck;

    setStatus(QStringLiteral("Wait upgrade ACK"));
    appendLog(QStringLiteral("[AUTO] TX protocol upgrade handshake, wait APP ACK."));

    if (!writeBytes(makeProtocolUpgradeHandshake())) {
        finish(false, QStringLiteral("[AUTO] TX protocol upgrade handshake failed."));
        return;
    }

    startHandshakeTimer();
}

void FirmwareUploader::sendHeaderManual()
{
    if (m_busy) {
        return;
    }

    if (!prepareFirmware()) {
        return;
    }

    if (!openSerial()) {
        return;
    }

    if (!writeBytes(makeHeader())) {
        closeSession();
        setStatus(QStringLiteral("Header failed"));
        appendLog(QStringLiteral("Header packet send failed."));
        return;
    }

    setManualHeaderSent(true);
    setStatus(QStringLiteral("Header sent"));
    appendLog(QStringLiteral("Manual header packet sent. Continue firmware after bootloader is ready."));
}

void FirmwareUploader::sendVersionFrameManual()
{
    if (m_busy) {
        return;
    }

    if (!prepareFirmware()) {
        return;
    }

    if (!validateVersionFrameInput()) {
        setStatus(QStringLiteral("Version error"));
        return;
    }

    if (!openSerial()) {
        return;
    }

    if (!writeBytes(makeProtocolUpgradeHandshake())) {
        setStatus(QStringLiteral("Version failed"));
        appendLog(QStringLiteral("Protocol upgrade handshake send failed."));
        return;
    }

    setStatus(QStringLiteral("Version sent"));
    appendLog(QStringLiteral("Protocol upgrade handshake sent: %1, flag=0x%2")
                  .arg(m_versionText)
                  .arg(QString::number(m_versionFlag, 16).toUpper().rightJustified(2, QLatin1Char('0'))));
}

void FirmwareUploader::sendFirmwareManual()
{
    if (m_busy || !m_manualHeaderSent) {
        return;
    }

    if (m_firmware.isEmpty() && !prepareFirmware()) {
        return;
    }

    if (!m_serial.isOpen() && !openSerial()) {
        return;
    }

    m_offset = 0;
    m_progress = 0.0;
    emit progressChanged();

    setBusy(true);
    m_autoStage = AutoStage::SendingFirmware;
    setStatus(QStringLiteral("Sending firmware"));
    appendLog(QStringLiteral("Manual firmware send started, packet=%1 bytes, delay=%2 ms.")
                  .arg(m_packetSize)
                  .arg(m_packetDelayMs));
    m_sendTimer.start(0);
}

void FirmwareUploader::copyHeaderToClipboard()
{
    if (m_headerHex.isEmpty()) {
        appendLog(QStringLiteral("Header packet is empty, copy skipped."));
        setStatus(QStringLiteral("Header empty"));
        return;
    }

    QGuiApplication::clipboard()->setText(m_headerHex);
    appendLog(QStringLiteral("Header packet copied to clipboard."));
    setStatus(QStringLiteral("Copied"));
}

void FirmwareUploader::cancel()
{
    if (m_busy) {
        finish(false, QStringLiteral("Canceled."));
        return;
    }

    if (m_serial.isOpen() || m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("Stopped"));
        appendLog(QStringLiteral("Serial closed."));
    }
}

void FirmwareUploader::clearLog()
{
    m_logText.clear();
    emit logTextChanged();
}

void FirmwareUploader::checkSelectedFile()
{
    if (m_filePath.isEmpty() || m_busy) {
        return;
    }

    QFileInfo info(m_filePath);
    const QDateTime lastModified = info.exists() ? info.lastModified() : QDateTime();
    const qint64 size = info.exists() ? info.size() : -1;
    if (lastModified == m_fileLastModified && size == m_fileLastSize) {
        return;
    }

    if (m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("Ready"));
        appendLog(QStringLiteral("Firmware file changed, manual session reset."));
    }

    resetPreparedFirmware();
    updateFileSnapshot();
}

void FirmwareUploader::sendNextPacket()
{
    if (!m_busy) {
        return;
    }

    if (m_offset >= m_firmware.size()) {
        if (m_autoStage == AutoStage::SendingFirmware) {
            m_autoStage = AutoStage::WaitFinishAck;
            setStatus(QStringLiteral("Wait XYB3"));
            appendLog(QStringLiteral("[AUTO] Firmware bytes sent, wait XYB3(bootloader verified and will jump)."));
            startHandshakeTimer();
            return;
        }

        finish(true, QStringLiteral("Send complete."));
        return;
    }

    const qsizetype remaining = m_firmware.size() - m_offset;
    const qsizetype count = qMin<qsizetype>(remaining, m_packetSize);
    const QByteArray packet = m_firmware.mid(m_offset, count);

    if (!writeBytes(packet)) {
        finish(false, QStringLiteral("Firmware send failed at offset 0x%1.")
                          .arg(QString::number(m_offset, 16).toUpper()));
        return;
    }

    m_offset += count;
    m_progress = static_cast<double>(m_offset) / static_cast<double>(m_firmware.size());
    emit progressChanged();

    appendLog(QStringLiteral("TX firmware %1 / %2").arg(m_offset).arg(m_firmware.size()));

    if (m_offset >= m_firmware.size() && m_autoStage == AutoStage::SendingFirmware) {
        m_autoStage = AutoStage::WaitFinishAck;
        setStatus(QStringLiteral("Wait XYB3"));
        appendLog(QStringLiteral("[AUTO] Firmware bytes sent, wait XYB3(bootloader verified and will jump)."));
        startHandshakeTimer();
        return;
    }

    m_sendTimer.start(m_packetDelayMs);
}

void FirmwareUploader::handleSerialReadyRead()
{
    m_rxBuffer.append(m_serial.readAll());
    consumeRxBuffer();
}

void FirmwareUploader::handleHandshakeTimeout()
{
    finish(false, QStringLiteral("[AUTO] Timeout at %1. Last RX hex: %2")
                      .arg(stageName(m_autoStage),
                           QString::fromLatin1(m_rxBuffer.toHex(' ').toUpper())));
}

QByteArray FirmwareUploader::makeHeader() const
{
    QByteArray header;
    header.reserve(12);
    appendLe32(&header, SimpleAppMagic);
    appendLe32(&header, static_cast<quint32>(m_firmware.size()));
    appendLe32(&header, m_crc);
    return header;
}

QByteArray FirmwareUploader::makeVersionFrame() const
{
    return makeProtocolUpgradeHandshake();
}

quint16 FirmwareUploader::protocolCrc16(const QByteArray &data) const
{
    quint16 crc = 0x0000;
    for (const auto byte : data) {
        crc ^= static_cast<quint16>(static_cast<quint8>(byte)) << 8;
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000) {
                crc = static_cast<quint16>((crc << 1) ^ 0x1021);
            } else {
                crc = static_cast<quint16>(crc << 1);
            }
        }
    }
    return crc;
}

QByteArray FirmwareUploader::makeProtocolUpgradeHandshake() const
{
    QByteArray frame;
    frame.reserve(32);

    QByteArray body;
    body.reserve(32);

    const quint16 infoLen = 8 + VersionTextLength + 1;
    body.append(static_cast<char>((infoLen >> 8) & 0xFF));
    body.append(static_cast<char>(infoLen & 0xFF));
    body.append(static_cast<char>(ProtocolPortPc));
    body.append(char(0x00));
    body.append(static_cast<char>(ProtocolPortDevice));
    body.append(char(0x00));
    body.append(static_cast<char>(ProtocolModelWrite));
    body.append(static_cast<char>(ProtocolCmdUpgradeHandshake));
    body.append(m_versionText.toLatin1());
    body.append(static_cast<char>(m_versionFlag & 0xFF));

    const quint16 crc = protocolCrc16(body);
    body.append(static_cast<char>((crc >> 8) & 0xFF));
    body.append(static_cast<char>(crc & 0xFF));

    frame.append(static_cast<char>(ProtocolHead1));
    frame.append(static_cast<char>(ProtocolHead2));
    for (const auto byte : body) {
        frame.append(byte);
        if (static_cast<quint8>(byte) == ProtocolEnd1) {
            frame.append(byte);
        }
    }
    frame.append(static_cast<char>(ProtocolEnd1));
    frame.append(static_cast<char>(ProtocolEnd2));
    return frame;
}

bool FirmwareUploader::validateVersionFrameInput()
{
    if (m_versionText.size() != VersionTextLength) {
        appendLog(QStringLiteral("Version length error, expected yyyyMMddHHmm."));
        return false;
    }

    for (const QChar ch : m_versionText) {
        if (!ch.isDigit()) {
            appendLog(QStringLiteral("Version must contain digits only."));
            return false;
        }
    }

    return true;
}

bool FirmwareUploader::prepareFirmware()
{
    if (m_portName.isEmpty()) {
        appendLog(QStringLiteral("No upgrade serial port selected."));
        setStatus(QStringLiteral("No port"));
        return false;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        appendLog(QStringLiteral("Open firmware failed: %1").arg(file.errorString()));
        setStatus(QStringLiteral("Firmware error"));
        return false;
    }

    m_firmware = file.readAll();
    if (m_firmware.isEmpty()) {
        appendLog(QStringLiteral("Firmware file is empty."));
        setStatus(QStringLiteral("Firmware error"));
        return false;
    }

    m_crc = crc32(m_firmware);
    m_offset = 0;
    m_progress = 0.0;
    emit progressChanged();
    return true;
}

void FirmwareUploader::updateFileInfo()
{
    QFile file(m_filePath);
    QString nextFileName = QStringLiteral("-");
    QString nextFileSize = QStringLiteral("-");
    QString nextCrc = QStringLiteral("-");

    if (file.open(QIODevice::ReadOnly)) {
        const QByteArray firmware = file.readAll();
        if (!firmware.isEmpty()) {
            const quint32 crc = crc32(firmware);
            nextFileName = QFileInfo(m_filePath).fileName();
            nextFileSize = QStringLiteral("%1 bytes").arg(firmware.size());
            nextCrc = QStringLiteral("0x%1")
                          .arg(QString::number(crc, 16).toUpper().rightJustified(8, QLatin1Char('0')));
        }
    }

    if (m_fileName == nextFileName && m_fileSizeText == nextFileSize && m_crcHex == nextCrc) {
        return;
    }

    m_fileName = nextFileName;
    m_fileSizeText = nextFileSize;
    m_crcHex = nextCrc;
    emit fileInfoChanged();
}

void FirmwareUploader::updateVersionInfo()
{
    QString nextFrameHex;

    if (m_versionText.size() == VersionTextLength) {
        const QByteArray frame = makeProtocolUpgradeHandshake();
        nextFrameHex = QString::fromLatin1(frame.toHex(' ').toUpper());
    }

    if (m_versionFrameHex == nextFrameHex) {
        return;
    }

    m_versionFrameHex = nextFrameHex;
    emit versionInfoChanged();
}

void FirmwareUploader::updateHeaderHex()
{
    QFile file(m_filePath);
    QString next;

    if (file.open(QIODevice::ReadOnly)) {
        const QByteArray firmware = file.readAll();
        if (!firmware.isEmpty()) {
            QByteArray header;
            header.reserve(12);
            appendLe32(&header, SimpleAppMagic);
            appendLe32(&header, static_cast<quint32>(firmware.size()));
            appendLe32(&header, crc32(firmware));
            next = QString::fromLatin1(header.toHex(' ').toUpper());
        }
    }

    if (m_headerHex == next) {
        return;
    }

    m_headerHex = next;
    emit headerHexChanged();
}

void FirmwareUploader::updateFileSnapshot()
{
    QFileInfo info(m_filePath);
    if (info.exists()) {
        m_fileLastModified = info.lastModified();
        m_fileLastSize = info.size();
        m_versionText = info.lastModified().toString(QStringLiteral("yyyyMMddHHmm"));
        emit versionInfoChanged();
    } else {
        m_fileLastModified = QDateTime();
        m_fileLastSize = -1;
    }

    updateFileInfo();
    updateVersionInfo();
    updateHeaderHex();
}

void FirmwareUploader::resetPreparedFirmware()
{
    m_firmware.clear();
    m_crc = 0;
    m_offset = 0;
    m_progress = 0.0;
    emit progressChanged();
}

quint32 FirmwareUploader::crc32(const QByteArray &data) const
{
    quint32 crc = 0xFFFFFFFFU;
    for (unsigned char byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i) {
            const quint32 mask = -(crc & 1U);
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }

    return ~crc;
}

void FirmwareUploader::appendLe32(QByteArray *data, quint32 value) const
{
    data->append(static_cast<char>(value & 0xFFU));
    data->append(static_cast<char>((value >> 8U) & 0xFFU));
    data->append(static_cast<char>((value >> 16U) & 0xFFU));
    data->append(static_cast<char>((value >> 24U) & 0xFFU));
}

void FirmwareUploader::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void FirmwareUploader::setManualHeaderSent(bool manualHeaderSent)
{
    if (m_manualHeaderSent == manualHeaderSent) {
        return;
    }

    m_manualHeaderSent = manualHeaderSent;
    emit manualHeaderSentChanged();
}

void FirmwareUploader::updateSerialOpenState()
{
    const bool open = m_serial.isOpen();
    if (m_serialOpen == open) {
        return;
    }

    m_serialOpen = open;
    emit serialOpenChanged();
}

void FirmwareUploader::setStatus(const QString &status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged();
}

void FirmwareUploader::appendLog(const QString &line)
{
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    m_logText.append(QStringLiteral("[%1] %2\n").arg(stamp, line));
    emit logTextChanged();
}

bool FirmwareUploader::openSerial()
{
    if (m_serial.isOpen()) {
        return true;
    }

    if (m_portName.isEmpty()) {
        appendLog(QStringLiteral("No upgrade serial port selected."));
        setStatus(QStringLiteral("No port"));
        return false;
    }

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(m_baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        appendLog(QStringLiteral("Open serial failed: %1").arg(m_serial.errorString()));
        setStatus(QStringLiteral("Serial failed"));
        updateSerialOpenState();
        return false;
    }

    updateSerialOpenState();
    return true;
}

bool FirmwareUploader::writeBytes(const QByteArray &data)
{
    const qint64 written = m_serial.write(data);
    if (written != data.size()) {
        appendLog(QStringLiteral("Serial write length error: %1 / %2.").arg(written).arg(data.size()));
        return false;
    }

    if (!m_serial.waitForBytesWritten(3000)) {
        appendLog(QStringLiteral("Serial write timeout: %1").arg(m_serial.errorString()));
        return false;
    }

    return true;
}

void FirmwareUploader::finish(bool ok, const QString &message)
{
    m_sendTimer.stop();
    stopHandshakeTimer();
    closeSession();
    setBusy(false);
    m_autoStage = AutoStage::Idle;
    m_rxBuffer.clear();
    setStatus(ok ? QStringLiteral("Complete") : QStringLiteral("Stopped"));
    appendLog(message);

    if (ok) {
        m_progress = 1.0;
        emit progressChanged();
    }
}

void FirmwareUploader::closeSession()
{
    m_serial.close();
    updateSerialOpenState();
    setManualHeaderSent(false);
}

void FirmwareUploader::startHandshakeTimer()
{
    m_handshakeTimer.start(HandshakeTimeoutMs);
}

void FirmwareUploader::stopHandshakeTimer()
{
    if (m_handshakeTimer.isActive()) {
        m_handshakeTimer.stop();
    }
}

void FirmwareUploader::consumeRxBuffer()
{
    if (tryConsumeProtocolUpgradeAck()) {
        return;
    }

    static const QList<QByteArray> codes = {
        QByteArray(kBootReadyAck, 4),
        QByteArray(kBootHeaderAck, 4),
        QByteArray(kBootFinishAck, 4)
    };

    bool consumed = true;
    while (consumed) {
        consumed = false;
        for (const QByteArray &code : codes) {
            const qsizetype index = m_rxBuffer.indexOf(code);
            if (index >= 0) {
                m_rxBuffer.remove(0, index + code.size());
                handleHandshakeCode(code);
                consumed = true;
                break;
            }
        }
    }

    if (m_rxBuffer.size() > 128) {
        m_rxBuffer.remove(0, m_rxBuffer.size() - 128);
    }
}

bool FirmwareUploader::tryConsumeProtocolUpgradeAck()
{
    if (m_autoStage != AutoStage::WaitAppVersionAck) {
        return false;
    }

    const QByteArray ack = makeProtocolUpgradeHandshake();
    Q_UNUSED(ack);

    const int minLen = 12;
    for (qsizetype start = 0; start + minLen <= m_rxBuffer.size(); ++start) {
        if (static_cast<quint8>(m_rxBuffer[start]) != ProtocolHead1 ||
            static_cast<quint8>(m_rxBuffer[start + 1]) != ProtocolHead2) {
            continue;
        }

        for (qsizetype end = start + minLen - 1; end < m_rxBuffer.size() - 1; ++end) {
            if (static_cast<quint8>(m_rxBuffer[end]) == ProtocolEnd1 &&
                static_cast<quint8>(m_rxBuffer[end + 1]) == ProtocolEnd2) {
                QByteArray packet = m_rxBuffer.mid(start, end + 2 - start);
                m_rxBuffer.remove(0, end + 2);

                QByteArray decoded;
                decoded.reserve(packet.size());
                decoded.append(packet[0]);
                decoded.append(packet[1]);
                for (qsizetype i = 2; i < packet.size() - 2; ++i) {
                    decoded.append(packet[i]);
                    if (static_cast<quint8>(packet[i]) == ProtocolEnd1 &&
                        i + 1 < packet.size() - 2 &&
                        static_cast<quint8>(packet[i + 1]) == ProtocolEnd1) {
                        ++i;
                    }
                }
                decoded.append(packet[packet.size() - 2]);
                decoded.append(packet[packet.size() - 1]);

                if (decoded.size() < 14) {
                    return true;
                }

                const quint8 originPort = static_cast<quint8>(decoded[4]);
                const quint8 goalPort = static_cast<quint8>(decoded[6]);
                const quint8 model = static_cast<quint8>(decoded[8]);
                const quint8 cmd = static_cast<quint8>(decoded[9]);
                const int dataLen = decoded.size() - 14;

                if (originPort == ProtocolPortDevice &&
                    goalPort == ProtocolPortPc &&
                    model == ProtocolModelWrite &&
                    cmd == ProtocolCmdUpgradeHandshake &&
                    dataLen == 0) {
                    handleHandshakeCode(QByteArrayLiteral("PROTO_ACK"));
                }
                return true;
            }
        }
    }

    return false;
}

void FirmwareUploader::handleHandshakeCode(const QByteArray &code)
{
    appendLog(QStringLiteral("[AUTO] RX code %1 at %2.")
                  .arg(QString::fromLatin1(code), stageName(m_autoStage)));

    if (code == QByteArrayLiteral("PROTO_ACK") && m_autoStage == AutoStage::WaitAppVersionAck) {
        m_autoStage = AutoStage::WaitBootReady;
        setStatus(QStringLiteral("Wait XYB1"));
        appendLog(QStringLiteral("[AUTO] APP accepted protocol upgrade handshake and reset. Wait XYB1 bootloader ready."));
        startHandshakeTimer();
        return;
    }

    if (code == kBootReadyAck && m_autoStage == AutoStage::WaitBootReady) {
        m_autoStage = AutoStage::WaitHeaderAck;
        setStatus(QStringLiteral("Wait XYB2"));
        appendLog(QStringLiteral("[AUTO] Bootloader ready. TX header packet, wait XYB2."));
        if (!writeBytes(makeHeader())) {
            finish(false, QStringLiteral("[AUTO] TX header packet failed."));
            return;
        }
        startHandshakeTimer();
        return;
    }

    if (code == kBootHeaderAck && m_autoStage == AutoStage::WaitHeaderAck) {
        m_autoStage = AutoStage::SendingFirmware;
        stopHandshakeTimer();
        setStatus(QStringLiteral("Sending firmware"));
        appendLog(QStringLiteral("[AUTO] Bootloader accepted header and waits app data. Start firmware TX."));
        m_offset = 0;
        m_progress = 0.0;
        emit progressChanged();
        m_sendTimer.start(0);
        return;
    }

    if (code == kBootFinishAck && m_autoStage == AutoStage::WaitFinishAck) {
        finish(true, QStringLiteral("[AUTO] Upgrade complete. Bootloader verified firmware and is jumping to APP."));
        return;
    }

    appendLog(QStringLiteral("[AUTO] Ignore code %1 in stage %2.")
                  .arg(QString::fromLatin1(code), stageName(m_autoStage)));
}

QString FirmwareUploader::stageName(AutoStage stage) const
{
    switch (stage) {
    case AutoStage::Idle:
        return QStringLiteral("Idle");
    case AutoStage::WaitAppVersionAck:
        return QStringLiteral("WaitAppVersionAck(PROTO_ACK)");
    case AutoStage::WaitBootReady:
        return QStringLiteral("WaitBootReady(XYB1)");
    case AutoStage::WaitHeaderAck:
        return QStringLiteral("WaitHeaderAck(XYB2)");
    case AutoStage::SendingFirmware:
        return QStringLiteral("SendingFirmware");
    case AutoStage::WaitFinishAck:
        return QStringLiteral("WaitFinishAck(XYB3)");
    }

    return QStringLiteral("Unknown");
}
