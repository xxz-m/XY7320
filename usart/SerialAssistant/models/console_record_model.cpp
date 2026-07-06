/**
 * @file console_record_model.cpp
 * @brief ConsoleRecordModel implementation.
 */

#include "console_record_model.h"

#include <QColor>

namespace {
QString directionColor(const QString& direction, bool isError)
{
    if (isError)
        return QStringLiteral("#ef4444");
    if (direction == QLatin1String("tx"))
        return QStringLiteral("#22c55e");
    if (direction == QLatin1String("rx"))
        return QStringLiteral("#818cf8");
    return QStringLiteral("#94a3b8");
}
}

ConsoleRecordModel::ConsoleRecordModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ConsoleRecordModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_records.size();
}

QVariant ConsoleRecordModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return QVariant();

    const ConsoleRecord& record = m_records.at(index.row());
    switch (role) {
    case IdRole:
        return QVariant::fromValue(record.id);
    case RecordTimeRole:
        return record.timestamp.toString(QStringLiteral("HH:mm:ss.zzz"));
    case DirectionRole:
        return record.direction;
    case ContentRole:
    case TextRole:
        return record.text;
    case HexTextRole:
        return record.hexText;
    case ByteCountRole:
        return record.rawData.size();
    case IsErrorRole:
        return record.isError;
    case DirectionColorRole:
        return directionColor(record.direction, record.isError);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ConsoleRecordModel::roleNames() const
{
    return {
        {IdRole, "recordId"},
        {RecordTimeRole, "recordTime"},
        {DirectionRole, "direction"},
        {ContentRole, "content"},
        {TextRole, "text"},
        {HexTextRole, "hexText"},
        {ByteCountRole, "byteCount"},
        {IsErrorRole, "isError"},
        {DirectionColorRole, "directionColor"},
    };
}

void ConsoleRecordModel::appendReceive(const QDateTime& timestamp,
                                       const QByteArray& data,
                                       const QString& text,
                                       const QString& hexText)
{
    appendRecord(ConsoleRecord{0, timestamp, data, QStringLiteral("rx"), text, hexText, false});
}

void ConsoleRecordModel::appendTransmit(const QDateTime& timestamp,
                                        const QByteArray& data,
                                        const QString& text,
                                        const QString& hexText)
{
    appendRecord(ConsoleRecord{0, timestamp, data, QStringLiteral("tx"), text, hexText, false});
}

void ConsoleRecordModel::appendSystem(const QString& message, bool isError)
{
    appendRecord(ConsoleRecord{0, QDateTime::currentDateTime(), QByteArray(), QStringLiteral("system"), message, QString(), isError});
}

void ConsoleRecordModel::clear()
{
    if (m_records.isEmpty())
        return;
    beginResetModel();
    m_records.clear();
    endResetModel();
}

void ConsoleRecordModel::appendRecord(ConsoleRecord record)
{
    record.id = m_nextId++;
    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(std::move(record));
    endInsertRows();
    enforceLimit();
}

void ConsoleRecordModel::enforceLimit()
{
    if (m_records.size() <= m_maxRecords)
        return;

    const int removeCount = m_records.size() - m_maxRecords;
    beginRemoveRows(QModelIndex(), 0, removeCount - 1);
    m_records.erase(m_records.begin(), m_records.begin() + removeCount);
    endRemoveRows();
}
