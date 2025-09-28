#include "projectpanel.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

ProjectPanel::ProjectPanel(QWidget *parent)
    : QWidget(parent), treeView(nullptr), fileSystemModel(nullptr),
      openProjectButton(nullptr), refreshButton(nullptr), projectLabel(nullptr)
{
    setupUI();
}

void ProjectPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // Header section
    QHBoxLayout *headerLayout = new QHBoxLayout();

    projectLabel = new QLabel(tr("No Project"));
    projectLabel->setStyleSheet("font-weight: bold; color: #666;");
    headerLayout->addWidget(projectLabel);

    headerLayout->addStretch();

    openProjectButton = new QPushButton(tr("Open"));
    openProjectButton->setMaximumWidth(60);
    connect(openProjectButton, &QPushButton::clicked, this, &ProjectPanel::openProject);
    headerLayout->addWidget(openProjectButton);

    refreshButton = new QPushButton("↻");
    refreshButton->setMaximumWidth(30);
    refreshButton->setToolTip(tr("Refresh Project"));
    connect(refreshButton, &QPushButton::clicked, this, &ProjectPanel::refreshProject);
    headerLayout->addWidget(refreshButton);

    mainLayout->addLayout(headerLayout);

    // Tree view section
    treeView = new QTreeView();
    treeView->setHeaderHidden(true);
    treeView->setRootIsDecorated(true);
    treeView->setSortingEnabled(true);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath("");
    fileSystemModel->setNameFilters(QStringList());
    fileSystemModel->setNameFilterDisables(false);

    treeView->setModel(fileSystemModel);

    // Hide all columns except name
    treeView->hideColumn(1); // Size
    treeView->hideColumn(2); // Type
    treeView->hideColumn(3); // Date Modified

    connect(treeView, &QTreeView::doubleClicked, this, &ProjectPanel::onItemDoubleClicked);
    connect(treeView, &QTreeView::clicked, this, &ProjectPanel::onItemClicked);
    connect(treeView, &QTreeView::customContextMenuRequested, this, &ProjectPanel::showContextMenu);

    mainLayout->addWidget(treeView);

    // Initially disable refresh button
    refreshButton->setEnabled(false);
    treeView->hide();
}

void ProjectPanel::setProjectPath(const QString &path)
{
    if (QDir(path).exists()) {
        currentProjectPath = path;
        QModelIndex rootIndex = fileSystemModel->setRootPath(path);
        treeView->setRootIndex(rootIndex);
        updateProjectDisplay();
        emit projectChanged(path);
    }
}

QString ProjectPanel::getProjectPath() const
{
    return currentProjectPath;
}

bool ProjectPanel::hasProject() const
{
    return !currentProjectPath.isEmpty();
}

void ProjectPanel::openProject()
{
    QString projectPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Project Folder"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!projectPath.isEmpty()) {
        setProjectPath(projectPath);
    }
}

void ProjectPanel::closeProject()
{
    currentProjectPath.clear();
    updateProjectDisplay();
    emit projectChanged("");
}

void ProjectPanel::refreshProject()
{
    if (hasProject()) {
        fileSystemModel->setRootPath("");
        QModelIndex rootIndex = fileSystemModel->setRootPath(currentProjectPath);
        treeView->setRootIndex(rootIndex);
    }
}

void ProjectPanel::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isFile()) {
        emit fileRequested(filePath);
    }
}

void ProjectPanel::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    // Single click could be used for selection/preview in the future
}

void ProjectPanel::showContextMenu(const QPoint &point)
{
    QModelIndex index = treeView->indexAt(point);
    if (!index.isValid()) return;

    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);

    QMenu contextMenu(this);

    if (fileInfo.isFile()) {
        QAction *openAction = contextMenu.addAction(tr("Open"));
        connect(openAction, &QAction::triggered, [this, filePath]() {
            emit fileRequested(filePath);
        });

        contextMenu.addSeparator();
    }

    QAction *showInExplorerAction = contextMenu.addAction(tr("Show in File Manager"));
    connect(showInExplorerAction, &QAction::triggered, [filePath]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).dir().absolutePath()));
    });

    if (hasProject()) {
        contextMenu.addSeparator();
        QAction *refreshAction = contextMenu.addAction(tr("Refresh"));
        connect(refreshAction, &QAction::triggered, this, &ProjectPanel::refreshProject);
    }

    contextMenu.exec(treeView->mapToGlobal(point));
}

void ProjectPanel::updateProjectDisplay()
{
    if (hasProject()) {
        QDir dir(currentProjectPath);
        projectLabel->setText(dir.dirName());
        projectLabel->setToolTip(currentProjectPath);
        openProjectButton->setText(tr("Close"));
        refreshButton->setEnabled(true);
        treeView->show();

        // Disconnect and reconnect to handle both open/close
        disconnect(openProjectButton, &QPushButton::clicked, this, &ProjectPanel::openProject);
        connect(openProjectButton, &QPushButton::clicked, this, &ProjectPanel::closeProject);
    } else {
        projectLabel->setText(tr("No Project"));
        projectLabel->setToolTip("");
        openProjectButton->setText(tr("Open"));
        refreshButton->setEnabled(false);
        treeView->hide();

        // Disconnect and reconnect to handle both open/close
        disconnect(openProjectButton, &QPushButton::clicked, this, &ProjectPanel::closeProject);
        connect(openProjectButton, &QPushButton::clicked, this, &ProjectPanel::openProject);
    }
}