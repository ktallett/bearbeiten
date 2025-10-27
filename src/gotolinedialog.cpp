#include "gotolinedialog.h"
#include <QIntValidator>
#include <QGridLayout>

GoToLineDialog::GoToLineDialog(QWidget *parent)
    : QDialog(parent), maximumLine(1)
{
    setupUI();
    setWindowTitle(tr("Go to Line"));
    setModal(true);
    resize(400, 200);

    connect(lineNumberEdit, &QLineEdit::textChanged, this, &GoToLineDialog::onLineNumberChanged);
    connect(lineNumberEdit, &QLineEdit::returnPressed, this, &GoToLineDialog::onGoClicked);
    connect(goButton, &QPushButton::clicked, this, &GoToLineDialog::onGoClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    onLineNumberChanged();
}

void GoToLineDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Line number input section
    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLabel *label = new QLabel(tr("Line number:"));
    lineNumberEdit = new QLineEdit();
    lineNumberEdit->setValidator(new QIntValidator(1, 999999, this));
    lineNumberEdit->setPlaceholderText(tr("Enter line number"));

    inputLayout->addWidget(label);
    inputLayout->addWidget(lineNumberEdit);

    mainLayout->addLayout(inputLayout);

    // Status label
    statusLabel = new QLabel();
    statusLabel->setStyleSheet("color: gray; font-style: italic;");
    mainLayout->addWidget(statusLabel);

    // Preview section
    previewLabel = new QLabel(tr("Preview:"));
    mainLayout->addWidget(previewLabel);

    previewEdit = new QPlainTextEdit();
    previewEdit->setReadOnly(true);
    previewEdit->setMaximumHeight(80);
    previewEdit->setStyleSheet("background-color: #f5f5f5; border: 1px solid #ccc;");
    mainLayout->addWidget(previewEdit);

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

int GoToLineDialog::getLineNumber() const
{
    bool ok;
    int lineNum = lineNumberEdit->text().toInt(&ok);
    return ok ? lineNum : -1;
}

void GoToLineDialog::setMaximumLine(int maxLine)
{
    maximumLine = maxLine;
    if (const QIntValidator *validator = qobject_cast<const QIntValidator*>(lineNumberEdit->validator())) {
        // Create a new validator with updated range
        lineNumberEdit->setValidator(new QIntValidator(1, maximumLine, this));
    }
    statusLabel->setText(tr("Enter a line number between 1 and %1").arg(maximumLine));
}

void GoToLineDialog::setLinePreview(int lineNumber, const QString &lineText)
{
    if (lineNumber > 0 && lineNumber <= maximumLine) {
        previewEdit->setPlainText(lineText.trimmed());
        previewEdit->setVisible(true);
        previewLabel->setVisible(true);
    } else {
        previewEdit->setVisible(false);
        previewLabel->setVisible(false);
    }
}

void GoToLineDialog::onLineNumberChanged()
{
    int lineNum = getLineNumber();
    bool valid = lineNum > 0 && lineNum <= maximumLine;

    goButton->setEnabled(valid);

    if (lineNumberEdit->text().isEmpty()) {
        statusLabel->setText(tr("Enter a line number between 1 and %1").arg(maximumLine));
        previewEdit->setVisible(false);
        previewLabel->setVisible(false);
    } else if (!valid) {
        statusLabel->setText(tr("Invalid line number"));
        statusLabel->setStyleSheet("color: red; font-style: italic;");
        previewEdit->setVisible(false);
        previewLabel->setVisible(false);
    } else {
        statusLabel->setText(tr("Line %1 of %2").arg(lineNum).arg(maximumLine));
        statusLabel->setStyleSheet("color: gray; font-style: italic;");
        emit lineNumberChanged(lineNum);
    }
}

void GoToLineDialog::onGoClicked()
{
    int lineNum = getLineNumber();
    if (lineNum > 0 && lineNum <= maximumLine) {
        emit goToLineRequested(lineNum);
        accept();
    }
}
