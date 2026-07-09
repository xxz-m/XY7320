/**
 * @file console_record_model.cpp
 * @brief ConsoleRecordModel implementation.
 */

#include "console_record_model.h"

#include <QColor>
#include <QRegularExpression>
#include <QStringList>
#include <QtGlobal>

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

QString directionIcon(const QString& direction)
{
    if (direction == QLatin1String("rx"))
        return QStringLiteral("←");
    if (direction == QLatin1String("tx"))
        return QStringLiteral("→");
    return QStringLiteral("•");
}

QString escapeHtml(QString text)
{
    return text.replace(QLatin1Char('&'), QStringLiteral("&amp;"))
               .replace(QLatin1Char('<'), QStringLiteral("&lt;"))
               .replace(QLatin1Char('>'), QStringLiteral("&gt;"))
               .replace(QLatin1Char('"'), QStringLiteral("&quot;"))
               .replace(QLatin1Char('\''), QStringLiteral("&#39;"));
}

QString highlightNumbers(const QString& escapedText)
{
    QString result;
    result.reserve(escapedText.size() + 64);
    bool inNumber = false;

    for (const QChar ch : escapedText) {
        const bool isDigit = ch.isDigit();
        if (isDigit && !inNumber) {
            result += QStringLiteral("<span class=\"num\">");
            inNumber = true;
        } else if (!isDigit && inNumber) {
            result += QStringLiteral("</span>");
            inNumber = false;
        }
        result += ch;
    }

    if (inNumber)
        result += QStringLiteral("</span>");
    return result;
}

QStringList splitKeywords(const QString& keywordText)
{
    QStringList result;
    const QStringList parts = keywordText.split(QLatin1Char(','));
    for (const QString& part : parts) {
        const QString trimmed = part.trimmed();
        if (!trimmed.isEmpty())
            result.append(trimmed);
    }
    return result;
}
}

ConsoleRecordModel::ConsoleRecordModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_flushTimer.setSingleShot(true);
    m_flushTimer.setInterval(60);
    connect(&m_flushTimer, &QTimer::timeout, this, &ConsoleRecordModel::flushPending);
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
        return record.timestamp.toString(m_timeFormat);
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

QString ConsoleRecordModel::displayText() const
{
    return m_displayText;
}

QString ConsoleRecordModel::displayRichText() const
{
    return m_displayRichText;
}

