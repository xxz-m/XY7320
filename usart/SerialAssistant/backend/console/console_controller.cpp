/**
 * @file console_controller.cpp
 * @brief ConsoleController implementation.
 */

#include "console_controller.h"

#include "../../models/console_record_model.h"
#include "../io/serial_port_controller.h"
#include "../settings/settings_manager.h"
#include "text_codec.h"

ConsoleController::ConsoleController(SerialPortController* serialController,
                                     ConsoleRecordModel* recordModel,
                                     SettingsManager* settingsManager,
                                     QObject* parent)
    : QObject(parent)
    , m_recordModel(recordModel)
    , m_settingsManager(settingsManager)
{
    m_flushTimer.setInterval(30);
    m_flushTimer.setSingleShot(true);
    connect(&m_flushTimer, &QTimer::timeout, this, &ConsoleController::flushReceivedData);

    if (serialController) {
        connect(serialController, &SerialPortController::dataReceived,
                this, &ConsoleController::appendReceivedBytes);
    }
}

ConsoleRecordModel* ConsoleController::records() const { return m_recordModel; }
bool ConsoleController::receivePaused() const { return m_receivePaused; }
qulonglong ConsoleController::rxBytes() const { return m_rxBytes; }
qulonglong ConsoleController::rxFrames() const { return m_rxFrames; }

void ConsoleController::setReceivePaused(bool receivePaused)
{
    if (m_receivePaused == receivePaused)
        return;
    m_receivePaused = receivePaused;
    Q_EMIT receivePausedChanged();
}

void ConsoleController::clear()
{
    m_flushTimer.stop();
    m_pendingData.clear();
    m_pendingTimestamp = QDateTime();
    m_rxBytes = 0;
    m_rxFrames = 0;
    if (m_recordModel)
        m_recordModel->clear();
    Q_EMIT statisticsChanged();
}

void ConsoleController::appendReceivedBytes(const QByteArray& data)
{
    if (data.isEmpty())
        return;

    m_rxBytes += static_cast<qulonglong>(data.size());
    Q_EMIT statisticsChanged();

    if (m_pendingData.isEmpty())
        m_pendingTimestamp = QDateTime::currentDateTime();

    for (char byte : data) {
        m_pendingData.append(byte);
        if (byte == '\n') {
            const QByteArray recordData = m_pendingData;
            const QDateTime timestamp = m_pendingTimestamp;
            m_pendingData.clear();
            m_pendingTimestamp = QDateTime();
            commitRecord(recordData, timestamp);
        }
    }

    if (!m_pendingData.isEmpty())
        m_flushTimer.start();
}

void ConsoleController::flushReceivedData()
{
    if (m_pendingData.isEmpty())
        return;

    const QByteArray recordData = m_pendingData;
    const QDateTime timestamp = m_pendingTimestamp.isValid() ? m_pendingTimestamp : QDateTime::currentDateTime();
    m_pendingData.clear();
    m_pendingTimestamp = QDateTime();
    commitRecord(recordData, timestamp);
}

void ConsoleController::commitRecord(const QByteArray& data, const QDateTime& timestamp)
{
    if (data.isEmpty())
        return;

    ++m_rxFrames;
    if (!m_receivePaused && m_recordModel) {
        const QString text = TextCodec::decodeText(data).trimmed();
        m_recordModel->appendReceive(timestamp, data, text, TextCodec::formatHex(data));
    }
    Q_EMIT statisticsChanged();
}
