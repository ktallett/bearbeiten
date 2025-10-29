#include "characterinspector.h"
#include <QPushButton>
#include <QByteArray>

CharacterInspector::CharacterInspector(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle(tr("Character Inspector"));
    setModal(false);
    resize(400, 450);
}

void CharacterInspector::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // Character display (large)
    characterLabel = new QLabel(this);
    QFont largeFont;
    largeFont.setPointSize(48);
    characterLabel->setFont(largeFont);
    characterLabel->setAlignment(Qt::AlignCenter);
    characterLabel->setMinimumHeight(80);
    characterLabel->setStyleSheet("QLabel { background-color: palette(base); border: 1px solid palette(mid); border-radius: 4px; padding: 10px; }");
    mainLayout->addWidget(characterLabel);

    // Unicode information group
    QGroupBox *unicodeGroup = new QGroupBox(tr("Unicode Information"), this);
    QVBoxLayout *unicodeLayout = new QVBoxLayout(unicodeGroup);

    codepointLabel = new QLabel(this);
    decimalLabel = new QLabel(this);
    categoryLabel = new QLabel(this);
    descriptionLabel = new QLabel(this);
    descriptionLabel->setWordWrap(true);

    unicodeLayout->addWidget(codepointLabel);
    unicodeLayout->addWidget(decimalLabel);
    unicodeLayout->addWidget(categoryLabel);
    unicodeLayout->addWidget(descriptionLabel);

    mainLayout->addWidget(unicodeGroup);

    // Encoding information group
    QGroupBox *encodingGroup = new QGroupBox(tr("Byte Representation"), this);
    QVBoxLayout *encodingLayout = new QVBoxLayout(encodingGroup);

    utf8Label = new QLabel(this);
    utf16Label = new QLabel(this);
    utf32Label = new QLabel(this);

    utf8Label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    utf16Label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    utf32Label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    encodingLayout->addWidget(utf8Label);
    encodingLayout->addWidget(utf16Label);
    encodingLayout->addWidget(utf32Label);

    mainLayout->addWidget(encodingGroup);

    // Properties group
    QGroupBox *propertiesGroup = new QGroupBox(tr("Properties"), this);
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesGroup);

    propertiesLabel = new QLabel(this);
    propertiesLabel->setWordWrap(true);
    propertiesLayout->addWidget(propertiesLabel);

    mainLayout->addWidget(propertiesGroup);

    // Close button
    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton);

    setLayout(mainLayout);
}

void CharacterInspector::inspectCharacter(QChar ch)
{
    // Display the character
    if (ch.isPrint()) {
        characterLabel->setText(QString(ch));
    } else {
        characterLabel->setText(tr("(Non-printable)"));
    }

    // Codepoint
    codepointLabel->setText(tr("<b>Codepoint:</b> U+%1")
        .arg(uint(ch.unicode()), 4, 16, QChar('0')).toUpper());

    // Decimal value
    decimalLabel->setText(tr("<b>Decimal:</b> %1").arg(uint(ch.unicode())));

    // Category
    QString category = getCategoryName(ch.category());
    categoryLabel->setText(tr("<b>Category:</b> %1").arg(category));

    // Description
    QString description = getCharacterDescription(ch);
    descriptionLabel->setText(tr("<b>Description:</b> %1").arg(description));

    // Byte representations
    utf8Label->setText(tr("<b>UTF-8:</b> %1").arg(getUTF8Representation(ch)));
    utf16Label->setText(tr("<b>UTF-16:</b> %1").arg(getUTF16Representation(ch)));
    utf32Label->setText(tr("<b>UTF-32:</b> %1").arg(getUTF32Representation(ch)));

    // Properties
    QStringList properties;
    if (ch.isDigit()) properties << "Digit";
    if (ch.isLetter()) properties << "Letter";
    if (ch.isLower()) properties << "Lowercase";
    if (ch.isUpper()) properties << "Uppercase";
    if (ch.isSpace()) properties << "Whitespace";
    if (ch.isPunct()) properties << "Punctuation";
    if (ch.isSymbol()) properties << "Symbol";
    if (ch.isMark()) properties << "Mark";

    propertiesLabel->setText(properties.isEmpty() ? tr("None") : properties.join(", "));
}

void CharacterInspector::inspectCharacterAtPosition(const QString &text, int position)
{
    if (position < 0 || position >= text.length()) {
        characterLabel->setText(tr("(No character)"));
        codepointLabel->setText(tr("<b>Codepoint:</b> N/A"));
        decimalLabel->setText(tr("<b>Decimal:</b> N/A"));
        categoryLabel->setText(tr("<b>Category:</b> N/A"));
        descriptionLabel->setText(tr("<b>Description:</b> Position out of range"));
        utf8Label->setText(tr("<b>UTF-8:</b> N/A"));
        utf16Label->setText(tr("<b>UTF-16:</b> N/A"));
        utf32Label->setText(tr("<b>UTF-32:</b> N/A"));
        propertiesLabel->setText(tr("N/A"));
        return;
    }

    QChar ch = text.at(position);
    inspectCharacter(ch);
}