QString ConsoleRecordModel::pendingRichText() const
{
    return m_pendingRichText;
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

void ConsoleRecordModel::setTimeFormat(const QString& timeFormat)
{
    const QString nextFormat = timeFormat.isEmpty() ? QStringLiteral("HH:mm:ss.zzz") : timeFormat;
    if (m_timeFormat == nextFormat)
        return;
    m_timeFormat = nextFormat;
    if (!m_records.isEmpty()) {
        Q_EMIT dataChanged(index(0, 0), index(m_records.size() - 1, 0), {RecordTimeRole});
        m_flushTimer.stop();
        m_pendingRichText.clear();
        rebuildDisplayText();
        Q_EMIT displayRichTextChanged();
    }
}

void ConsoleRecordModel::setDisplayFilter(bool enabled,
                                          const QString& keywordText,
                                          bool caseSensitive,
                                          bool regexEnabled)
{
    const QStringList nextKeywords = splitKeywords(keywordText);
    if (m_filterEnabled == enabled &&
        m_filterKeywords == nextKeywords &&
        m_filterCaseSensitive == caseSensitive &&
        m_filterRegexEnabled == regexEnabled) {
        return;
    }

    m_filterEnabled = enabled;
    m_filterKeywords = nextKeywords;
    m_filterCaseSensitive = caseSensitive;
    m_filterRegexEnabled = regexEnabled;
    m_flushTimer.stop();
    m_pendingRichText.clear();
    rebuildDisplayText();
    Q_EMIT displayRichTextChanged();
}

void ConsoleRecordModel::clear()
{
    if (m_records.isEmpty())
        return;
    m_flushTimer.stop();
    beginResetModel();
    m_records.clear();
    m_displayText.clear();
    m_displayRichText.clear();
    m_pendingRichText.clear();
    endResetModel();
    Q_EMIT displayTextChanged();
    Q_EMIT displayRichTextChanged();
    Q_EMIT pendingRichTextChanged();
}

void ConsoleRecordModel::appendRecord(ConsoleRecord record)
{
    record.id = m_nextId++;
    if (!record.timestamp.isValid())
        record.timestamp = QDateTime::currentDateTime();
    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(std::move(record));
    endInsertRows();
    enforceLimit();
    scheduleFlush();
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

void ConsoleRecordModel::rebuildDisplayText()
{
    QStringList plainLines;
    QStringList richLines;
    plainLines.reserve(m_records.size());
    richLines.reserve(m_records.size());

    for (const ConsoleRecord& record : std::as_const(m_records)) {
        const QString direction = record.direction.toUpper();
        const QString time = record.timestamp.toString(m_timeFormat);
        const QString payload = record.text.isEmpty() ? record.hexText : record.text;

        if (!matchesFilter(payload))
            continue;

        const QString color = directionColor(record.direction, record.isError);
        const QString icon = directionIcon(record.direction);

        plainLines.append(QStringLiteral("[%1] %2 %3").arg(time, direction, payload));
        richLines.append(QStringLiteral("<span class=\"time\">[%1]</span> "
                                        "<span style=\"color:%2; font-weight:700;\">%3 %4</span> "
                                        "<span>%5</span>")
                             .arg(escapeHtml(time),
                                  color,
                                  escapeHtml(icon),
                                  escapeHtml(direction),
                                  highlightNumbers(escapeHtml(payload))));
    }

    m_displayText = plainLines.join(QLatin1Char('\n'));
    m_displayRichText = richLines.join(QStringLiteral("<br/>"));
    Q_EMIT displayTextChanged();
}

QString ConsoleRecordModel::formatRichLine(const ConsoleRecord& record) const
{
    const QString direction = record.direction.toUpper();
    const QString time = record.timestamp.toString(m_timeFormat);
    const QString payload = record.text.isEmpty() ? record.hexText : record.text;
    const QString color = directionColor(record.direction, record.isError);
    const QString icon = directionIcon(record.direction);

    return QStringLiteral("<span class=\"time\">[%1]</span> "
                          "<span style=\"color:%2; font-weight:700;\">%3 %4</span> "
                          "<span>%5</span>")
        .arg(escapeHtml(time),
             color,
             escapeHtml(icon),
             escapeHtml(direction),
             highlightNumbers(escapeHtml(payload)));
}

bool ConsoleRecordModel::matchesFilter(const QString& payload) const
{
    if (!m_filterEnabled || m_filterKeywords.isEmpty())
        return true;
    for (const QString& keyword : m_filterKeywords) {
        if (m_filterRegexEnabled) {
            QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
            if (!m_filterCaseSensitive)
                options |= QRegularExpression::CaseInsensitiveOption;
            const QRegularExpression regex(keyword, options);
            if (regex.isValid() && regex.match(payload).hasMatch())
                return true;
        } else if (payload.contains(keyword, m_filterCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void ConsoleRecordModel::scheduleFlush()
{
    if (!m_flushTimer.isActive())
        m_flushTimer.start();
}

void ConsoleRecordModel::flushPending()
{
    if (m_records.isEmpty()) {
        m_pendingRichText.clear();
        Q_EMIT pendingRichTextChanged();
        return;
    }

    const int firstDirty = qMax(0, m_records.size() - 64); // 兜底窗口：最多重建 64 行
    QStringList pending;
    pending.reserve(m_records.size() - firstDirty);
    for (int i = firstDirty; i < m_records.size(); ++i) {
        const ConsoleRecord& record = m_records.at(i);
        const QString payload = record.text.isEmpty() ? record.hexText : record.text;
        if (!matchesFilter(payload))
            continue;
        pending.append(formatRichLine(record));
    }

    // 始终追加显示尾段（双缓冲：保留全量副本，仅发送增量尾巴给 QML）
    const QString delta = pending.join(QStringLiteral("<br/>"));
    if (delta.isEmpty()) {
        m_pendingRichText.clear();
    } else {
        m_pendingRichText = delta;
    }
    Q_EMIT pendingRichTextChanged();
}
