#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>
#include <QTextCharFormat>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    enum Language {
        None,
        HTML,
        Python,
        Julia,
        CSS,
        JavaScript,
        Haskell,
        C,
        CPlusPlus,
        Fortran,
        Lisp,
        Rust,
        Go,
        TypeScript,
        Lua,
        Java,
        Ruby
    };

    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    void setLanguage(Language language);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    void setupFormats();
    void setupRules(Language language);

    // Format definitions
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat htmlTagFormat;
    QTextCharFormat htmlAttributeFormat;
    QTextCharFormat htmlValueFormat;
    QTextCharFormat cssPropertyFormat;
    QTextCharFormat cssSelectorFormat;

    Language currentLanguage;

    // Language-specific setup methods
    void setupHtmlRules();
    void setupPythonRules();
    void setupJuliaRules();
    void setupCssRules();
    void setupJavaScriptRules();
    void setupHaskellRules();
    void setupCRules();
    void setupCPlusPlusRules();
    void setupFortranRules();
    void setupLispRules();
    void setupRustRules();
    void setupGoRules();
    void setupTypeScriptRules();
    void setupLuaRules();
    void setupJavaRules();
    void setupRubyRules();
};

#endif // SYNTAXHIGHLIGHTER_H