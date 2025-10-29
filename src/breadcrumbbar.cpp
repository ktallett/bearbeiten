#include "breadcrumbbar.h"
#include <QStyle>
#include <QApplication>
#include <QFontMetrics>
#include <QDebug>

BreadcrumbBar::BreadcrumbBar(QWidget *parent)
    : QWidget(parent), layout(nullptr), iconLabel(nullptr)
{
    setupUI();
}

void BreadcrumbBar::setupUI()
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(0);

    // Add file icon
    iconLabel = new QLabel(this);
    iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileIcon).pixmap(16, 16));
    layout->addWidget(iconLabel);

    layout->addStretch();

    setLayout(layout);
    setMaximumHeight(30);
    setMinimumHeight(25);
}

void BreadcrumbBar::setFilePath(const QString &filePath)
{
    if (filePath.isEmpty() || filePath == currentFilePath) {
        return;
    }

    currentFilePath = filePath;
    updateBreadcrumb();
}

void BreadcrumbBar::setCurrentSymbol(const QString &symbolName, const QString &symbolType)
{
    currentSymbolName = symbolName;
    currentSymbolType = symbolType;
    updateBreadcrumb();
}

void BreadcrumbBar::clear()
{
    currentFilePath.clear();
    currentSymbolName.clear();
    currentSymbolType.clear();
    clearBreadcrumb();
}

