/**
 * @file text_codec.h
 * @brief Text and HEX conversion helpers for the console backend.
 */

#ifndef SERIAL_ASSISTANT_TEXT_CODEC_H
#define SERIAL_ASSISTANT_TEXT_CODEC_H

#include <QByteArray>
#include <QString>

/**
 * Converts between user-facing text/HEX and raw serial bytes.
 */
class TextCodec final
{
public:
    enum class Encoding {
        Utf8,
        Gbk,
        Latin1,
        Ascii
    };

    enum class InvalidBytePolicy {
        Replace = 0,
        Ignore = 1,
        Escape = 2
    };

    [[nodiscard]] static Encoding encodingFromName(const QString& name);
    [[nodiscard]] static QByteArray encodeText(const QString& text, Encoding encoding = Encoding::Utf8);
    [[nodiscard]] static QString decodeText(const QByteArray& data,
                                            Encoding encoding = Encoding::Utf8,
                                            InvalidBytePolicy invalidBytePolicy = InvalidBytePolicy::Replace);
    [[nodiscard]] static QByteArray parseHex(const QString& input, QString* errorText = nullptr);
    [[nodiscard]] static QString formatHex(const QByteArray& data);
};

#endif
