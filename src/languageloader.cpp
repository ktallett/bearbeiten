#include "languageloader.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QColor>

LanguageLoader::LanguageLoader()
{
}

bool LanguageLoader::loadLanguages(const QString &languagesDir)
{
    languages.clear();

    QDir dir(languagesDir);
    if (!dir.exists()) {
        qWarning() << "Languages directory does not exist:" << languagesDir;
        return false;
    }

    QStringList jsonFiles = dir.entryList(QStringList() << "*.json", QDir::Files);

    if (jsonFiles.isEmpty()) {
        qWarning() << "No JSON language files found in:" << languagesDir;
        return false;
    }

    for (const QString &fileName : jsonFiles) {
        QString filePath = dir.absoluteFilePath(fileName);
        LanguageDefinition langDef = loadLanguageFromFile(filePath);

        if (langDef.isValid()) {
            languages[langDef.name.toLower()] = langDef;
            qDebug() << "Loaded language:" << langDef.displayName;
        } else {
            qWarning() << "Failed to load language from:" << fileName;
        }
    }

    return !languages.isEmpty();
}

QStringList LanguageLoader::getAvailableLanguages() const
{
    QStringList result;
    for (auto it = languages.begin(); it != languages.end(); ++it) {
        result << it.value().displayName;
    }
    result.sort();
    return result;
}

LanguageDefinition LanguageLoader::getLanguageDefinition(const QString &languageName) const
{
    return languages.value(languageName.toLower());
}

LanguageDefinition LanguageLoader::loadLanguageFromFile(const QString &filePath) const
{
    LanguageDefinition langDef;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open language file:" << filePath;
        return langDef;
    }

    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in" << filePath << ":" << error.errorString();
        return langDef;
    }

    QJsonObject root = doc.object();

    // Basic info
    langDef.name = root["name"].toString();
    langDef.displayName = root["displayName"].toString();

    // File extensions
    QJsonArray extensions = root["fileExtensions"].toArray();
    for (const auto &ext : extensions) {
        langDef.fileExtensions << ext.toString();
    }

    // Colors
    QJsonObject colors = root["colors"].toObject();
    for (auto it = colors.begin(); it != colors.end(); ++it) {
        langDef.colors[it.key()] = it.value().toString();
    }

    // Styles
    QJsonObject styles = root["styles"].toObject();
    for (auto it = styles.begin(); it != styles.end(); ++it) {
        QJsonObject styleObj = it.value().toObject();
        LanguageStyle style;
        style.bold = styleObj["bold"].toBool();
        style.italic = styleObj["italic"].toBool();
        langDef.styles[it.key()] = style;
    }

    // Patterns
    QJsonObject patterns = root["patterns"].toObject();
    for (auto it = patterns.begin(); it != patterns.end(); ++it) {
        QJsonArray patternArray = it.value().toArray();
        QStringList patternList;
        for (const auto &pattern : patternArray) {
            patternList << pattern.toString();
        }
        langDef.patterns[it.key()] = patternList;
    }

    // Multiline comments
    QJsonObject multilineComments = root["multilineComments"].toObject();
    if (!multilineComments.isEmpty()) {
        langDef.multilineCommentStart = multilineComments["start"].toString();
        langDef.multilineCommentEnd = multilineComments["end"].toString();
    }

    return langDef;
}

QVector<HighlightingRule> LanguageLoader::createHighlightingRules(const LanguageDefinition &langDef) const
{
    QVector<HighlightingRule> rules;

    // Process each pattern category
    for (auto it = langDef.patterns.begin(); it != langDef.patterns.end(); ++it) {
        const QString &category = it.key();
        const QStringList &patterns = it.value();
        processPatternCategory(category, patterns, langDef, rules);
    }

    return rules;
}

void LanguageLoader::processPatternCategory(const QString &category, const QStringList &patterns,
                                          const LanguageDefinition &langDef, QVector<HighlightingRule> &rules) const
{
    QTextCharFormat format = createTextFormat(category, langDef);

    for (const QString &pattern : patterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = format;
        rule.category = category;
        rules.append(rule);
    }
}

QTextCharFormat LanguageLoader::createTextFormat(const QString &category, const LanguageDefinition &langDef) const
{
    QTextCharFormat format;

    // Set color
    QString colorStr = langDef.colors.value(category);
    if (!colorStr.isEmpty()) {
        QColor color(colorStr);
        if (color.isValid()) {
            format.setForeground(color);
        }
    }

    // Set style
    LanguageStyle style = langDef.styles.value(category);
    if (style.bold) {
        format.setFontWeight(QFont::Bold);
    }
    if (style.italic) {
        format.setFontItalic(true);
    }

    return format;
}

QString LanguageLoader::detectLanguageFromExtension(const QString &filename) const
{
    QFileInfo fileInfo(filename);
    QString extension = "." + fileInfo.suffix().toLower();

    for (auto it = languages.begin(); it != languages.end(); ++it) {
        const LanguageDefinition &langDef = it.value();
        if (langDef.fileExtensions.contains(extension, Qt::CaseInsensitive)) {
            return langDef.name;
        }
    }

    return QString(); // No language found
}