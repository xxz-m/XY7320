/**
 * @file console_controller.h
 * @brief Receive-side console backend controller.
 */

#ifndef SERIAL_ASSISTANT_CONSOLE_CONTROLLER_H
#define SERIAL_ASSISTANT_CONSOLE_CONTROLLER_H

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QTimer>

#include "../../models/console_record_model.h"

class SerialPortController;
class SettingsManager;

/**
 * Converts incoming serial bytes into bounded console records and statistics.
 */
class ConsoleController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConsoleRecordModel* records READ records CONSTANT)
    Q_PROPERTY(bool receivePaused READ receivePaused WRITE setReceivePaused NOTIFY receivePausedChanged)
    Q_PROPERTY(qulonglong rxBytes READ rxBytes NOTIFY statisticsChanged)
    Q_PROPERTY(qulonglong rxFrames READ rxFrames NOTIFY statisticsChanged)

public:
    ConsoleController(SerialPortController* serialController,
                      ConsoleRecordModel* recordModel,
                      SettingsManager* settingsManager,
                      QObject* parent = nullptr);

    [[nodiscard]] ConsoleRecordModel* records() const;
    [[nodiscard]] bool receivePaused() const;
    [[nodiscard]] qulonglong rxBytes() const;
    [[nodiscard]] qulonglong rxFrames() const;

    void setReceivePaused(bool receivePaused);

    Q_INVOKABLE void clear();

signals:
    void receivePausedChanged();
    void statisticsChanged();

private slots:
    void appendReceivedBytes(const QByteArray& data);
    void flushReceivedData();

private:
    void commitRecord(const QByteArray& data, const QDateTime& timestamp);

    ConsoleRecordModel* m_recordModel;
    SettingsManager* m_settingsManager;
    QTimer m_flushTimer;
    QByteArray m_pendingData;
    QDateTime m_pendingTimestamp;
    bool m_receivePaused = false;
    qulonglong m_rxBytes = 0;
    qulonglong m_rxFrames = 0;
};

#endif
