#ifndef CHARACTERINSPECTOR_H
#define CHARACTERINSPECTOR_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QString>
#include <QChar>

/**
 * @brief A dialog that displays detailed information about a Unicode character
 *
 * The CharacterInspector shows:
 * - The character itself (large display)
 * - Unicode codepoint (U+XXXX format)
 * - Character category and properties
 * - UTF-8, UTF-16, and UTF-32 byte representations
 * - Decimal and hexadecimal values
 */
class CharacterInspector : public QDialog
{
    Q_OBJECT

public:
    explicit CharacterInspector(QWidget *parent = nullptr);

    /**
     * @brief Inspect and display information about a character
     * @param ch The character to inspect
     */
    void inspectCharacter(QChar ch);

    /**
     * @brief Inspect the character at the cursor position in text
     * @param text The full text
     * @param position The cursor position
     */
    void inspectCharacterAtPosition(const QString &text, int position);

private:
    void setupUI();
    QString getCharacterCategory(QChar ch);
    QString getCharacterDescription(QChar ch);
    QString getCategoryName(QChar::Category category);
    QString getUTF8Representation(QChar ch);
    QString getUTF16Representation(QChar ch);
    QString getUTF32Representation(QChar ch);

    // UI components
    QLabel *characterLabel;
    QLabel *codepointLabel;
    QLabel *categoryLabel;
    QLabel *descriptionLabel;
    QLabel *decimalLabel;
    QLabel *utf8Label;
    QLabel *utf16Label;
    QLabel *utf32Label;
    QLabel *propertiesLabel;
};

#endif // CHARACTERINSPECTOR_H
