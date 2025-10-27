#include "jsonsyntaxhighlighter.h"
#include <QDebug>

JsonSyntaxHighlighter::JsonSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), useDarkTheme(false)
{
}

bool JsonSyntaxHighlighter::loadLanguages(const QString &languagesDir)
{
    bool success = languageLoader.loadLanguages(languagesDir);
    if (success) {
        qDebug() << "Loaded languages:" << languageLoader.getAvailableLanguages();
    }
    return success;
}

QStringList JsonSyntaxHighlighter::getAvailableLanguages() const
{
    return languageLoader.getAvailableLanguages();
}

void JsonSyntaxHighlighter::setLanguage(const QString &languageName)
{
    if (languageName.isEmpty() || languageName.toLower() == "none") {
        currentLanguageName.clear();
        currentLanguage = LanguageDefinition();
        highlightingRules.clear();
        rehighlight();
        return;
    }

    LanguageDefinition langDef = languageLoader.getLanguageDefinition(languageName);
    if (langDef.isValid()) {
        currentLanguageName = languageName;
        currentLanguage = langDef;
        updateHighlightingRules();
        rehighlight();
        qDebug() << "Set language to:" << langDef.displayName;
    } else {
        qWarning() << "Language not found:" << languageName;
    }
}

void JsonSyntaxHighlighter::setLanguageFromFilename(const QString &filename)
{
    QString detectedLanguage = languageLoader.detectLanguageFromExtension(filename);
    if (!detectedLanguage.isEmpty()) {
        setLanguage(detectedLanguage);
        qDebug() << "Auto-detected language" << detectedLanguage << "for file:" << filename;
    } else {
        qDebug() << "No language detected for file:" << filename;
        setLanguage(""); // Clear highlighting
    }
}

QString JsonSyntaxHighlighter::getCurrentLanguage() const
{
    return currentLanguageName;
}

void JsonSyntaxHighlighter::updateHighlightingRules()
{
    highlightingRules = languageLoader.createHighlightingRules(currentLanguage, useDarkTheme);
    qDebug() << "Created" << highlightingRules.size() << "highlighting rules for" << currentLanguage.displayName
             << "(dark theme:" << useDarkTheme << ")";
}

void JsonSyntaxHighlighter::setTheme(bool isDark)
{
    if (useDarkTheme != isDark) {
        useDarkTheme = isDark;
        if (currentLanguage.isValid()) {
            updateHighlightingRules();
            rehighlight();
        }
    }
}

void JsonSyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply all highlighting rules
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multiline comments if defined
    highlightMultilineComments(text);
}

void JsonSyntaxHighlighter::highlightMultilineComments(const QString &text)
{
    if (currentLanguage.multilineCommentStart.isEmpty() || currentLanguage.multilineCommentEnd.isEmpty()) {
        return;
    }

    // Create format for multiline comments
    QTextCharFormat multiLineCommentFormat;
    const QMap<QString, QString> &colorMap = useDarkTheme ? currentLanguage.darkColors : currentLanguage.colors;
    QString commentColor = colorMap.value("comments", useDarkTheme ? "#6A9955" : "#008000");
    multiLineCommentFormat.setForeground(QColor(commentColor));

    LanguageStyle commentStyle = currentLanguage.styles.value("comments");
    if (commentStyle.bold) {
        multiLineCommentFormat.setFontWeight(QFont::Bold);
    }
    if (commentStyle.italic) {
        multiLineCommentFormat.setFontItalic(true);
    }

    setCurrentBlockState(0);

    QRegularExpression startExpression(QRegularExpression::escape(currentLanguage.multilineCommentStart));
    QRegularExpression endExpression(QRegularExpression::escape(currentLanguage.multilineCommentEnd));

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = endExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}