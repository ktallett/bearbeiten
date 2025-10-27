#include "outlinepanel.h"
#include <QHeaderView>
#include <QRegularExpression>

OutlinePanel::OutlinePanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void OutlinePanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // Title label
    titleLabel = new QLabel(tr("Document Outline"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12pt; padding: 5px;");
    mainLayout->addWidget(titleLabel);

    // Status label
    statusLabel = new QLabel(tr("No symbols found"));
    statusLabel->setStyleSheet("color: gray; font-style: italic; padding: 2px 5px;");
    mainLayout->addWidget(statusLabel);

    // Tree widget
    treeWidget = new QTreeWidget();
    treeWidget->setHeaderHidden(true);
    treeWidget->setAlternatingRowColors(true);
    treeWidget->setAnimated(true);
    treeWidget->setIndentation(15);
    mainLayout->addWidget(treeWidget);

    connect(treeWidget, &QTreeWidget::itemClicked, this, &OutlinePanel::onItemClicked);
    connect(treeWidget, &QTreeWidget::itemDoubleClicked, this, &OutlinePanel::onItemDoubleClicked);
}

void OutlinePanel::updateOutline(const QString &documentText, const QString &fileName)
{
    currentFileName = fileName;

    if (!fileName.isEmpty()) {
        titleLabel->setText(tr("Outline: %1").arg(fileName));
    } else {
        titleLabel->setText(tr("Document Outline"));
    }

    QList<SymbolInfo> symbols = extractSymbols(documentText);
    populateTree(symbols);

    if (symbols.isEmpty()) {
        statusLabel->setText(tr("No symbols found"));
        statusLabel->setVisible(true);
    } else {
        statusLabel->setText(tr("%1 symbols").arg(symbols.count()));
        statusLabel->setVisible(true);
    }
}

void OutlinePanel::clear()
{
    treeWidget->clear();
    titleLabel->setText(tr("Document Outline"));
    statusLabel->setText(tr("No symbols found"));
    currentFileName.clear();
}

bool OutlinePanel::isEmpty() const
{
    return treeWidget->topLevelItemCount() == 0;
}

void OutlinePanel::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (item) {
        int lineNumber = item->data(0, Qt::UserRole).toInt();
        if (lineNumber > 0) {
            emit symbolClicked(lineNumber);
        }
    }
}

void OutlinePanel::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (item) {
        int lineNumber = item->data(0, Qt::UserRole).toInt();
        if (lineNumber > 0) {
            emit symbolClicked(lineNumber);
        }
    }
}

void OutlinePanel::populateTree(const QList<SymbolInfo> &symbols)
{
    treeWidget->clear();

    // Group symbols by type
    QMap<QString, QList<SymbolInfo>> groupedSymbols;
    for (const SymbolInfo &symbol : symbols) {
        groupedSymbols[symbol.type].append(symbol);
    }

    // Create tree structure
    for (auto it = groupedSymbols.constBegin(); it != groupedSymbols.constEnd(); ++it) {
        QString type = it.key();
        QList<SymbolInfo> symbolsOfType = it.value();

        // Create parent item for this type
        QTreeWidgetItem *typeItem = new QTreeWidgetItem(treeWidget);
        QString icon = getSymbolIcon(type);
        typeItem->setText(0, QString("%1 %2 (%3)").arg(icon).arg(type).arg(symbolsOfType.count()));
        typeItem->setExpanded(true);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsSelectable);

        // Add symbols under this type
        for (const SymbolInfo &symbol : symbolsOfType) {
            QTreeWidgetItem *symbolItem = new QTreeWidgetItem(typeItem);
            symbolItem->setText(0, QString("%1  %2 (Line %3)")
                                   .arg(icon)
                                   .arg(symbol.name)
                                   .arg(symbol.lineNumber));
            symbolItem->setData(0, Qt::UserRole, symbol.lineNumber);
            symbolItem->setToolTip(0, symbol.preview);
        }
    }
}

QList<SymbolInfo> OutlinePanel::extractSymbols(const QString &documentText)
{
    QList<SymbolInfo> symbols;
    QStringList lines = documentText.split('\n');

    // Pattern matching for various symbol types
    QRegularExpression functionPattern(R"(([\w:]+)\s+([\w:]+)\s*\([^)]*\)\s*\{?)"); // C/C++ functions
    QRegularExpression classPattern(R"(^\s*class\s+([\w:]+))"); // C++ classes
    QRegularExpression structPattern(R"(^\s*struct\s+([\w:]+))"); // C++ structs
    QRegularExpression markdownHeaderPattern(R"(^(#{1,6})\s+(.+)$)"); // Markdown headers
    QRegularExpression pythonFunctionPattern(R"(^\s*def\s+([\w_]+)\s*\()"); // Python functions
    QRegularExpression pythonClassPattern(R"(^\s*class\s+([\w_]+))"); // Python classes
    QRegularExpression jsFunctionPattern(R"(^\s*function\s+([\w_]+)\s*\()"); // JavaScript functions
    QRegularExpression jsClassPattern(R"(^\s*class\s+([\w_]+))"); // JavaScript classes
    QRegularExpression jsArrowFunctionPattern(R"(^\s*(?:const|let|var)\s+([\w_]+)\s*=\s*\([^)]*\)\s*=>)"); // JS arrow functions

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        int lineNumber = i + 1;

        // Check for C/C++ functions
        QRegularExpressionMatch match = functionPattern.match(line);
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
    }

    return symbols;
}

QString OutlinePanel::getSymbolIcon(const QString &symbolType)
{
    if (symbolType == "Function" || symbolType == "Function (Python)" || symbolType == "Function (JS)") {
        return "ƒ";
    } else if (symbolType == "Class" || symbolType == "Class (Python)" || symbolType == "Class (JS)") {
        return "C";
    } else if (symbolType == "Struct") {
        return "S";
    } else if (symbolType.startsWith("Header")) {
        return "#";
    } else {
        return "•";
    }
}
