/**
 * @file serial_port_controller.h
 * @brief Serial port IO service for SerialAssistant.
 */

#ifndef SERIAL_ASSISTANT_SERIAL_PORT_CONTROLLER_H
#define SERIAL_ASSISTANT_SERIAL_PORT_CONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QVariantList>

class SettingsManager;

/**
 * Owns QSerialPort and exposes raw asynchronous serial IO to the application.
 */
class SerialPortController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(int dataBits READ dataBits WRITE setDataBits NOTIFY dataBitsChanged)
    Q_PROPERTY(int parity READ parity WRITE setParity NOTIFY parityChanged)
    Q_PROPERTY(int stopBits READ stopBits WRITE setStopBits NOTIFY stopBitsChanged)
    Q_PROPERTY(int flowControl READ flowControl WRITE setFlowControl NOTIFY flowControlChanged)
    Q_PROPERTY(QVariantList baudRateOptions READ baudRateOptions CONSTANT)
    Q_PROPERTY(QVariantList dataBitsOptions READ dataBitsOptions CONSTANT)
    Q_PROPERTY(QVariantList parityOptions READ parityOptions CONSTANT)
    Q_PROPERTY(QVariantList stopBitsOptions READ stopBitsOptions CONSTANT)
    Q_PROPERTY(QVariantList flowControlOptions READ flowControlOptions CONSTANT)
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY isOpenChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)

public:
    explicit SerialPortController(SettingsManager* settingsManager, QObject* parent = nullptr);
    ~SerialPortController() override;

    [[nodiscard]] QVariantList ports() const;
    [[nodiscard]] QString portName() const;
    [[nodiscard]] int baudRate() const;
    [[nodiscard]] int dataBits() const;
    [[nodiscard]] int parity() const;
    [[nodiscard]] int stopBits() const;
    [[nodiscard]] int flowControl() const;
    [[nodiscard]] QVariantList baudRateOptions() const;
    [[nodiscard]] QVariantList dataBitsOptions() const;
    [[nodiscard]] QVariantList parityOptions() const;
    [[nodiscard]] QVariantList stopBitsOptions() const;
    [[nodiscard]] QVariantList flowControlOptions() const;
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] QString errorText() const;

    void setPortName(const QString& portName);
    void setBaudRate(int baudRate);
    void setDataBits(int dataBits);
    void setParity(int parity);
    void setStopBits(int stopBits);
    void setFlowControl(int flowControl);

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void openPort();
    Q_INVOKABLE void closePort();
    Q_INVOKABLE bool writeBytes(const QByteArray& data);

signals:
    void portsChanged();
    void portNameChanged();
    void baudRateChanged();
    void dataBitsChanged();
    void parityChanged();
    void stopBitsChanged();
    void flowControlChanged();
    void isOpenChanged();
    void statusTextChanged();
    void errorTextChanged();
    void dataReceived(const QByteArray& data);
    void bytesWritten(qint64 bytes);
    void errorOccurred(const QString& message);

private slots:
    void readAvailableData();
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    void applySettings();
    void setStatusText(const QString& statusText);
    void setErrorText(const QString& errorText);
    void closeForParameterChange();

    SettingsManager* m_settingsManager;
    QSerialPort m_serialPort;
    QVariantList m_ports;
    QTimer m_refreshTimer;
    QString m_portName;
    int m_baudRate;
    int m_dataBits;
    int m_parity;
    int m_stopBits;
    int m_flowControl;
    QString m_statusText;
    QString m_errorText;
};

#endif
