#ifndef BREADCRUMBBAR_H
#define BREADCRUMBBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "symbolextractor.h"

/**
 * @brief A breadcrumb navigation bar showing file path and document structure
 *
 * The BreadcrumbBar displays:
 * 1. File path as clickable segments (e.g., /home > user > project > main.cpp)
 * 2. Current symbol (function/class) at cursor position
 *
 * Clicking on path segments opens the directory in the system file manager.
 */
class BreadcrumbBar : public QWidget
{
    Q_OBJECT

public:
    explicit BreadcrumbBar(QWidget *parent = nullptr);

    /**
     * @brief Update the breadcrumb to show a new file path
     * @param filePath The absolute path to the current file
     */
    void setFilePath(const QString &filePath);

    /**
     * @brief Update the current symbol (function/class) in the breadcrumb
     * @param symbolName The name of the current symbol
     * @param symbolType The type of symbol (function, class, etc.)
     */
    void setCurrentSymbol(const QString &symbolName, const QString &symbolType);

    /**
     * @brief Clear the breadcrumb display
     */
    void clear();

signals:
    /**
     * @brief Emitted when a path segment is clicked
     * @param path The directory path that was clicked
     */
    void pathSegmentClicked(const QString &path);

private slots:
    void onPathSegmentClicked();

private:
    void setupUI();
    void updateBreadcrumb();
    void clearBreadcrumb();
    QString truncatePathSegment(const QString &segment, int maxLength = 20);

    QHBoxLayout *layout;
    QLabel *iconLabel;

    QString currentFilePath;
    QString currentSymbolName;
    QString currentSymbolType;

    // Store path segments for navigation
    QList<QPair<QString, QString>> pathSegments; // <display name, full path>
};

#endif // BREADCRUMBBAR_H
