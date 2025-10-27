#ifndef JSONSYNTAXHIGHLIGHTER_H
#define JSONSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>
#include <QTextCharFormat>
#include "languageloader.h"

class JsonSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JsonSyntaxHighlighter(QTextDocument *parent = nullptr);

    // Load available languages
    bool loadLanguages(const QString &languagesDir = "languages");

    // Get list of available languages
    QStringList getAvailableLanguages() const;

    // Set current language by name
    void setLanguage(const QString &languageName);

    // Auto-detect and set language from filename
    void setLanguageFromFilename(const QString &filename);

    // Get current language name
    QString getCurrentLanguage() const;

    // Set theme (true for dark, false for light)
    void setTheme(bool isDark);

    // Get current theme
    bool isDarkTheme() const { return useDarkTheme; }

protected:
    void highlightBlock(const QString &text) override;

private:
    LanguageLoader languageLoader;
    QString currentLanguageName;
    LanguageDefinition currentLanguage;
    QVector<HighlightingRule> highlightingRules;
    bool useDarkTheme;

    void updateHighlightingRules();
    void highlightMultilineComments(const QString &text);
};

#endif // JSONSYNTAXHIGHLIGHTER_H