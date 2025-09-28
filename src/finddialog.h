#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);

    QString findText() const;
    QString replaceText() const;
    bool caseSensitive() const;
    bool wholeWords() const;
    bool useRegex() const;

    void setFindText(const QString &text);

public slots:
    void findNext();
    void findPrevious();
    void replace();
    void replaceAll();

signals:
    void findRequested(const QString &text, bool forward, bool caseSensitive, bool wholeWords, bool useRegex);
    void replaceRequested(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex);
    void replaceAllRequested(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex);

private slots:
    void onFindTextChanged();

private:
    void setupUI();

    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QPushButton *findNextButton;
    QPushButton *findPreviousButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QPushButton *closeButton;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QCheckBox *regexCheckBox;
    QLabel *statusLabel;
};

#endif // FINDDIALOG_H