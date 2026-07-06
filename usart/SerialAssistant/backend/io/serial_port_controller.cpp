/**
 * @file serial_port_controller.cpp
 * @brief SerialPortController implementation.
 */

#include "serial_port_controller.h"

#include "../settings/settings_manager.h"

#include <QSerialPortInfo>
#include <utility>

SerialPortController::SerialPortController(SettingsManager* settingsManager, QObject* parent)
    : QObject(parent)
    , m_settingsManager(settingsManager)
    , m_portName(settingsManager ? settingsManager->portName() : QString())
    , m_baudRate(settingsManager ? settingsManager->baudRate() : QSerialPort::Baud115200)
    , m_dataBits(settingsManager ? settingsManager->dataBits() : QSerialPort::Data8)
    , m_parity(settingsManager ? settingsManager->parity() : QSerialPort::NoParity)
    , m_stopBits(settingsManager ? settingsManager->stopBits() : QSerialPort::OneStop)
    , m_flowControl(settingsManager ? settingsManager->flowControl() : QSerialPort::NoFlowControl)
    , m_statusText(QStringLiteral("未连接"))
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialPortController::readAvailableData);
    connect(&m_serialPort, &QSerialPort::bytesWritten, this, &SerialPortController::bytesWritten);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortController::handleSerialError);

    m_refreshTimer.setInterval(1500);
    connect(&m_refreshTimer, &QTimer::timeout, this, &SerialPortController::refreshPorts);
    refreshPorts();
    m_refreshTimer.start();
}

SerialPortController::~SerialPortController()
{
    m_refreshTimer.stop();
    closePort();
}

QVariantList SerialPortController::ports() const { return m_ports; }
QString SerialPortController::portName() const { return m_portName; }
int SerialPortController::baudRate() const { return m_baudRate; }
int SerialPortController::dataBits() const { return m_dataBits; }
int SerialPortController::parity() const { return m_parity; }
int SerialPortController::stopBits() const { return m_stopBits; }
int SerialPortController::flowControl() const { return m_flowControl; }

QVariantList SerialPortController::baudRateOptions() const
{
    return {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
}

QVariantList SerialPortController::dataBitsOptions() const
{
    return {5, 6, 7, 8};
}

QVariantList SerialPortController::parityOptions() const
{
    return {
        QVariantMap{{QStringLiteral("text"), QStringLiteral("无校验")}, {QStringLiteral("value"), QSerialPort::NoParity}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("偶校验")}, {QStringLiteral("value"), QSerialPort::EvenParity}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("奇校验")}, {QStringLiteral("value"), QSerialPort::OddParity}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("空格校验")}, {QStringLiteral("value"), QSerialPort::SpaceParity}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("标记校验")}, {QStringLiteral("value"), QSerialPort::MarkParity}},
    };
}

QVariantList SerialPortController::stopBitsOptions() const
{
    return {
        QVariantMap{{QStringLiteral("text"), QStringLiteral("1")}, {QStringLiteral("value"), QSerialPort::OneStop}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("1.5")}, {QStringLiteral("value"), QSerialPort::OneAndHalfStop}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("2")}, {QStringLiteral("value"), QSerialPort::TwoStop}},
    };
}

QVariantList SerialPortController::flowControlOptions() const
{
    return {
        QVariantMap{{QStringLiteral("text"), QStringLiteral("无流控")}, {QStringLiteral("value"), QSerialPort::NoFlowControl}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("硬件流控")}, {QStringLiteral("value"), QSerialPort::HardwareControl}},
        QVariantMap{{QStringLiteral("text"), QStringLiteral("软件流控")}, {QStringLiteral("value"), QSerialPort::SoftwareControl}},
    };
}

bool SerialPortController::isOpen() const { return m_serialPort.isOpen(); }
QString SerialPortController::statusText() const { return m_statusText; }
QString SerialPortController::errorText() const { return m_errorText; }

void SerialPortController::setPortName(const QString& portName)
{
    if (m_portName == portName)
        return;
    closeForParameterChange();
    m_portName = portName;
    if (m_settingsManager)
        m_settingsManager->setPortName(portName);
    Q_EMIT portNameChanged();
}

void SerialPortController::setBaudRate(int baudRate)
{
    if (m_baudRate == baudRate)
        return;
    closeForParameterChange();
    m_baudRate = baudRate;
    if (m_settingsManager)
        m_settingsManager->setBaudRate(baudRate);
    Q_EMIT baudRateChanged();
}

void SerialPortController::setDataBits(int dataBits)
{
    if (m_dataBits == dataBits)
        return;
    closeForParameterChange();
    m_dataBits = dataBits;
    if (m_settingsManager)
        m_settingsManager->setDataBits(dataBits);
    Q_EMIT dataBitsChanged();
}

void SerialPortController::setParity(int parity)
{
    if (m_parity == parity)
        return;
    closeForParameterChange();
    m_parity = parity;
    if (m_settingsManager)
        m_settingsManager->setParity(parity);
    Q_EMIT parityChanged();
}

