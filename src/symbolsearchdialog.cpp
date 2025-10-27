#include "symbolsearchdialog.h"
#include <QGridLayout>
#include <QListWidgetItem>
#include <QKeyEvent>

SymbolSearchDialog::SymbolSearchDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle(tr("Go to Symbol"));
    setModal(true);
    resize(500, 400);

    connect(searchEdit, &QLineEdit::textChanged, this, &SymbolSearchDialog::onSearchTextChanged);
    connect(searchEdit, &QLineEdit::returnPressed, this, &SymbolSearchDialog::onItemSelected);
    connect(symbolList, &QListWidget::itemActivated, this, &SymbolSearchDialog::onItemActivated);
    connect(symbolList, &QListWidget::itemDoubleClicked, this, &SymbolSearchDialog::onItemActivated);
    connect(goButton, &QPushButton::clicked, this, &SymbolSearchDialog::onItemSelected);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SymbolSearchDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Search input section
    QHBoxLayout *searchLayout = new QHBoxLayout();

    QLabel *searchLabel = new QLabel(tr("Search:"));
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(tr("Type to filter symbols (fuzzy match)..."));

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);

    mainLayout->addLayout(searchLayout);

    // Status label
    statusLabel = new QLabel();
    statusLabel->setStyleSheet("color: gray; font-style: italic;");
    mainLayout->addWidget(statusLabel);

    // Symbol list
    symbolList = new QListWidget();
    symbolList->setAlternatingRowColors(true);
    mainLayout->addWidget(symbolList);

    // Buttons section
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    goButton = new QPushButton(tr("Go"));
    cancelButton = new QPushButton(tr("Cancel"));

    goButton->setDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(goButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void SymbolSearchDialog::setSymbols(const QList<SymbolInfo> &symbols)
{
    allSymbols = symbols;
    symbolList->clear();

    for (const SymbolInfo &symbol : allSymbols) {
        QString displayText = QString("%1 (%2) - Line %3")
                                  .arg(symbol.name)
                                  .arg(symbol.type)
                                  .arg(symbol.lineNumber);

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, symbol.lineNumber);
        item->setToolTip(symbol.preview);
        symbolList->addItem(item);
    }

    statusLabel->setText(tr("%1 symbols found").arg(allSymbols.count()));

    // Select first item by default
    if (symbolList->count() > 0) {
        symbolList->setCurrentRow(0);
    }
}

void SymbolSearchDialog::clearFilter()
{
    searchEdit->clear();
}

void SymbolSearchDialog::onSearchTextChanged()
{
    filterSymbols();
}

void SymbolSearchDialog::onItemActivated(QListWidgetItem *item)
{
    if (item) {
        int lineNumber = item->data(Qt::UserRole).toInt();
        emit symbolSelected(lineNumber);
        accept();
    }
}

void SymbolSearchDialog::onItemSelected()
{
    QListWidgetItem *item = symbolList->currentItem();
    if (item) {
        onItemActivated(item);
    }
}

void SymbolSearchDialog::filterSymbols()
{
    QString filterText = searchEdit->text().trimmed();

    symbolList->clear();

    if (filterText.isEmpty()) {
        // Show all symbols
        for (const SymbolInfo &symbol : allSymbols) {
            QString displayText = QString("%1 (%2) - Line %3")
                                      .arg(symbol.name)
                                      .arg(symbol.type)
                                      .arg(symbol.lineNumber);

            QListWidgetItem *item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, symbol.lineNumber);
            item->setToolTip(symbol.preview);
            symbolList->addItem(item);
        }
    } else {
        // Fuzzy filter
        int matchCount = 0;
        for (const SymbolInfo &symbol : allSymbols) {
            if (fuzzyMatch(filterText, symbol.name)) {
                QString displayText = QString("%1 (%2) - Line %3")
                                          .arg(symbol.name)
                                          .arg(symbol.type)
                                          .arg(symbol.lineNumber);

                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, symbol.lineNumber);
                item->setToolTip(symbol.preview);
                symbolList->addItem(item);
                matchCount++;
            }
        }
        statusLabel->setText(tr("%1 of %2 symbols match").arg(matchCount).arg(allSymbols.count()));
    }

    // Select first item
    if (symbolList->count() > 0) {
        symbolList->setCurrentRow(0);
    }

    goButton->setEnabled(symbolList->count() > 0);
}

bool SymbolSearchDialog::fuzzyMatch(const QString &pattern, const QString &text)
{
    if (pattern.isEmpty()) {
        return true;
    }

    QString patternLower = pattern.toLower();
    QString textLower = text.toLower();

    int patternIndex = 0;
    int textIndex = 0;

    while (patternIndex < patternLower.length() && textIndex < textLower.length()) {
        if (patternLower[patternIndex] == textLower[textIndex]) {
            patternIndex++;
        }
        textIndex++;
    }

    return patternIndex == patternLower.length();
}
