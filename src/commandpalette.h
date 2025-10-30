#ifndef COMMANDPALETTE_H
#define COMMANDPALETTE_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QAction>
#include <QList>
#include <QVBoxLayout>

class CommandPalette : public QDialog
{
    Q_OBJECT

public:
    explicit CommandPalette(QWidget *parent = nullptr);

    void setActions(const QList<QAction*> &actions);
    void show();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void filterCommands(const QString &text);
    void executeCommand(QListWidgetItem *item);

private:
    QLineEdit *searchEdit;
    QListWidget *commandList;
    QList<QAction*> allActions;

    struct CommandItem {
        QAction *action;
        QString displayText;
        QString searchText;
    };

    QList<CommandItem> commandItems;

    void populateCommands();
    bool fuzzyMatch(const QString &pattern, const QString &text);
    int fuzzyScore(const QString &pattern, const QString &text);
};

#endif // COMMANDPALETTE_H
