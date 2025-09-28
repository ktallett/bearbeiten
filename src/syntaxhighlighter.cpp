#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), currentLanguage(None)
{
    setupFormats();
}

void SyntaxHighlighter::setLanguage(Language language)
{
    currentLanguage = language;
    highlightingRules.clear();
    setupRules(language);
    rehighlight();
}

void SyntaxHighlighter::setupFormats()
{
    keywordFormat.setForeground(QColor(0, 0, 255));
    keywordFormat.setFontWeight(QFont::Bold);

    classFormat.setForeground(QColor(128, 0, 128));
    classFormat.setFontWeight(QFont::Bold);

    singleLineCommentFormat.setForeground(QColor(0, 128, 0));
    singleLineCommentFormat.setFontItalic(true);

    multiLineCommentFormat.setForeground(QColor(0, 128, 0));
    multiLineCommentFormat.setFontItalic(true);

    quotationFormat.setForeground(QColor(163, 21, 21));

    functionFormat.setForeground(QColor(0, 0, 139));
    functionFormat.setFontWeight(QFont::Bold);

    numberFormat.setForeground(QColor(255, 140, 0));

    operatorFormat.setForeground(QColor(139, 0, 0));
    operatorFormat.setFontWeight(QFont::Bold);

    preprocessorFormat.setForeground(QColor(128, 128, 128));
    preprocessorFormat.setFontWeight(QFont::Bold);

    htmlTagFormat.setForeground(QColor(0, 0, 255));
    htmlTagFormat.setFontWeight(QFont::Bold);

    htmlAttributeFormat.setForeground(QColor(255, 0, 0));

    htmlValueFormat.setForeground(QColor(163, 21, 21));

    cssPropertyFormat.setForeground(QColor(0, 128, 128));
    cssPropertyFormat.setFontWeight(QFont::Bold);

    cssSelectorFormat.setForeground(QColor(128, 0, 128));
    cssSelectorFormat.setFontWeight(QFont::Bold);
}

void SyntaxHighlighter::setupRules(Language language)
{
    switch (language) {
    case HTML:
        setupHtmlRules();
        break;
    case Python:
        setupPythonRules();
        break;
    case Julia:
        setupJuliaRules();
        break;
    case CSS:
        setupCssRules();
        break;
    case JavaScript:
        setupJavaScriptRules();
        break;
    case Haskell:
        setupHaskellRules();
        break;
    case C:
        setupCRules();
        break;
    case CPlusPlus:
        setupCPlusPlusRules();
        break;
    case Fortran:
        setupFortranRules();
        break;
    case Lisp:
        setupLispRules();
        break;
    case Rust:
        setupRustRules();
        break;
    case Go:
        setupGoRules();
        break;
    case TypeScript:
        setupTypeScriptRules();
        break;
    case Lua:
        setupLuaRules();
        break;
    case Java:
        setupJavaRules();
        break;
    case Ruby:
        setupRubyRules();
        break;
    default:
        break;
    }
}

