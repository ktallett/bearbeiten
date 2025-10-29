#ifndef ENCODINGMANAGER_H
#define ENCODINGMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QByteArray>

/**
 * @brief Manages text encoding detection, conversion, and compatibility checking
 *
 * The EncodingManager provides:
 * - Encoding detection for loaded files
 * - Conversion between different encodings
 * - Character compatibility checking for target encodings
 * - List of supported encodings
 */
class EncodingManager
{
public:
    enum class Encoding {
        UTF8,
        UTF16LE,
        UTF16BE,
        UTF32LE,
        UTF32BE,
        ISO_8859_1,   // Latin-1
        ISO_8859_15,  // Latin-9 (with Euro)
        Windows_1252, // Windows Latin-1
        ASCII,
        Unknown
    };

    EncodingManager();

    /**
     * @brief Detect encoding from byte array
     * @param data The raw file data
     * @return Detected encoding
     */
    static Encoding detectEncoding(const QByteArray &data);

    /**
     * @brief Get human-readable name for encoding
     * @param encoding The encoding to name
     * @return Display name (e.g., "UTF-8", "ISO-8859-1")
     */
    static QString encodingName(Encoding encoding);

    /**
     * @brief Get list of all supported encodings
     * @return List of encoding names
     */
    static QStringList supportedEncodings();

    /**
     * @brief Convert encoding name to enum
     * @param name The encoding name
     * @return Corresponding encoding enum
     */
    static Encoding encodingFromName(const QString &name);

    /**
     * @brief Decode byte array using specified encoding
     * @param data The raw data
     * @param encoding The encoding to use
     * @return Decoded text
     */
    static QString decode(const QByteArray &data, Encoding encoding);

    /**
     * @brief Encode text using specified encoding
     * @param text The text to encode
     * @param encoding The target encoding
     * @param lossy If true, replace incompatible characters; if false, return empty on error
     * @return Encoded byte array (empty if lossy=false and incompatible characters found)
     */
    static QByteArray encode(const QString &text, Encoding encoding, bool lossy = false);

    /**
     * @brief Check if text can be encoded in target encoding without loss
     * @param text The text to check
     * @param encoding The target encoding
     * @return True if all characters are compatible
     */
    static bool isCompatible(const QString &text, Encoding encoding);

    /**
     * @brief Find incompatible characters for target encoding
     * @param text The text to check
     * @param encoding The target encoding
     * @return List of incompatible characters with positions
     */
    static QList<QPair<int, QChar>> findIncompatibleCharacters(const QString &text, Encoding encoding);

    /**
     * @brief Check if encoding has BOM (Byte Order Mark)
     * @param data The raw file data
     * @return True if BOM detected
     */
    static bool hasBOM(const QByteArray &data);

    /**
     * @brief Get BOM for encoding
     * @param encoding The encoding
     * @return BOM bytes (empty if encoding doesn't use BOM)
     */
    static QByteArray getBOM(Encoding encoding);

private:
    static bool isUTF8(const QByteArray &data);
    static bool isASCII(const QByteArray &data);
};

#endif // ENCODINGMANAGER_H