QString CharacterInspector::getCategoryName(QChar::Category category)
{
    switch (category) {
        case QChar::Mark_NonSpacing: return "Mark, Non-Spacing";
        case QChar::Mark_SpacingCombining: return "Mark, Spacing Combining";
        case QChar::Mark_Enclosing: return "Mark, Enclosing";
        case QChar::Number_DecimalDigit: return "Number, Decimal Digit";
        case QChar::Number_Letter: return "Number, Letter";
        case QChar::Number_Other: return "Number, Other";
        case QChar::Separator_Space: return "Separator, Space";
        case QChar::Separator_Line: return "Separator, Line";
        case QChar::Separator_Paragraph: return "Separator, Paragraph";
        case QChar::Other_Control: return "Other, Control";
        case QChar::Other_Format: return "Other, Format";
        case QChar::Other_Surrogate: return "Other, Surrogate";
        case QChar::Other_PrivateUse: return "Other, Private Use";
        case QChar::Other_NotAssigned: return "Other, Not Assigned";
        case QChar::Letter_Uppercase: return "Letter, Uppercase";
        case QChar::Letter_Lowercase: return "Letter, Lowercase";
        case QChar::Letter_Titlecase: return "Letter, Titlecase";
        case QChar::Letter_Modifier: return "Letter, Modifier";
        case QChar::Letter_Other: return "Letter, Other";
        case QChar::Punctuation_Connector: return "Punctuation, Connector";
        case QChar::Punctuation_Dash: return "Punctuation, Dash";
        case QChar::Punctuation_Open: return "Punctuation, Open";
        case QChar::Punctuation_Close: return "Punctuation, Close";
        case QChar::Punctuation_InitialQuote: return "Punctuation, Initial Quote";
        case QChar::Punctuation_FinalQuote: return "Punctuation, Final Quote";
        case QChar::Punctuation_Other: return "Punctuation, Other";
        case QChar::Symbol_Math: return "Symbol, Math";
        case QChar::Symbol_Currency: return "Symbol, Currency";
        case QChar::Symbol_Modifier: return "Symbol, Modifier";
        case QChar::Symbol_Other: return "Symbol, Other";
        default: return "Unknown";
    }
}

QString CharacterInspector::getCharacterDescription(QChar ch)
{
    // Provide descriptions for common characters
    uint code = uint(ch.unicode());

    // ASCII printable range
    if (code >= 32 && code <= 126) {
        if (ch.isLetter()) return tr("Latin letter '%1'").arg(ch);
        if (ch.isDigit()) return tr("Digit '%1'").arg(ch);
    }

    // Common whitespace
    if (code == 0x0020) return "SPACE";
    if (code == 0x0009) return "CHARACTER TABULATION (Tab)";
    if (code == 0x000A) return "LINE FEED (LF)";
    if (code == 0x000D) return "CARRIAGE RETURN (CR)";
    if (code == 0x00A0) return "NO-BREAK SPACE";

    // Common punctuation
    if (code == 0x0021) return "EXCLAMATION MARK";
    if (code == 0x003F) return "QUESTION MARK";
    if (code == 0x002E) return "FULL STOP";
    if (code == 0x002C) return "COMMA";
    if (code == 0x003B) return "SEMICOLON";
    if (code == 0x003A) return "COLON";

    // Common symbols
    if (code == 0x0024) return "DOLLAR SIGN";
    if (code == 0x00A3) return "POUND SIGN";
    if (code == 0x20AC) return "EURO SIGN";
    if (code == 0x00A9) return "COPYRIGHT SIGN";
    if (code == 0x00AE) return "REGISTERED SIGN";

    // Fallback based on category
    if (ch.isLetter()) {
        if (ch.isUpper()) return tr("Uppercase letter");
        if (ch.isLower()) return tr("Lowercase letter");
        return tr("Letter character");
    }
    if (ch.isDigit()) return tr("Numeric digit");
    if (ch.isPunct()) return tr("Punctuation character");
    if (ch.isSymbol()) return tr("Symbol character");
    if (ch.isSpace()) return tr("Whitespace character");
    if (ch.category() == QChar::Other_Control) return tr("Control character");

    return tr("Unicode character");
}

QString CharacterInspector::getUTF8Representation(QChar ch)
{
    QString str(ch);
    QByteArray utf8 = str.toUtf8();

    QStringList bytes;
    for (int i = 0; i < utf8.size(); ++i) {
        bytes << QString("0x%1").arg((unsigned char)utf8.at(i), 2, 16, QChar('0')).toUpper();
    }

    return bytes.join(" ");
}

QString CharacterInspector::getUTF16Representation(QChar ch)
{
    uint code = uint(ch.unicode());
    return QString("0x%1").arg(code, 4, 16, QChar('0')).toUpper();
}

QString CharacterInspector::getUTF32Representation(QChar ch)
{
    uint code = uint(ch.unicode());
    return QString("0x%1").arg(code, 8, 16, QChar('0')).toUpper();
}
