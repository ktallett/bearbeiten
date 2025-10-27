#ifndef SYMBOLSEARCHDIALOG_H
#define SYMBOLSEARCHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

struct SymbolInfo {
    QString name;
    QString type;
    int lineNumber;
    QString preview;

    SymbolInfo() : lineNumber(0) {}
    SymbolInfo(const QString &n, const QString &t, int line, const QString &p)
        : name(n), type(t), lineNumber(line), preview(p) {}
};

class SymbolSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SymbolSearchDialog(QWidget *parent = nullptr);

    void setSymbols(const QList<SymbolInfo> &symbols);
    void clearFilter();

signals:
    void symbolSelected(int lineNumber);

private slots:
    void onSearchTextChanged();
    void onItemActivated(QListWidgetItem *item);
    void onItemSelected();

private:
    void setupUI();
    void filterSymbols();
    bool fuzzyMatch(const QString &pattern, const QString &text);

    QLineEdit *searchEdit;
    QListWidget *symbolList;
    QPushButton *goButton;
    QPushButton *cancelButton;
    QLabel *statusLabel;

    QList<SymbolInfo> allSymbols;
};

#endif // SYMBOLSEARCHDIALOG_H
