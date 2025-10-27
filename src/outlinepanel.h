#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include "symbolsearchdialog.h"
#include "symbolextractor.h"

class OutlinePanel : public QWidget
{
    Q_OBJECT

public:
    explicit OutlinePanel(QWidget *parent = nullptr);

    void updateOutline(const QString &documentText, const QString &fileName = QString());
    void clear();
    bool isEmpty() const;

signals:
    void symbolClicked(int lineNumber);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void setupUI();
    void populateTree(const QList<SymbolInfo> &symbols);
    QString getSymbolIcon(const QString &symbolType);

    QTreeWidget *treeWidget;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QString currentFileName;
    SymbolExtractor symbolExtractor;
};

#endif // OUTLINEPANEL_H