void SyntaxHighlighter::setupHtmlRules()
{
    HighlightingRule rule;

    // HTML tags
    rule.pattern = QRegularExpression(QStringLiteral("<[!?/]?\\b[A-Za-z0-9-]+(?:\\s|>|/>)"));
    rule.format = htmlTagFormat;
    highlightingRules.append(rule);

    // HTML attributes
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9-]+(?=\\s*=)"));
    rule.format = htmlAttributeFormat;
    highlightingRules.append(rule);

    // HTML attribute values
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = htmlValueFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("<!--.*-->"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupPythonRules()
{
    HighlightingRule rule;

    // Keywords
    QStringList keywordPatterns;
    keywordPatterns << "\\bclass\\b" << "\\bdef\\b" << "\\bif\\b" << "\\belif\\b" << "\\belse\\b"
                    << "\\bfor\\b" << "\\bwhile\\b" << "\\btry\\b" << "\\bexcept\\b" << "\\bfinally\\b"
                    << "\\bwith\\b" << "\\bas\\b" << "\\bimport\\b" << "\\bfrom\\b" << "\\breturn\\b"
                    << "\\byield\\b" << "\\blambda\\b" << "\\band\\b" << "\\bor\\b" << "\\bnot\\b"
                    << "\\bin\\b" << "\\bis\\b" << "\\bTrue\\b" << "\\bFalse\\b" << "\\bNone\\b"
                    << "\\bpass\\b" << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bglobal\\b" << "\\bnonlocal\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Functions
    rule.pattern = QRegularExpression(QStringLiteral("\\bdef\\s+(\\w+)"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupJuliaRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bfunction\\b" << "\\bend\\b" << "\\bif\\b" << "\\belseif\\b" << "\\belse\\b"
                    << "\\bfor\\b" << "\\bwhile\\b" << "\\btry\\b" << "\\bcatch\\b" << "\\bfinally\\b"
                    << "\\breturn\\b" << "\\busing\\b" << "\\bimport\\b" << "\\bmodule\\b" << "\\bstruct\\b"
                    << "\\bmutable\\b" << "\\babstract\\b" << "\\bprimitive\\b" << "\\btype\\b"
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bnothing\\b" << "\\bmacro\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupCssRules()
{
    HighlightingRule rule;

    // CSS selectors
    rule.pattern = QRegularExpression(QStringLiteral("\\.[a-zA-Z][a-zA-Z0-9_-]*"));
    rule.format = cssSelectorFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("#[a-zA-Z][a-zA-Z0-9_-]*"));
    rule.format = cssSelectorFormat;
    highlightingRules.append(rule);

    // CSS properties
    rule.pattern = QRegularExpression(QStringLiteral("\\b[a-zA-Z-]+(?=\\s*:)"));
    rule.format = cssPropertyFormat;
    highlightingRules.append(rule);

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("/\\*.*\\*/"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupJavaScriptRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\blet\\b" << "\\bconst\\b" << "\\bfunction\\b" << "\\breturn\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\bdo\\b"
                    << "\\btry\\b" << "\\bcatch\\b" << "\\bfinally\\b" << "\\bthrow\\b" << "\\bnew\\b"
                    << "\\bthis\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\bnull\\b" << "\\bundefined\\b"
                    << "\\bclass\\b" << "\\bextends\\b" << "\\bimport\\b" << "\\bexport\\b" << "\\bdefault\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("`[^`]*`"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupHaskellRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bmodule\\b" << "\\bwhere\\b" << "\\bimport\\b" << "\\bdata\\b" << "\\btype\\b"
                    << "\\bnewtype\\b" << "\\bclass\\b" << "\\binstance\\b" << "\\blet\\b" << "\\bin\\b"
                    << "\\bif\\b" << "\\bthen\\b" << "\\belse\\b" << "\\bcase\\b" << "\\bof\\b"
                    << "\\bdo\\b" << "\\breturn\\b" << "\\bmdo\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("--[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupCRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bint\\b" << "\\bfloat\\b" << "\\bdouble\\b" << "\\bchar\\b" << "\\bvoid\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\bdo\\b"
                    << "\\breturn\\b" << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bswitch\\b" << "\\bcase\\b"
                    << "\\bdefault\\b" << "\\bstruct\\b" << "\\bunion\\b" << "\\benum\\b" << "\\btypedef\\b"
                    << "\\bstatic\\b" << "\\bextern\\b" << "\\bconst\\b" << "\\bvolatile\\b" << "\\bregister\\b"
                    << "\\bsizeof\\b" << "\\blong\\b" << "\\bshort\\b" << "\\bunsigned\\b" << "\\bsigned\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Preprocessor
    rule.pattern = QRegularExpression(QStringLiteral("#[a-zA-Z]+"));
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?[fFlL]?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupCPlusPlusRules()
{
    setupCRules(); // Start with C rules

    HighlightingRule rule;

    QStringList cppKeywordPatterns;
    cppKeywordPatterns << "\\bclass\\b" << "\\bpublic\\b" << "\\bprivate\\b" << "\\bprotected\\b"
                       << "\\bnamespace\\b" << "\\busing\\b" << "\\btemplate\\b" << "\\btypename\\b"
                       << "\\bvirtual\\b" << "\\boverride\\b" << "\\bfinal\\b" << "\\bnew\\b" << "\\bdelete\\b"
                       << "\\btry\\b" << "\\bcatch\\b" << "\\bthrow\\b" << "\\bauto\\b" << "\\bdecltype\\b"
                       << "\\bconstexpr\\b" << "\\bnullptr\\b" << "\\bstatic_cast\\b" << "\\bdynamic_cast\\b"
                       << "\\bconst_cast\\b" << "\\breinterpret_cast\\b";

    foreach (const QString &pattern, cppKeywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupFortranRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bprogram\\b" << "\\bsubroutine\\b" << "\\bfunction\\b" << "\\bend\\b"
                    << "\\binteger\\b" << "\\breal\\b" << "\\bdouble\\b" << "\\bcomplex\\b" << "\\blogical\\b"
                    << "\\bcharacter\\b" << "\\bif\\b" << "\\bthen\\b" << "\\belse\\b" << "\\belseif\\b"
                    << "\\bdo\\b" << "\\bwhile\\b" << "\\bselect\\b" << "\\bcase\\b" << "\\bstop\\b"
                    << "\\breturn\\b" << "\\bcall\\b" << "\\bimplicit\\b" << "\\bnone\\b" << "\\bparameter\\b"
                    << "\\bdimension\\b" << "\\bcommon\\b" << "\\bequivalence\\b" << "\\bexternal\\b"
                    << "\\bintrinsic\\b" << "\\bsave\\b" << "\\bdata\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("![^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("C[^\n]*"), QRegularExpression::CaseInsensitiveOption);
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupLispRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bdefun\\b" << "\\bdefmacro\\b" << "\\bdefvar\\b" << "\\bdefparameter\\b"
                    << "\\blet\\b" << "\\blet\\*\\b" << "\\bif\\b" << "\\bwhen\\b" << "\\bunless\\b"
                    << "\\bcond\\b" << "\\bcase\\b" << "\\bloop\\b" << "\\bdo\\b" << "\\bdotimes\\b"
                    << "\\bdolist\\b" << "\\blambda\\b" << "\\bfuncall\\b" << "\\bapply\\b"
                    << "\\bquote\\b" << "\\blist\\b" << "\\bcons\\b" << "\\bcar\\b" << "\\bcdr\\b"
                    << "\\bnull\\b" << "\\bt\\b" << "\\bnil\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral(";[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupRustRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bfn\\b" << "\\blet\\b" << "\\bmut\\b" << "\\bif\\b" << "\\belse\\b"
                    << "\\bfor\\b" << "\\bwhile\\b" << "\\bloop\\b" << "\\bmatch\\b" << "\\bstruct\\b"
                    << "\\benum\\b" << "\\bimpl\\b" << "\\btrait\\b" << "\\bmod\\b" << "\\buse\\b"
                    << "\\bpub\\b" << "\\bstatic\\b" << "\\bconst\\b" << "\\bunsafe\\b" << "\\bextern\\b"
                    << "\\breturn\\b" << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bwhere\\b" << "\\bSelf\\b"
                    << "\\bself\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\bSome\\b" << "\\bNone\\b"
                    << "\\bOk\\b" << "\\bErr\\b" << "\\bOption\\b" << "\\bResult\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupGoRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bpackage\\b" << "\\bimport\\b" << "\\bfunc\\b" << "\\bvar\\b" << "\\bconst\\b"
                    << "\\btype\\b" << "\\bstruct\\b" << "\\binterface\\b" << "\\bmap\\b" << "\\bchan\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\brange\\b" << "\\bswitch\\b"
                    << "\\bcase\\b" << "\\bdefault\\b" << "\\bselect\\b" << "\\bgo\\b" << "\\bdefer\\b"
                    << "\\breturn\\b" << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bfallthrough\\b"
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bnil\\b" << "\\biota\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("`[^`]*`"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupTypeScriptRules()
{
    setupJavaScriptRules(); // Start with JavaScript rules

    HighlightingRule rule;

    QStringList tsKeywordPatterns;
    tsKeywordPatterns << "\\binterface\\b" << "\\btype\\b" << "\\benum\\b" << "\\bnamespace\\b"
                      << "\\babstract\\b" << "\\bpublic\\b" << "\\bprivate\\b" << "\\bprotected\\b"
                      << "\\breadonly\\b" << "\\bstatic\\b" << "\\bimplements\\b" << "\\bextends\\b"
                      << "\\bstring\\b" << "\\bnumber\\b" << "\\bboolean\\b" << "\\bany\\b" << "\\bvoid\\b"
                      << "\\bnever\\b" << "\\bunknown\\b" << "\\bobject\\b";

    foreach (const QString &pattern, tsKeywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupLuaRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bfunction\\b" << "\\bend\\b" << "\\bif\\b" << "\\bthen\\b" << "\\belse\\b"
                    << "\\belseif\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\brepeat\\b" << "\\buntil\\b"
                    << "\\bdo\\b" << "\\breturn\\b" << "\\bbreak\\b" << "\\blocal\\b" << "\\band\\b"
                    << "\\bor\\b" << "\\bnot\\b" << "\\bin\\b" << "\\btrue\\b" << "\\bfalse\\b"
                    << "\\bnil\\b" << "\\brequire\\b" << "\\bmodule\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("--[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupJavaRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bpublic\\b" << "\\bprivate\\b" << "\\bprotected\\b" << "\\bstatic\\b"
                    << "\\bfinal\\b" << "\\babstract\\b" << "\\bclass\\b" << "\\binterface\\b"
                    << "\\bextends\\b" << "\\bimplements\\b" << "\\bpackage\\b" << "\\bimport\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\bdo\\b"
                    << "\\bswitch\\b" << "\\bcase\\b" << "\\bdefault\\b" << "\\btry\\b" << "\\bcatch\\b"
                    << "\\bfinally\\b" << "\\bthrow\\b" << "\\bthrows\\b" << "\\breturn\\b" << "\\bbreak\\b"
                    << "\\bcontinue\\b" << "\\bnew\\b" << "\\bthis\\b" << "\\bsuper\\b" << "\\bnull\\b"
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bint\\b" << "\\bfloat\\b" << "\\bdouble\\b"
                    << "\\bboolean\\b" << "\\bchar\\b" << "\\bbyte\\b" << "\\bshort\\b" << "\\blong\\b"
                    << "\\bvoid\\b" << "\\bString\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?[fFdDlL]?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupRubyRules()
{
    HighlightingRule rule;

    QStringList keywordPatterns;
    keywordPatterns << "\\bclass\\b" << "\\bmodule\\b" << "\\bdef\\b" << "\\bend\\b" << "\\bif\\b"
                    << "\\bunless\\b" << "\\belse\\b" << "\\belsif\\b" << "\\bfor\\b" << "\\bwhile\\b"
                    << "\\buntil\\b" << "\\bcase\\b" << "\\bwhen\\b" << "\\bthen\\b" << "\\bbegin\\b"
                    << "\\brescue\\b" << "\\bensure\\b" << "\\breturn\\b" << "\\byield\\b" << "\\bbreak\\b"
                    << "\\bnext\\b" << "\\bredo\\b" << "\\bretry\\b" << "\\bsuper\\b" << "\\bself\\b"
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bnil\\b" << "\\brequire\\b" << "\\binclude\\b"
                    << "\\bextend\\b" << "\\battr_reader\\b" << "\\battr_writer\\b" << "\\battr_accessor\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Strings
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Comments
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Numbers
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}