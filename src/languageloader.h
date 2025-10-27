#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QMap>

struct LanguageStyle {
    QString color;
    bool bold = false;
    bool italic = false;
};

struct LanguageDefinition {
    QString name;
    QString displayName;
    QStringList fileExtensions;

    QMap<QString, QString> colors;      // Light theme colors
    QMap<QString, QString> darkColors;  // Dark theme colors
    QMap<QString, LanguageStyle> styles;
    QMap<QString, QStringList> patterns;

    // Multiline comment support
    QString multilineCommentStart;
    QString multilineCommentEnd;

    bool isValid() const { return !name.isEmpty(); }
};

struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
    QString category;
};

class LanguageLoader
{
public:
    LanguageLoader();

    // Load all language definitions from the languages directory
    bool loadLanguages(const QString &languagesDir = "languages");

    // Get list of available languages
    QStringList getAvailableLanguages() const;

    // Get language definition by name
    LanguageDefinition getLanguageDefinition(const QString &languageName) const;

    // Create highlighting rules from a language definition
    QVector<HighlightingRule> createHighlightingRules(const LanguageDefinition &langDef, bool useDarkTheme = false) const;

    // Auto-detect language from file extension
    QString detectLanguageFromExtension(const QString &filename) const;

private:
    QMap<QString, LanguageDefinition> languages;

    LanguageDefinition loadLanguageFromFile(const QString &filePath) const;
    QTextCharFormat createTextFormat(const QString &category, const LanguageDefinition &langDef, bool useDarkTheme = false) const;
    void processPatternCategory(const QString &category, const QStringList &patterns,
                              const LanguageDefinition &langDef, QVector<HighlightingRule> &rules, bool useDarkTheme = false) const;
};

#endif // LANGUAGELOADER_H