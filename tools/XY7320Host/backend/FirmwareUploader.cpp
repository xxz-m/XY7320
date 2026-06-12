#include "FirmwareUploader.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QClipboard>
#include <QSerialPortInfo>
#include <QSet>
#include <QSettings>

#include <utility>

FirmwareUploader::FirmwareUploader(QObject *parent)
    : QObject(parent)
{
    m_sendTimer.setSingleShot(true);
    connect(&m_sendTimer, &QTimer::timeout, this, &FirmwareUploader::sendNextPacket);

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
        setStatus(QStringLiteral("就绪"));
        appendLog(QStringLiteral("串口已变更，手动发送会话已重置。"));
    } else if (m_serial.isOpen()) {
        m_serial.close();
        updateSerialOpenState();
        setStatus(QStringLiteral("已关闭"));
        appendLog(QStringLiteral("串口已变更，串口已关闭。"));
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
        setStatus(QStringLiteral("就绪"));
        appendLog(QStringLiteral("波特率已变更，手动发送会话已重置。"));
    } else if (m_serial.isOpen()) {
        m_serial.close();
        updateSerialOpenState();
        setStatus(QStringLiteral("已关闭"));
        appendLog(QStringLiteral("波特率已变更，串口已关闭。"));
    }
    m_baudRate = baudRate;
    emit baudRateChanged();
}

void FirmwareUploader::setFilePath(const QString &filePath)
{
    if (m_filePath == filePath) {
        return;
    }
    if (m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("就绪"));
        appendLog(QStringLiteral("固件已变更，手动发送会话已重置。"));
    }
    m_filePath = filePath;
    emit filePathChanged();
    resetPreparedFirmware();
    updateFileSnapshot();
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
        setStatus(QStringLiteral("串口已打开"));
        appendLog(QStringLiteral("串口 %1 已打开，波特率 %2。").arg(m_portName).arg(m_baudRate));
    }
}

void FirmwareUploader::closePort()
{
    if (m_busy) {
        return;
    }

    if (m_serial.isOpen() || m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("已关闭"));
        appendLog(QStringLiteral("串口已关闭。"));
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

    setBusy(true);
    setStatus(QStringLiteral("发送头包"));
    appendLog(QStringLiteral("串口 %1 已打开，波特率 %2。").arg(m_portName).arg(m_baudRate));
    appendLog(QStringLiteral("固件：%1，大小：%2 字节，CRC32：0x%3")
                  .arg(QFileInfo(m_filePath).fileName())
                  .arg(m_firmware.size())
                  .arg(QString::number(m_crc, 16).toUpper().rightJustified(8, QLatin1Char('0'))));
    appendLog(QStringLiteral("包大小：%1 字节，擦除等待：%2 ms，包间隔：%3 ms。")
                  .arg(m_packetSize)
                  .arg(m_headerDelayMs)
                  .arg(m_packetDelayMs));

    if (!writeBytes(makeHeader())) {
        finish(false, QStringLiteral("头包发送失败。"));
        return;
    }

    appendLog(QStringLiteral("头包已发送，等待 Bootloader 擦除 APP 分区。"));
    setStatus(QStringLiteral("等待擦除"));
    m_sendTimer.start(m_headerDelayMs);
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
        setStatus(QStringLiteral("头包失败"));
        appendLog(QStringLiteral("头包发送失败。"));
        return;
    }

    setManualHeaderSent(true);
    setStatus(QStringLiteral("头包已发送"));
    appendLog(QStringLiteral("手动发送头包完成，可继续发送固件。"));
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
    setStatus(QStringLiteral("发送固件"));
    appendLog(QStringLiteral("手动启动固件发送，按 %1 字节分包，间隔 %2 ms。")
                  .arg(m_packetSize)
                  .arg(m_packetDelayMs));
    m_sendTimer.start(0);
}

void FirmwareUploader::copyHeaderToClipboard()
{
    if (m_headerHex.isEmpty()) {
        appendLog(QStringLiteral("头包为空，无法复制。"));
        setStatus(QStringLiteral("头包为空"));
        return;
    }

    QGuiApplication::clipboard()->setText(m_headerHex);
    appendLog(QStringLiteral("头包已复制到剪贴板。"));
    setStatus(QStringLiteral("已复制"));
}

void FirmwareUploader::cancel()
{
    if (m_busy) {
        finish(false, QStringLiteral("已取消。"));
        return;
    }

    if (m_serial.isOpen() || m_manualHeaderSent) {
        closeSession();
        setStatus(QStringLiteral("已停止"));
        appendLog(QStringLiteral("串口已关闭。"));
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
        setStatus(QStringLiteral("就绪"));
        appendLog(QStringLiteral("固件文件已变化，手动发送会话已重置。"));
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
        finish(true, QStringLiteral("发送完成。"));
        return;
    }

    const qsizetype remaining = m_firmware.size() - m_offset;
    const qsizetype count = qMin<qsizetype>(remaining, m_packetSize);
    const QByteArray packet = m_firmware.mid(m_offset, count);

    if (!writeBytes(packet)) {
        finish(false, QStringLiteral("发送失败，偏移 0x%1。")
                          .arg(QString::number(m_offset, 16).toUpper()));
        return;
    }

    m_offset += count;
    m_progress = static_cast<double>(m_offset) / static_cast<double>(m_firmware.size());
    emit progressChanged();

    appendLog(QStringLiteral("已发送 %1 / %2").arg(m_offset).arg(m_firmware.size()));
    setStatus(QStringLiteral("发送固件"));
    m_sendTimer.start(m_packetDelayMs);
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

bool FirmwareUploader::prepareFirmware()
{
    if (m_portName.isEmpty()) {
        appendLog(QStringLiteral("未选择升级串口。"));
        setStatus(QStringLiteral("缺少串口"));
        return false;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        appendLog(QStringLiteral("打开固件失败：%1").arg(file.errorString()));
        setStatus(QStringLiteral("固件错误"));
        return false;
    }

    m_firmware = file.readAll();
    if (m_firmware.isEmpty()) {
        appendLog(QStringLiteral("固件文件为空。"));
        setStatus(QStringLiteral("固件错误"));
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
            nextFileSize = QStringLiteral("%1 字节").arg(firmware.size());
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
    } else {
        m_fileLastModified = QDateTime();
        m_fileLastSize = -1;
    }
    updateFileInfo();
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
        appendLog(QStringLiteral("未选择升级串口。"));
        setStatus(QStringLiteral("缺少串口"));
        return false;
    }

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(m_baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        appendLog(QStringLiteral("打开串口失败：%1").arg(m_serial.errorString()));
        setStatus(QStringLiteral("串口失败"));
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
        appendLog(QStringLiteral("串口写入长度异常：%1 / %2。").arg(written).arg(data.size()));
        return false;
    }

    if (!m_serial.waitForBytesWritten(3000)) {
        appendLog(QStringLiteral("串口写入超时：%1").arg(m_serial.errorString()));
        return false;
    }

    return true;
}

void FirmwareUploader::finish(bool ok, const QString &message)
{
    m_sendTimer.stop();
    closeSession();
    setBusy(false);
    setStatus(ok ? QStringLiteral("完成") : QStringLiteral("已停止"));
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
