#ifndef SYMBOLEXTRACTOR_H
#define SYMBOLEXTRACTOR_H

#include <QString>
#include <QList>
#include <QRegularExpression>
#include "symbolsearchdialog.h"

/**
 * @brief Service class for extracting symbols from source code
 *
 * This class provides language-agnostic symbol extraction from source code.
 * It supports multiple programming languages including C/C++, Python, JavaScript,
 * TypeScript, Rust, and Markdown.
 */
class SymbolExtractor
{
public:
    SymbolExtractor();

    /**
     * @brief Extract symbols from source code text
     * @param documentText The source code to analyze
     * @return List of extracted symbols with their metadata
     */
    QList<SymbolInfo> extractSymbols(const QString &documentText);

private:
    void initializePatterns();

    // C/C++ patterns
    QRegularExpression functionPattern;
    QRegularExpression classPattern;
    QRegularExpression structPattern;

    // Python patterns
    QRegularExpression pythonFunctionPattern;
    QRegularExpression pythonClassPattern;

    // JavaScript patterns
    QRegularExpression jsFunctionPattern;
    QRegularExpression jsClassPattern;
    QRegularExpression jsArrowFunctionPattern;

    // Rust patterns
    QRegularExpression rustFunctionPattern;
    QRegularExpression rustStructPattern;
    QRegularExpression rustEnumPattern;
    QRegularExpression rustTraitPattern;
    QRegularExpression rustImplPattern;

    // TypeScript patterns
    QRegularExpression tsFunctionPattern;
    QRegularExpression tsClassPattern;
    QRegularExpression tsInterfacePattern;
    QRegularExpression tsTypePattern;
    QRegularExpression tsEnumPattern;
    QRegularExpression tsArrowFunctionPattern;

    // Markdown patterns
    QRegularExpression markdownHeaderPattern;
};

#endif // SYMBOLEXTRACTOR_H
