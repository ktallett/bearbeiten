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

    QList<SymbolInfo> symbols = symbolExtractor.extractSymbols(documentText);
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

QString OutlinePanel::getSymbolIcon(const QString &symbolType)
{
    if (symbolType == "Function" || symbolType == "Function (Python)" || symbolType == "Function (JS)" ||
        symbolType == "Function (Rust)" || symbolType == "Function (TS)") {
        return "ƒ";
    } else if (symbolType == "Class" || symbolType == "Class (Python)" || symbolType == "Class (JS)" ||
               symbolType == "Class (TS)") {
        return "C";
    } else if (symbolType == "Struct" || symbolType == "Struct (Rust)") {
        return "S";
    } else if (symbolType.startsWith("Header")) {
        return "#";
    } else if (symbolType == "Enum (Rust)" || symbolType == "Enum (TS)") {
        return "E";
    } else if (symbolType == "Trait (Rust)") {
        return "T";
    } else if (symbolType == "Impl (Rust)") {
        return "I";
    } else if (symbolType == "Interface (TS)") {
        return "Ⓘ";
    } else if (symbolType == "Type (TS)") {
        return "τ";
    } else {
        return "•";
    }
}
