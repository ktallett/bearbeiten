#include "encodingmanager.h"
#include <QDebug>

EncodingManager::EncodingManager()
{
}

EncodingManager::Encoding EncodingManager::detectEncoding(const QByteArray &data)
{
    if (data.isEmpty()) {
        return Encoding::UTF8; // Default
    }

    // Check for BOM (Byte Order Mark)
    if (data.size() >= 3) {
        // UTF-8 BOM: EF BB BF
        if ((unsigned char)data[0] == 0xEF &&
            (unsigned char)data[1] == 0xBB &&
            (unsigned char)data[2] == 0xBF) {
            return Encoding::UTF8;
        }
    }

    if (data.size() >= 2) {
        // UTF-16 LE BOM: FF FE
        if ((unsigned char)data[0] == 0xFF &&
            (unsigned char)data[1] == 0xFE) {
            return Encoding::UTF16LE;
        }
        // UTF-16 BE BOM: FE FF
        if ((unsigned char)data[0] == 0xFE &&
            (unsigned char)data[1] == 0xFF) {
            return Encoding::UTF16BE;
        }
    }

    if (data.size() >= 4) {
        // UTF-32 LE BOM: FF FE 00 00
        if ((unsigned char)data[0] == 0xFF &&
            (unsigned char)data[1] == 0xFE &&
            (unsigned char)data[2] == 0x00 &&
            (unsigned char)data[3] == 0x00) {
            return Encoding::UTF32LE;
        }
        // UTF-32 BE BOM: 00 00 FE FF
        if ((unsigned char)data[0] == 0x00 &&
            (unsigned char)data[1] == 0x00 &&
            (unsigned char)data[2] == 0xFE &&
            (unsigned char)data[3] == 0xFF) {
            return Encoding::UTF32BE;
        }
    }

    // No BOM - try to detect encoding by content
    if (isASCII(data)) {
        return Encoding::ASCII;
    }

    if (isUTF8(data)) {
        return Encoding::UTF8;
    }

    // Default to ISO-8859-1 (Latin-1) as fallback for 8-bit encodings
    // This is a safe fallback since it maps all byte values to Unicode
    return Encoding::ISO_8859_1;
}

bool EncodingManager::isASCII(const QByteArray &data)
{
    for (int i = 0; i < data.size(); ++i) {
        if ((unsigned char)data[i] > 127) {
            return false;
        }
    }
    return true;
}

bool EncodingManager::isUTF8(const QByteArray &data)
{
    int i = 0;
    while (i < data.size()) {
        unsigned char c = (unsigned char)data[i];

        // ASCII character
        if (c <= 0x7F) {
            i++;
            continue;
        }

        // Multi-byte sequence
        int extraBytes = 0;
        if ((c & 0xE0) == 0xC0) {
            extraBytes = 1; // 2-byte sequence
        } else if ((c & 0xF0) == 0xE0) {
            extraBytes = 2; // 3-byte sequence
        } else if ((c & 0xF8) == 0xF0) {
            extraBytes = 3; // 4-byte sequence
        } else {
            return false; // Invalid UTF-8 start byte
        }

        // Check continuation bytes
        for (int j = 0; j < extraBytes; ++j) {
            i++;
            if (i >= data.size()) {
                return false; // Incomplete sequence
            }
            unsigned char cont = (unsigned char)data[i];
            if ((cont & 0xC0) != 0x80) {
                return false; // Invalid continuation byte
            }
        }

        i++;
    }

    return true;
}

QString EncodingManager::encodingName(Encoding encoding)
{
    switch (encoding) {
        case Encoding::UTF8: return "UTF-8";
        case Encoding::UTF16LE: return "UTF-16 LE";
        case Encoding::UTF16BE: return "UTF-16 BE";
        case Encoding::UTF32LE: return "UTF-32 LE";
        case Encoding::UTF32BE: return "UTF-32 BE";
        case Encoding::ISO_8859_1: return "ISO-8859-1";
        case Encoding::ISO_8859_15: return "ISO-8859-15";
        case Encoding::Windows_1252: return "Windows-1252";
        case Encoding::ASCII: return "ASCII";
        default: return "Unknown";
    }
}

