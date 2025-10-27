#include "symbolextractor.h"

SymbolExtractor::SymbolExtractor()
{
    initializePatterns();
}

void SymbolExtractor::initializePatterns()
{
    // C/C++ patterns
    functionPattern.setPattern(R"(([\w:]+)\s+([\w:]+)\s*\([^)]*\)\s*\{?)");
    classPattern.setPattern(R"(^\s*class\s+([\w:]+))");
    structPattern.setPattern(R"(^\s*struct\s+([\w:]+))");

    // Python patterns
    pythonFunctionPattern.setPattern(R"(^\s*def\s+([\w_]+)\s*\()");
    pythonClassPattern.setPattern(R"(^\s*class\s+([\w_]+))");

    // JavaScript patterns
    jsFunctionPattern.setPattern(R"(^\s*function\s+([\w_]+)\s*\()");
    jsClassPattern.setPattern(R"(^\s*class\s+([\w_]+))");
    jsArrowFunctionPattern.setPattern(R"(^\s*(?:const|let|var)\s+([\w_]+)\s*=\s*\([^)]*\)\s*=>)");

    // Rust patterns
    rustFunctionPattern.setPattern(R"(^\s*(?:pub\s+)?(?:async\s+)?fn\s+([\w_]+))");
    rustStructPattern.setPattern(R"(^\s*(?:pub\s+)?struct\s+([\w_]+))");
    rustEnumPattern.setPattern(R"(^\s*(?:pub\s+)?enum\s+([\w_]+))");
    rustTraitPattern.setPattern(R"(^\s*(?:pub\s+)?trait\s+([\w_]+))");
    rustImplPattern.setPattern(R"(^\s*impl(?:\s+<[^>]+>)?\s+([\w_]+))");

    // TypeScript patterns
    tsFunctionPattern.setPattern(R"(^\s*(?:export\s+)?(?:async\s+)?function\s+([\w_]+)\s*[<(])");
    tsClassPattern.setPattern(R"(^\s*(?:export\s+)?(?:abstract\s+)?class\s+([\w_]+))");
    tsInterfacePattern.setPattern(R"(^\s*(?:export\s+)?interface\s+([\w_]+))");
    tsTypePattern.setPattern(R"(^\s*(?:export\s+)?type\s+([\w_]+))");
    tsEnumPattern.setPattern(R"(^\s*(?:export\s+)?enum\s+([\w_]+))");
    tsArrowFunctionPattern.setPattern(R"(^\s*(?:export\s+)?(?:const|let|var)\s+([\w_]+)\s*=\s*(?:async\s*)?\([^)]*\)\s*=>)");

    // Markdown patterns
    markdownHeaderPattern.setPattern(R"(^(#{1,6})\s+(.+)$)");
}