void SerialPortController::setStopBits(int stopBits)
{
    if (m_stopBits == stopBits)
        return;
    closeForParameterChange();
    m_stopBits = stopBits;
    if (m_settingsManager)
        m_settingsManager->setStopBits(stopBits);
    Q_EMIT stopBitsChanged();
}

void SerialPortController::setFlowControl(int flowControl)
{
    if (m_flowControl == flowControl)
        return;
    closeForParameterChange();
    m_flowControl = flowControl;
    if (m_settingsManager)
        m_settingsManager->setFlowControl(flowControl);
    Q_EMIT flowControlChanged();
}

void SerialPortController::refreshPorts()
{
    QVariantList nextPorts;
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
        QVariantMap item;
        const QString displayName = info.description().isEmpty()
                                    ? info.portName()
                                    : QStringLiteral("%1 · %2").arg(info.portName(), info.description());
        item.insert(QStringLiteral("name"), info.portName());
        item.insert(QStringLiteral("value"), info.portName());
        item.insert(QStringLiteral("text"), displayName);
        item.insert(QStringLiteral("systemLocation"), info.systemLocation());
        item.insert(QStringLiteral("description"), info.description());
        item.insert(QStringLiteral("manufacturer"), info.manufacturer());
        item.insert(QStringLiteral("displayName"), displayName);
        nextPorts.append(item);
    }

    if (m_ports == nextPorts)
        return;

    m_ports = std::move(nextPorts);
    Q_EMIT portsChanged();
}

void SerialPortController::openPort()
{
    if (m_serialPort.isOpen())
        return;

    if (m_portName.isEmpty()) {
        setErrorText(QStringLiteral("请选择串口"));
        Q_EMIT errorOccurred(m_errorText);
        return;
    }

    m_serialPort.setPortName(m_portName);
    applySettings();

    if (!m_serialPort.open(QIODevice::ReadWrite)) {
        setErrorText(QStringLiteral("打开串口失败：%1").arg(m_serialPort.errorString()));
        setStatusText(QStringLiteral("打开失败"));
        Q_EMIT errorOccurred(m_errorText);
        return;
    }

    setErrorText(QString());
    setStatusText(QStringLiteral("已连接 %1").arg(m_portName));
    Q_EMIT isOpenChanged();
}

void SerialPortController::closePort()
{
    if (!m_serialPort.isOpen())
        return;

    m_serialPort.close();
    setStatusText(QStringLiteral("未连接"));
    Q_EMIT isOpenChanged();
}

bool SerialPortController::writeBytes(const QByteArray& data)
{
    if (!m_serialPort.isOpen()) {
        setErrorText(QStringLiteral("串口未打开"));
        Q_EMIT errorOccurred(m_errorText);
        return false;
    }

    if (data.isEmpty()) {
        setErrorText(QStringLiteral("发送内容为空"));
        Q_EMIT errorOccurred(m_errorText);
        return false;
    }

    const qint64 written = m_serialPort.write(data);
    if (written < 0) {
        setErrorText(QStringLiteral("发送失败：%1").arg(m_serialPort.errorString()));
        Q_EMIT errorOccurred(m_errorText);
        return false;
    }

    setErrorText(QString());
    return true;
}

void SerialPortController::readAvailableData()
{
    const QByteArray data = m_serialPort.readAll();
    if (!data.isEmpty())
        Q_EMIT dataReceived(data);
}

void SerialPortController::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    if (error == QSerialPort::ResourceError) {
        const QString message = QStringLiteral("串口设备已断开或不可用：%1").arg(m_serialPort.errorString());
        setErrorText(message);
        closePort();
        Q_EMIT errorOccurred(message);
        return;
    }

    const QString message = QStringLiteral("串口错误：%1").arg(m_serialPort.errorString());
    setErrorText(message);
    Q_EMIT errorOccurred(message);
}

void SerialPortController::applySettings()
{
    m_serialPort.setBaudRate(m_baudRate);
    m_serialPort.setDataBits(static_cast<QSerialPort::DataBits>(m_dataBits));
    m_serialPort.setParity(static_cast<QSerialPort::Parity>(m_parity));
    m_serialPort.setStopBits(static_cast<QSerialPort::StopBits>(m_stopBits));
    m_serialPort.setFlowControl(static_cast<QSerialPort::FlowControl>(m_flowControl));
}

void SerialPortController::setStatusText(const QString& statusText)
{
    if (m_statusText == statusText)
        return;
    m_statusText = statusText;
    Q_EMIT statusTextChanged();
}

void SerialPortController::setErrorText(const QString& errorText)
{
    if (m_errorText == errorText)
        return;
    m_errorText = errorText;
    Q_EMIT errorTextChanged();
}

void SerialPortController::closeForParameterChange()
{
    if (m_serialPort.isOpen())
        closePort();
}
