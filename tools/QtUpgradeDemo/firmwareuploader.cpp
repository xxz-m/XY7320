#include "firmwareuploader.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSerialPortInfo>
#include <QUrl>

FirmwareUploader::FirmwareUploader(QObject *parent)
    : QObject(parent)
{
    m_sendTimer.setSingleShot(true);
    connect(&m_sendTimer, &QTimer::timeout, this, &FirmwareUploader::sendNextPacket);
    refreshPorts();
}

QStringList FirmwareUploader::ports() const { return m_ports; }
QString FirmwareUploader::portName() const { return m_portName; }
int FirmwareUploader::baudRate() const { return m_baudRate; }
QString FirmwareUploader::filePath() const { return m_filePath; }
QString FirmwareUploader::headerHex() const { return m_headerHex; }
int FirmwareUploader::packetSize() const { return m_packetSize; }
int FirmwareUploader::headerDelayMs() const { return m_headerDelayMs; }
int FirmwareUploader::packetDelayMs() const { return m_packetDelayMs; }
double FirmwareUploader::progress() const { return m_progress; }
bool FirmwareUploader::busy() const { return m_busy; }
QString FirmwareUploader::status() const { return m_status; }
QString FirmwareUploader::logText() const { return m_logText; }

void FirmwareUploader::setPortName(const QString &portName)
{
    if (m_portName == portName) {
        return;
    }
    m_portName = portName;
    emit portNameChanged();
}

void FirmwareUploader::setBaudRate(int baudRate)
{
    if (m_baudRate == baudRate) {
        return;
    }
    m_baudRate = baudRate;
    emit baudRateChanged();
}

void FirmwareUploader::setFilePath(const QString &filePath)
{
    if (m_filePath == filePath) {
        return;
    }
    m_filePath = filePath;
    emit filePathChanged();
    updateHeaderHex();
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
    QStringList names;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        names.append(info.portName());
    }

    m_ports = names;
    emit portsChanged();

    if (m_portName.isEmpty() && !m_ports.isEmpty()) {
        setPortName(m_ports.first());
    }
}

void FirmwareUploader::setFileUrl(const QUrl &url)
{
    setFilePath(url.isLocalFile() ? url.toLocalFile() : url.toString());
}

void FirmwareUploader::start()
{
    if (m_busy) {
        return;
    }

    if (m_portName.isEmpty()) {
        appendLog("未选择串口。");
        return;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        appendLog(QString("打开固件失败：%1").arg(file.errorString()));
        return;
    }

    m_firmware = file.readAll();
    if (m_firmware.isEmpty()) {
        appendLog("固件文件为空。");
        return;
    }

    m_crc = crc32(m_firmware);
    m_offset = 0;
    m_progress = 0.0;
    emit progressChanged();

    m_serial.close();
    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(m_baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        appendLog(QString("打开串口失败：%1").arg(m_serial.errorString()));
        return;
    }

    setBusy(true);
    setStatus("发送头包");
    appendLog(QString("串口 %1 已打开，波特率 %2。").arg(m_portName).arg(m_baudRate));
    appendLog(QString("固件文件：%1").arg(QFileInfo(m_filePath).fileName()));
    appendLog(QString("固件大小：%1 字节，CRC32：0x%2")
                  .arg(m_firmware.size())
                  .arg(QString::number(m_crc, 16).toUpper().rightJustified(8, '0')));
    appendLog(QString("包大小：%1 字节，擦除等待：%2 ms，包间隔：%3 ms。")
                  .arg(m_packetSize)
                  .arg(m_headerDelayMs)
                  .arg(m_packetDelayMs));

    if (!writeBytes(makeHeader())) {
        finish(false, "头包发送失败。");
        return;
    }

    appendLog("头包已发送，等待 bootloader 擦除 APP 分区。");
    setStatus("发送固件");
    m_sendTimer.start(m_headerDelayMs);
}

void FirmwareUploader::cancel()
{
    if (!m_busy) {
        return;
    }
    finish(false, "已取消。");
}

void FirmwareUploader::clearLog()
{
    m_logText.clear();
    emit logTextChanged();
}

void FirmwareUploader::sendNextPacket()
{
    if (!m_busy) {
        return;
    }

    if (m_offset >= m_firmware.size()) {
        finish(true, "发送完成。");
        return;
    }

    const qsizetype remaining = m_firmware.size() - m_offset;
    const qsizetype count = qMin<qsizetype>(remaining, m_packetSize);
    const QByteArray packet = m_firmware.mid(m_offset, count);

    if (!writeBytes(packet)) {
        finish(false, QString("发送失败，偏移 0x%1。").arg(QString::number(m_offset, 16).toUpper()));
        return;
    }

    m_offset += count;
    m_progress = static_cast<double>(m_offset) / static_cast<double>(m_firmware.size());
    emit progressChanged();

    appendLog(QString("已发送 %1 / %2").arg(m_offset).arg(m_firmware.size()));
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
    const QString stamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_logText.append(QString("[%1] %2\n").arg(stamp, line));
    emit logTextChanged();
}

bool FirmwareUploader::writeBytes(const QByteArray &data)
{
    const qint64 written = m_serial.write(data);
    if (written != data.size()) {
        appendLog(QString("串口写入长度异常：%1 / %2。").arg(written).arg(data.size()));
        return false;
    }

    if (!m_serial.waitForBytesWritten(3000)) {
        appendLog(QString("串口写入超时：%1").arg(m_serial.errorString()));
        return false;
    }

    return true;
}

void FirmwareUploader::finish(bool ok, const QString &message)
{
    m_sendTimer.stop();
    m_serial.close();
    setBusy(false);
    setStatus(ok ? "完成" : "已停止");
    appendLog(message);

    if (ok) {
        m_progress = 1.0;
        emit progressChanged();
    }
}