void BreadcrumbBar::clearBreadcrumb()
{
    // Remove all widgets except the icon and final stretch
    while (layout->count() > 2) {
        QLayoutItem *item = layout->takeAt(1); // Always remove after icon
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    pathSegments.clear();
}

void BreadcrumbBar::updateBreadcrumb()
{
    clearBreadcrumb();

    if (currentFilePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(currentFilePath);
    QString absolutePath = fileInfo.absoluteFilePath();

    // Split the path into segments
    QStringList parts;
    QDir dir = fileInfo.absoluteDir();
    QString fileName = fileInfo.fileName();

    // Build path from root to file
    QString currentPath = absolutePath;
    QDir currentDir(currentPath);

    // Get all parent directories
    QStringList dirParts;
    while (true) {
        dirParts.prepend(currentDir.dirName());
        if (currentDir.isRoot()) {
            break;
        }
        if (!currentDir.cdUp()) {
            break;
        }
    }

    // Add root/drive
#ifdef Q_OS_WIN
    if (!dirParts.isEmpty() && dirParts.first().endsWith(":")) {
        // On Windows, show drive letter
        QString drive = dirParts.first();
        QPushButton *driveButton = new QPushButton(drive, this);
        driveButton->setFlat(true);
        driveButton->setCursor(Qt::PointingHandCursor);
        driveButton->setProperty("pathData", drive + "/");
        connect(driveButton, &QPushButton::clicked, this, &BreadcrumbBar::onPathSegmentClicked);
        layout->insertWidget(layout->count() - 1, driveButton);
        pathSegments.append(qMakePair(drive, drive + "/"));
        dirParts.removeFirst();
    }
#else
    // On Unix-like systems, show root
    QPushButton *rootButton = new QPushButton("/", this);
    rootButton->setFlat(true);
    rootButton->setCursor(Qt::PointingHandCursor);
    rootButton->setProperty("pathData", "/");
    connect(rootButton, &QPushButton::clicked, this, &BreadcrumbBar::onPathSegmentClicked);
    layout->insertWidget(layout->count() - 1, rootButton);
    pathSegments.append(qMakePair(QString("/"), QString("/")));

    // Skip the first empty element and root element
    if (!dirParts.isEmpty() && dirParts.first().isEmpty()) {
        dirParts.removeFirst();
    }
#endif

    // Build cumulative path for each segment
    QString cumulativePath = "/";
#ifdef Q_OS_WIN
    cumulativePath = dirParts.isEmpty() ? "" : (dirParts.first() + "/");
#endif

    // Add directory segments (excluding the filename)
    QStringList pathToFile = fileInfo.absolutePath().split("/", Qt::SkipEmptyParts);
#ifdef Q_OS_WIN
    if (!pathToFile.isEmpty() && pathToFile.first().endsWith(":")) {
        pathToFile.removeFirst(); // Remove drive letter, already added
    }
#endif

    for (const QString &segment : pathToFile) {
        if (segment.isEmpty()) continue;

        // Add separator
        QLabel *separator = new QLabel(" › ", this);
        separator->setStyleSheet("color: palette(mid);");
        layout->insertWidget(layout->count() - 1, separator);

        // Build path up to this segment
#ifdef Q_OS_WIN
        if (cumulativePath.isEmpty() || cumulativePath.endsWith(":")) {
            cumulativePath += segment;
        } else {
            cumulativePath += "/" + segment;
        }
#else
        if (cumulativePath == "/") {
            cumulativePath += segment;
        } else {
            cumulativePath += "/" + segment;
        }
#endif

        // Add clickable segment
        QString displayName = truncatePathSegment(segment);
        QPushButton *segmentButton = new QPushButton(displayName, this);
        segmentButton->setFlat(true);
        segmentButton->setCursor(Qt::PointingHandCursor);
        segmentButton->setProperty("pathData", cumulativePath);
        segmentButton->setToolTip(cumulativePath);
        connect(segmentButton, &QPushButton::clicked, this, &BreadcrumbBar::onPathSegmentClicked);
        layout->insertWidget(layout->count() - 1, segmentButton);
        pathSegments.append(qMakePair(displayName, cumulativePath));
    }

    // Add separator before filename
    QLabel *separator = new QLabel(" › ", this);
    separator->setStyleSheet("color: palette(mid);");
    layout->insertWidget(layout->count() - 1, separator);

    // Add filename (not clickable, just display)
    QString displayFileName = truncatePathSegment(fileName, 30);
    QLabel *fileLabel = new QLabel(displayFileName, this);
    fileLabel->setStyleSheet("font-weight: bold; color: palette(text);");
    fileLabel->setToolTip(fileName);
    layout->insertWidget(layout->count() - 1, fileLabel);

    // Add current symbol if available
    if (!currentSymbolName.isEmpty()) {
        QLabel *symbolSeparator = new QLabel(" › ", this);
        symbolSeparator->setStyleSheet("color: palette(mid);");
        layout->insertWidget(layout->count() - 1, symbolSeparator);

        QString symbolIcon = "⚡"; // Default icon
        if (currentSymbolType == "function") {
            symbolIcon = "ƒ";
        } else if (currentSymbolType == "class") {
            symbolIcon = "⬢";
        } else if (currentSymbolType == "struct") {
            symbolIcon = "◊";
        } else if (currentSymbolType == "method") {
            symbolIcon = "⚡";
        } else if (currentSymbolType == "header") {
            symbolIcon = "#";
        }

        QString symbolText = symbolIcon + " " + truncatePathSegment(currentSymbolName, 25);
        QLabel *symbolLabel = new QLabel(symbolText, this);
        symbolLabel->setStyleSheet("font-style: italic; color: palette(link);");
        symbolLabel->setToolTip(tr("Current: %1 (%2)").arg(currentSymbolName).arg(currentSymbolType));
        layout->insertWidget(layout->count() - 1, symbolLabel);
    }
}

QString BreadcrumbBar::truncatePathSegment(const QString &segment, int maxLength)
{
    if (segment.length() <= maxLength) {
        return segment;
    }

    // Truncate in the middle to preserve start and end
    int halfLength = (maxLength - 3) / 2;
    return segment.left(halfLength) + "..." + segment.right(halfLength);
}

void BreadcrumbBar::onPathSegmentClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    QString path = button->property("pathData").toString();
    if (path.isEmpty()) {
        return;
    }

    // Open the directory in the system file manager
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    } else if (fileInfo.exists()) {
        // If it's a file, open its parent directory
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
    }

    emit pathSegmentClicked(path);
}