QStringList EncodingManager::supportedEncodings()
{
    return {
        "UTF-8",
        "UTF-16 LE",
        "UTF-16 BE",
        "UTF-32 LE",
        "UTF-32 BE",
        "ISO-8859-1",
        "ISO-8859-15",
        "Windows-1252",
        "ASCII"
    };
}

EncodingManager::Encoding EncodingManager::encodingFromName(const QString &name)
{
    if (name == "UTF-8") return Encoding::UTF8;
    if (name == "UTF-16 LE") return Encoding::UTF16LE;
    if (name == "UTF-16 BE") return Encoding::UTF16BE;
    if (name == "UTF-32 LE") return Encoding::UTF32LE;
    if (name == "UTF-32 BE") return Encoding::UTF32BE;
    if (name == "ISO-8859-1") return Encoding::ISO_8859_1;
    if (name == "ISO-8859-15") return Encoding::ISO_8859_15;
    if (name == "Windows-1252") return Encoding::Windows_1252;
    if (name == "ASCII") return Encoding::ASCII;
    return Encoding::Unknown;
}

QString EncodingManager::decode(const QByteArray &data, Encoding encoding)
{
    QByteArray cleanData = data;

    // Remove BOM if present
    if (hasBOM(data)) {
        QByteArray bom = getBOM(encoding);
        if (!bom.isEmpty() && data.startsWith(bom)) {
            cleanData = data.mid(bom.size());
        }
    }

    switch (encoding) {
        case Encoding::UTF8:
            return QString::fromUtf8(cleanData);
        case Encoding::UTF16LE:
            return QString::fromUtf16(reinterpret_cast<const char16_t*>(cleanData.data()),
                                     cleanData.size() / 2);
        case Encoding::UTF16BE: {
            // Qt doesn't have direct BE support, need to swap bytes
            QByteArray swapped;
            for (int i = 0; i < cleanData.size() - 1; i += 2) {
                swapped.append(cleanData[i + 1]);
                swapped.append(cleanData[i]);
            }
            return QString::fromUtf16(reinterpret_cast<const char16_t*>(swapped.data()),
                                     swapped.size() / 2);
        }
        case Encoding::UTF32LE:
            return QString::fromUcs4(reinterpret_cast<const char32_t*>(cleanData.data()),
                                    cleanData.size() / 4);
        case Encoding::UTF32BE: {
            // Swap bytes for BE
            QByteArray swapped;
            for (int i = 0; i < cleanData.size() - 3; i += 4) {
                swapped.append(cleanData[i + 3]);
                swapped.append(cleanData[i + 2]);
                swapped.append(cleanData[i + 1]);
                swapped.append(cleanData[i]);
            }
            return QString::fromUcs4(reinterpret_cast<const char32_t*>(swapped.data()),
                                    swapped.size() / 4);
        }
        case Encoding::ISO_8859_1:
            return QString::fromLatin1(cleanData);
        case Encoding::ISO_8859_15:
        case Encoding::Windows_1252:
        case Encoding::ASCII:
            // For these, use Latin1 as approximation (works for ASCII and is close for others)
            return QString::fromLatin1(cleanData);
        default:
            return QString::fromUtf8(cleanData);
    }
}

QByteArray EncodingManager::encode(const QString &text, Encoding encoding, bool lossy)
{
    if (!lossy && !isCompatible(text, encoding)) {
        return QByteArray(); // Return empty if not compatible and lossy=false
    }

    QByteArray result;

    switch (encoding) {
        case Encoding::UTF8:
            result = text.toUtf8();
            break;
        case Encoding::UTF16LE: {
            const ushort *utf16 = text.utf16();
            result = QByteArray(reinterpret_cast<const char*>(utf16), text.size() * 2);
            break;
        }
        case Encoding::UTF16BE: {
            const ushort *utf16 = text.utf16();
            // Swap bytes for BE
            for (int i = 0; i < text.size(); ++i) {
                ushort c = utf16[i];
                result.append((c >> 8) & 0xFF);
                result.append(c & 0xFF);
            }
            break;
        }
        case Encoding::UTF32LE: {
            QVector<uint> utf32 = text.toUcs4();
            result = QByteArray(reinterpret_cast<const char*>(utf32.data()), utf32.size() * 4);
            break;
        }
        case Encoding::UTF32BE: {
            QVector<uint> utf32 = text.toUcs4();
            // Swap bytes for BE
            for (uint c : utf32) {
                result.append((c >> 24) & 0xFF);
                result.append((c >> 16) & 0xFF);
                result.append((c >> 8) & 0xFF);
                result.append(c & 0xFF);
            }
            break;
        }
        case Encoding::ISO_8859_1:
        case Encoding::ASCII:
            result = text.toLatin1();
            break;
        case Encoding::ISO_8859_15:
        case Encoding::Windows_1252:
            // Approximate with Latin1
            result = text.toLatin1();
            break;
        default:
            result = text.toUtf8();
            break;
    }

    return result;
}

