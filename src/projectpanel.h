#ifndef PROJECTPANEL_H
#define PROJECTPANEL_H

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QAction>

class ProjectPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectPanel(QWidget *parent = nullptr);

    void setProjectPath(const QString &path);
    QString getProjectPath() const;
    bool hasProject() const;

public slots:
    void openProject();
    void closeProject();
    void refreshProject();

signals:
    void fileRequested(const QString &filePath);
    void projectChanged(const QString &projectPath);

private slots:
    void onItemDoubleClicked(const QModelIndex &index);
    void onItemClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &point);

private:
    void setupUI();
    void updateProjectDisplay();

    QTreeView *treeView;
    QFileSystemModel *fileSystemModel;
    QPushButton *openProjectButton;
    QPushButton *refreshButton;
    QLabel *projectLabel;
    QString currentProjectPath;
};

#endif // PROJECTPANEL_H