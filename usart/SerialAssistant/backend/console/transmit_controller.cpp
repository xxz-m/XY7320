/**
 * @file transmit_controller.cpp
 * @brief TransmitController implementation.
 */

#include "transmit_controller.h"

#include "../../models/console_record_model.h"
#include "../io/serial_port_controller.h"
#include "../settings/settings_manager.h"
#include "text_codec.h"

#include <QDateTime>

TransmitController::TransmitController(SerialPortController* serialController,
                                       ConsoleRecordModel* recordModel,
                                       SettingsManager* settingsManager,
                                       QObject* parent)
    : QObject(parent)
    , m_serialController(serialController)
    , m_recordModel(recordModel)
    , m_settingsManager(settingsManager)
    , m_hexSend(settingsManager ? settingsManager->hexSend() : false)
    , m_loopSend(false)
    , m_loopIntervalMs(settingsManager ? settingsManager->loopIntervalMs() : 200)
    , m_lineEnding(settingsManager && settingsManager->appendNewline() ? CRLF : (settingsManager ? settingsManager->lineEnding() : None))
{
    m_loopTimer.setInterval(m_loopIntervalMs);
    connect(&m_loopTimer, &QTimer::timeout, this, &TransmitController::sendLoopPayload);

    if (m_serialController) {
        connect(m_serialController, &SerialPortController::errorOccurred,
                this, &TransmitController::handleSerialError);
        connect(m_serialController, &SerialPortController::isOpenChanged, this, [this]() {
            if (m_serialController && !m_serialController->isOpen())
                stopLoopSend();
        });
    }

}

bool TransmitController::hexSend() const { return m_hexSend; }
bool TransmitController::loopSend() const { return m_loopSend; }
int TransmitController::loopIntervalMs() const { return m_loopIntervalMs; }
int TransmitController::lineEnding() const { return m_lineEnding; }
qulonglong TransmitController::txBytes() const { return m_txBytes; }
qulonglong TransmitController::txFrames() const { return m_txFrames; }
QString TransmitController::errorText() const { return m_errorText; }

void TransmitController::setHexSend(bool hexSend)
{
    if (m_hexSend == hexSend)
        return;
    m_hexSend = hexSend;
    if (m_settingsManager)
        m_settingsManager->setHexSend(hexSend);
    Q_EMIT hexSendChanged();
}

void TransmitController::setLoopSend(bool loopSend)
{
    if (loopSend) {
        startLoopSend(m_loopInput);
        return;
    }
    stopLoopSend();
}

void TransmitController::setLoopIntervalMs(int loopIntervalMs)
{
    const int boundedInterval = qBound(10, loopIntervalMs, 60000);
    if (m_loopIntervalMs == boundedInterval)
        return;
    m_loopIntervalMs = boundedInterval;
    m_loopTimer.setInterval(m_loopIntervalMs);
    if (m_settingsManager)
        m_settingsManager->setLoopIntervalMs(m_loopIntervalMs);
    Q_EMIT loopIntervalMsChanged();
}

void TransmitController::setLineEnding(int lineEnding)
{
    if (m_lineEnding == lineEnding)
        return;
    m_lineEnding = lineEnding;
    if (m_settingsManager)
        m_settingsManager->setLineEnding(lineEnding);
    Q_EMIT lineEndingChanged();
}

bool TransmitController::send(const QString& input)
{
    QString error;
    const QByteArray payload = buildPayload(input, &error);
    if (!error.isEmpty()) {
        setErrorText(error);
        if (m_recordModel)
            m_recordModel->appendSystem(error, true);
        return false;
    }

    if (!m_serialController || !m_serialController->writeBytes(payload)) {
        const QString message = m_serialController ? m_serialController->errorText() : QStringLiteral("串口服务不可用");
        setErrorText(message);
        if (m_recordModel)
            m_recordModel->appendSystem(message, true);
        return false;
    }

    setErrorText(QString());
    recordTransmit(payload);
    return true;
}

void TransmitController::startLoopSend(const QString& input)
{
    m_loopInput = input;
    if (m_loopInput.isEmpty()) {
        setErrorText(QStringLiteral("循环发送内容为空"));
        return;
    }

    if (!m_serialController || !m_serialController->isOpen()) {
        setErrorText(QStringLiteral("串口未打开"));
        return;
    }

    if (!m_loopSend) {
        m_loopSend = true;
        if (m_settingsManager)
            m_settingsManager->setLoopSend(true);
        Q_EMIT loopSendChanged();
    }

    if (!m_loopTimer.isActive())
        m_loopTimer.start();
}

void TransmitController::stopLoopSend()
{
    if (m_loopTimer.isActive())
        m_loopTimer.stop();

    if (!m_loopSend)
        return;

    m_loopSend = false;
    if (m_settingsManager)
        m_settingsManager->setLoopSend(false);
    Q_EMIT loopSendChanged();
}

void TransmitController::sendLoopPayload()
{
    if (!send(m_loopInput))
        stopLoopSend();
}

void TransmitController::handleSerialError(const QString& message)
{
    setErrorText(message);
}

QByteArray TransmitController::buildPayload(const QString& input, QString* errorText) const
{
    if (input.isEmpty()) {
        if (errorText)
            *errorText = QStringLiteral("发送内容为空");
        return QByteArray();
    }

    if (m_hexSend)
        return TextCodec::parseHex(input, errorText);

    if (errorText)
        errorText->clear();
    const TextCodec::Encoding encoding = TextCodec::encodingFromName(m_settingsManager ? m_settingsManager->sendEncoding() : QString());
    QByteArray payload = TextCodec::encodeText(input, encoding);
    payload.append(lineEndingBytes());
    return payload;
}

QByteArray TransmitController::lineEndingBytes() const
{
    switch (m_lineEnding) {
    case LF:
        return QByteArray("\n", 1);
    case CR:
        return QByteArray("\r", 1);
    case CRLF:
        return QByteArray("\r\n", 2);
    case None:
    default:
        return QByteArray();
    }
}

void TransmitController::setErrorText(const QString& errorText)
{
    if (m_errorText == errorText)
        return;
    m_errorText = errorText;
    Q_EMIT errorTextChanged();
}

void TransmitController::recordTransmit(const QByteArray& payload)
{
    m_txBytes += static_cast<qulonglong>(payload.size());
    ++m_txFrames;
    if (m_recordModel) {
        const TextCodec::Encoding encoding = TextCodec::encodingFromName(m_settingsManager ? m_settingsManager->sendEncoding() : QString());
        const QString text = TextCodec::decodeText(payload, encoding).trimmed();
        m_recordModel->appendTransmit(QDateTime::currentDateTime(), payload, text, TextCodec::formatHex(payload));
    }
    Q_EMIT statisticsChanged();
}
