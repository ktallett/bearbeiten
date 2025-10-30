#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle(tr("Find and Replace"));
    setModal(false);
    resize(400, 200);

    connect(findLineEdit, &QLineEdit::textChanged, this, &FindDialog::onFindTextChanged);
    connect(findLineEdit, &QLineEdit::returnPressed, this, &FindDialog::findNext);
    connect(replaceLineEdit, &QLineEdit::returnPressed, this, &FindDialog::replace);

    connect(findNextButton, &QPushButton::clicked, this, &FindDialog::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &FindDialog::findPrevious);
    connect(replaceButton, &QPushButton::clicked, this, &FindDialog::replace);
    connect(replaceAllButton, &QPushButton::clicked, this, &FindDialog::replaceAll);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    onFindTextChanged();
}

void FindDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Find/Replace input section
    QGridLayout *inputLayout = new QGridLayout();

    inputLayout->addWidget(new QLabel(tr("Find:")), 0, 0);
    findLineEdit = new QLineEdit();
    inputLayout->addWidget(findLineEdit, 0, 1);

    inputLayout->addWidget(new QLabel(tr("Replace:")), 1, 0);
    replaceLineEdit = new QLineEdit();
    inputLayout->addWidget(replaceLineEdit, 1, 1);

    mainLayout->addLayout(inputLayout);

    // Options section
    QGroupBox *optionsGroup = new QGroupBox(tr("Options"));
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);

    caseSensitiveCheckBox = new QCheckBox(tr("Case sensitive"));
    wholeWordsCheckBox = new QCheckBox(tr("Whole words only"));
    regexCheckBox = new QCheckBox(tr("Regular expressions"));

    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addWidget(regexCheckBox);

    mainLayout->addWidget(optionsGroup);

    // Buttons section
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    findNextButton = new QPushButton(tr("Find Next"));
    findPreviousButton = new QPushButton(tr("Find Previous"));
    replaceButton = new QPushButton(tr("Replace"));
    replaceAllButton = new QPushButton(tr("Replace All"));
    closeButton = new QPushButton(tr("Close"));

    findNextButton->setDefault(true);

    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPreviousButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);

    // Status label
    statusLabel = new QLabel();
    statusLabel->setStyleSheet("color: red; font-style: italic;");
    mainLayout->addWidget(statusLabel);
}

QString FindDialog::findText() const
{
    return findLineEdit->text();
}

QString FindDialog::replaceText() const
{
    return replaceLineEdit->text();
}

bool FindDialog::caseSensitive() const
{
    return caseSensitiveCheckBox->isChecked();
}

bool FindDialog::wholeWords() const
{
    return wholeWordsCheckBox->isChecked();
}

bool FindDialog::useRegex() const
{
    return regexCheckBox->isChecked();
}

void FindDialog::setFindText(const QString &text)
{
    findLineEdit->setText(text);
    findLineEdit->selectAll();
}

void FindDialog::setStatus(const QString &message)
{
    statusLabel->setText(message);
}

void FindDialog::findNext()
{
    if (!findText().isEmpty()) {
        statusLabel->clear();
        emit findRequested(findText(), true, caseSensitive(), wholeWords(), useRegex());
    }
}

void FindDialog::findPrevious()
{
    if (!findText().isEmpty()) {
        statusLabel->clear();
        emit findRequested(findText(), false, caseSensitive(), wholeWords(), useRegex());
    }
}

void FindDialog::replace()
{
    if (!findText().isEmpty()) {
        statusLabel->clear();
        emit replaceRequested(findText(), replaceText(), caseSensitive(), wholeWords(), useRegex());
    }
}

void FindDialog::replaceAll()
{
    if (!findText().isEmpty()) {
        statusLabel->clear();
        emit replaceAllRequested(findText(), replaceText(), caseSensitive(), wholeWords(), useRegex());
    }
}

void FindDialog::onFindTextChanged()
{
    bool hasText = !findText().isEmpty();
    findNextButton->setEnabled(hasText);
    findPreviousButton->setEnabled(hasText);
    replaceButton->setEnabled(hasText);
    replaceAllButton->setEnabled(hasText);
    statusLabel->clear();
}