QList<SymbolInfo> SymbolExtractor::extractSymbols(const QString &documentText)
{
    QList<SymbolInfo> symbols;
    QStringList lines = documentText.split('\n');

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        int lineNumber = i + 1;
        QRegularExpressionMatch match;

        // Check for C/C++ functions
        match = functionPattern.match(line);
        if (match.hasMatch()) {
            QString returnType = match.captured(1);
            QString functionName = match.captured(2);
            // Skip common keywords that might be matched
            if (functionName != "if" && functionName != "while" && functionName != "for" &&
                functionName != "switch" && functionName != "return") {
                symbols.append(SymbolInfo(functionName, "Function", lineNumber, line.trimmed()));
            }
        }

        // Check for C++ classes
        match = classPattern.match(line);
        if (match.hasMatch()) {
            QString className = match.captured(1);
            symbols.append(SymbolInfo(className, "Class", lineNumber, line.trimmed()));
        }

        // Check for C++ structs
        match = structPattern.match(line);
        if (match.hasMatch()) {
            QString structName = match.captured(1);
            symbols.append(SymbolInfo(structName, "Struct", lineNumber, line.trimmed()));
        }

        // Check for Markdown headers
        match = markdownHeaderPattern.match(line);
        if (match.hasMatch()) {
            QString level = match.captured(1);
            QString headerText = match.captured(2);
            QString type = QString("Header H%1").arg(level.length());
            symbols.append(SymbolInfo(headerText, type, lineNumber, line.trimmed()));
        }

        // Check for Python functions
        match = pythonFunctionPattern.match(line);
        if (match.hasMatch()) {
            QString functionName = match.captured(1);
            symbols.append(SymbolInfo(functionName, "Function", lineNumber, line.trimmed()));
        }

        // Check for Python classes
        match = pythonClassPattern.match(line);
        if (match.hasMatch()) {
            QString className = match.captured(1);
            symbols.append(SymbolInfo(className, "Class", lineNumber, line.trimmed()));
        }

        // Check for JavaScript functions
        match = jsFunctionPattern.match(line);
        if (match.hasMatch()) {
            QString functionName = match.captured(1);
            symbols.append(SymbolInfo(functionName, "Function", lineNumber, line.trimmed()));
        }

        // Check for JavaScript arrow functions
        match = jsArrowFunctionPattern.match(line);
        if (match.hasMatch()) {
            QString functionName = match.captured(1);
            symbols.append(SymbolInfo(functionName, "Function", lineNumber, line.trimmed()));
        }

        // Check for JavaScript classes
        match = jsClassPattern.match(line);
        if (match.hasMatch()) {
            QString className = match.captured(1);
            symbols.append(SymbolInfo(className, "Class", lineNumber, line.trimmed()));
        }

        // Check for Rust functions
        match = rustFunctionPattern.match(line);
        if (match.hasMatch()) {
            QString functionName = match.captured(1);
            symbols.append(SymbolInfo(functionName, "Function (Rust)", lineNumber, line.trimmed()));
        }

        // Check for Rust structs
        match = rustStructPattern.match(line);
        if (match.hasMatch()) {
            QString structName = match.captured(1);
            symbols.append(SymbolInfo(structName, "Struct (Rust)", lineNumber, line.trimmed()));
        }

        // Check for Rust enums
        match = rustEnumPattern.match(line);
        if (match.hasMatch()) {
            QString enumName = match.captured(1);
            symbols.append(SymbolInfo(enumName, "Enum (Rust)", lineNumber, line.trimmed()));
        }

        // Check for Rust traits
        match = rustTraitPattern.match(line);
        if (match.hasMatch()) {
            QString traitName = match.captured(1);
            symbols.append(SymbolInfo(traitName, "Trait (Rust)", lineNumber, line.trimmed()));
        }

        // Check for Rust impl blocks
        match = rustImplPattern.match(line);
        if (match.hasMatch()) {
            QString implName = match.captured(1);
            symbols.append(SymbolInfo(implName, "Impl (Rust)", lineNumber, line.trimmed()));
        }

        // Check for TypeScript functions
        match = tsFunctionPattern.match(line);
        if (match.hasMatch()) {
            QString functionName = match.captured(1);
            symbols.append(SymbolInfo(functionName, "Function (TS)", lineNumber, line.trimmed()));
        }

        // Check for TypeScript arrow functions
        match = tsArrowFunctionPattern.match(line);
        if (match.hasMatch()) {
            QString functionName = match.captured(1);
            symbols.append(SymbolInfo(functionName, "Function (TS)", lineNumber, line.trimmed()));
        }

        // Check for TypeScript classes
        match = tsClassPattern.match(line);
        if (match.hasMatch()) {
            QString className = match.captured(1);
            symbols.append(SymbolInfo(className, "Class (TS)", lineNumber, line.trimmed()));
        }

        // Check for TypeScript interfaces
        match = tsInterfacePattern.match(line);
        if (match.hasMatch()) {
            QString interfaceName = match.captured(1);
            symbols.append(SymbolInfo(interfaceName, "Interface (TS)", lineNumber, line.trimmed()));
        }

        // Check for TypeScript types
        match = tsTypePattern.match(line);
        if (match.hasMatch()) {
            QString typeName = match.captured(1);
            symbols.append(SymbolInfo(typeName, "Type (TS)", lineNumber, line.trimmed()));
        }

        // Check for TypeScript enums
        match = tsEnumPattern.match(line);
        if (match.hasMatch()) {
            QString enumName = match.captured(1);
            symbols.append(SymbolInfo(enumName, "Enum (TS)", lineNumber, line.trimmed()));
        }
    }

    return symbols;
}