bool EncodingManager::isCompatible(const QString &text, Encoding encoding)
{
    switch (encoding) {
        case Encoding::UTF8:
        case Encoding::UTF16LE:
        case Encoding::UTF16BE:
        case Encoding::UTF32LE:
        case Encoding::UTF32BE:
            return true; // UTF encodings support all Unicode characters

        case Encoding::ASCII:
            for (const QChar &ch : text) {
                if (ch.unicode() > 127) {
                    return false;
                }
            }
            return true;

        case Encoding::ISO_8859_1:
            for (const QChar &ch : text) {
                if (ch.unicode() > 255) {
                    return false;
                }
            }
            return true;

        case Encoding::ISO_8859_15:
        case Encoding::Windows_1252:
            // Simplified check - these support most Latin characters up to 255
            for (const QChar &ch : text) {
                if (ch.unicode() > 255) {
                    return false;
                }
            }
            return true;

        default:
            return false;
    }
}

QList<QPair<int, QChar>> EncodingManager::findIncompatibleCharacters(const QString &text, Encoding encoding)
{
    QList<QPair<int, QChar>> incompatible;

    for (int i = 0; i < text.length(); ++i) {
        QChar ch = text.at(i);
        uint code = ch.unicode();

        bool compatible = true;

        switch (encoding) {
            case Encoding::UTF8:
            case Encoding::UTF16LE:
            case Encoding::UTF16BE:
            case Encoding::UTF32LE:
            case Encoding::UTF32BE:
                compatible = true;
                break;

            case Encoding::ASCII:
                if (code > 127) {
                    compatible = false;
                }
                break;

            case Encoding::ISO_8859_1:
            case Encoding::ISO_8859_15:
            case Encoding::Windows_1252:
                if (code > 255) {
                    compatible = false;
                }
                break;

            default:
                compatible = false;
                break;
        }

        if (!compatible) {
            incompatible.append(qMakePair(i, ch));
        }
    }

    return incompatible;
}

bool EncodingManager::hasBOM(const QByteArray &data)
{
    if (data.size() >= 3) {
        // UTF-8 BOM
        if ((unsigned char)data[0] == 0xEF &&
            (unsigned char)data[1] == 0xBB &&
            (unsigned char)data[2] == 0xBF) {
            return true;
        }
    }

    if (data.size() >= 2) {
        // UTF-16 BOM
        if (((unsigned char)data[0] == 0xFF && (unsigned char)data[1] == 0xFE) ||
            ((unsigned char)data[0] == 0xFE && (unsigned char)data[1] == 0xFF)) {
            return true;
        }
    }

    if (data.size() >= 4) {
        // UTF-32 BOM
        if (((unsigned char)data[0] == 0xFF && (unsigned char)data[1] == 0xFE &&
             (unsigned char)data[2] == 0x00 && (unsigned char)data[3] == 0x00) ||
            ((unsigned char)data[0] == 0x00 && (unsigned char)data[1] == 0x00 &&
             (unsigned char)data[2] == 0xFE && (unsigned char)data[3] == 0xFF)) {
            return true;
        }
    }

    return false;
}

QByteArray EncodingManager::getBOM(Encoding encoding)
{
    switch (encoding) {
        case Encoding::UTF8:
            return QByteArray("\xEF\xBB\xBF", 3);
        case Encoding::UTF16LE:
            return QByteArray("\xFF\xFE", 2);
        case Encoding::UTF16BE:
            return QByteArray("\xFE\xFF", 2);
        case Encoding::UTF32LE:
            return QByteArray("\xFF\xFE\x00\x00", 4);
        case Encoding::UTF32BE:
            return QByteArray("\x00\x00\xFE\xFF", 4);
        default:
            return QByteArray(); // No BOM for other encodings
    }
}
