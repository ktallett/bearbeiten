#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>

class GoToLineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoToLineDialog(QWidget *parent = nullptr);

    int getLineNumber() const;
    void setMaximumLine(int maxLine);
    void setLinePreview(int lineNumber, const QString &lineText);

signals:
    void lineNumberChanged(int lineNumber);
    void goToLineRequested(int lineNumber);

private slots:
    void onLineNumberChanged();
    void onGoClicked();

private:
    void setupUI();

    QLineEdit *lineNumberEdit;
    QPushButton *goButton;
    QPushButton *cancelButton;
    QLabel *statusLabel;
    QLabel *previewLabel;
    QPlainTextEdit *previewEdit;

    int maximumLine;
};

#endif // GOTOLINEDIALOG_H
