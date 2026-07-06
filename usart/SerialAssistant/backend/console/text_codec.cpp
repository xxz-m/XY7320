/**
 * @file text_codec.cpp
 * @brief TextCodec implementation.
 */

#include "text_codec.h"

#include <QChar>
#include <QStringConverter>
#include <QStringDecoder>
#include <QStringEncoder>

namespace {
bool isHexSeparator(QChar ch)
{
    return ch.isSpace() || ch == QLatin1Char(',') || ch == QLatin1Char(';') || ch == QLatin1Char(':')
           || ch == QLatin1Char('-');
}

int hexValue(QChar ch)
{
    if (ch >= QLatin1Char('0') && ch <= QLatin1Char('9'))
        return ch.unicode() - QLatin1Char('0').unicode();
    if (ch >= QLatin1Char('a') && ch <= QLatin1Char('f'))
        return ch.unicode() - QLatin1Char('a').unicode() + 10;
    if (ch >= QLatin1Char('A') && ch <= QLatin1Char('F'))
        return ch.unicode() - QLatin1Char('A').unicode() + 10;
    return -1;
}

QString escapeInvalidBytes(const QByteArray& data)
{
    QString text;
    text.reserve(data.size() * 4);
    for (unsigned char byte : data) {
        if (byte >= 0x20 && byte <= 0x7e) {
            text.append(QChar(byte));
        } else if (byte == '\r') {
            text.append(QStringLiteral("\\r"));
        } else if (byte == '\n') {
            text.append(QStringLiteral("\\n"));
        } else if (byte == '\t') {
            text.append(QStringLiteral("\\t"));
        } else {
            text.append(QStringLiteral("\\x%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper());
        }
    }
    return text;
}

QString decodeWithConverter(const QByteArray& data, QStringConverter::Encoding encoding, TextCodec::InvalidBytePolicy invalidBytePolicy)
{
    if (invalidBytePolicy == TextCodec::InvalidBytePolicy::Escape) {
        QStringDecoder strictDecoder(encoding, QStringConverter::Flag::ConvertInvalidToNull);
        const QString decoded = strictDecoder.decode(data);
        if (!decoded.contains(QChar::Null))
            return decoded;
        return escapeInvalidBytes(data);
    }

    QStringDecoder decoder(encoding, QStringConverter::Flag::ConvertInvalidToNull);
    QString decoded = decoder.decode(data);
    if (invalidBytePolicy == TextCodec::InvalidBytePolicy::Ignore)
        decoded.remove(QChar::Null);
    else
        decoded.replace(QChar::Null, QChar(0xfffd));
    return decoded;
}
}

TextCodec::Encoding TextCodec::encodingFromName(const QString& name)
{
    if (name.compare(QStringLiteral("GBK"), Qt::CaseInsensitive) == 0)
        return Encoding::Gbk;
    if (name.compare(QStringLiteral("ASCII"), Qt::CaseInsensitive) == 0)
        return Encoding::Ascii;
    if (name.compare(QStringLiteral("Latin-1"), Qt::CaseInsensitive) == 0
        || name.compare(QStringLiteral("Latin1"), Qt::CaseInsensitive) == 0)
        return Encoding::Latin1;
    return Encoding::Utf8;
}

QByteArray TextCodec::encodeText(const QString& text, Encoding encoding)
{
    switch (encoding) {
    case Encoding::Gbk: {
        QStringEncoder encoder(QStringConverter::System);
        return encoder.encode(text);
    }
    case Encoding::Latin1:
        return text.toLatin1();
    case Encoding::Ascii: {
        QByteArray bytes;
        bytes.reserve(text.size());
        for (QChar ch : text)
            bytes.append(ch.unicode() <= 0x7f ? static_cast<char>(ch.unicode()) : '?');
        return bytes;
    }
    case Encoding::Utf8:
    default:
        return text.toUtf8();
    }
}

QString TextCodec::decodeText(const QByteArray& data, Encoding encoding, InvalidBytePolicy invalidBytePolicy)
{
    switch (encoding) {
    case Encoding::Gbk:
        return decodeWithConverter(data, QStringConverter::System, invalidBytePolicy);
    case Encoding::Latin1:
        return QString::fromLatin1(data);
    case Encoding::Ascii: {
        QString text;
        text.reserve(data.size());
        for (unsigned char byte : data) {
            if (byte <= 0x7f) {
                text.append(QChar(byte));
            } else if (invalidBytePolicy == InvalidBytePolicy::Escape) {
                text.append(QStringLiteral("\\x%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper());
            } else if (invalidBytePolicy == InvalidBytePolicy::Replace) {
                text.append(QChar(0xfffd));
            }
        }
        return text;
    }
    case Encoding::Utf8:
    default:
        return decodeWithConverter(data, QStringConverter::Utf8, invalidBytePolicy);
    }
}

QByteArray TextCodec::parseHex(const QString& input, QString* errorText)
{
    QString cleaned;
    cleaned.reserve(input.size());

    for (int i = 0; i < input.size(); ++i) {
        const QChar ch = input.at(i);
        if (isHexSeparator(ch))
            continue;

        if (ch == QLatin1Char('0') && i + 1 < input.size()
            && (input.at(i + 1) == QLatin1Char('x') || input.at(i + 1) == QLatin1Char('X'))) {
            ++i;
            continue;
        }

        if (hexValue(ch) < 0) {
            if (errorText)
                *errorText = QStringLiteral("HEX 第 %1 个字符无效：%2").arg(i + 1).arg(ch);
            return QByteArray();
        }

        cleaned.append(ch);
    }

    if (cleaned.isEmpty()) {
        if (errorText)
            *errorText = QStringLiteral("HEX 内容为空");
        return QByteArray();
    }

    if ((cleaned.size() % 2) != 0) {
        if (errorText)
            *errorText = QStringLiteral("HEX 字符数量必须为偶数");
        return QByteArray();
    }

    QByteArray bytes;
    bytes.reserve(cleaned.size() / 2);
    for (int i = 0; i < cleaned.size(); i += 2) {
        const int high = hexValue(cleaned.at(i));
        const int low = hexValue(cleaned.at(i + 1));
        bytes.append(static_cast<char>((high << 4) | low));
    }

    if (errorText)
        errorText->clear();
    return bytes;
}

QString TextCodec::formatHex(const QByteArray& data)
{
    return QString::fromLatin1(data.toHex(' ').toUpper());
}
