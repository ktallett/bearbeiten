#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Bearbeiten");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Bearbeiten Project");

    // Numworks-inspired styling with grey, white, and subtle orange accents
    app.setStyleSheet(
        "QMainWindow {"
        "    background-color: #ffffff;"
        "}"
        "QMenuBar {"
        "    background-color: #f8f8f8;"
        "    color: #4a4a4a;"
        "    border-bottom: 1px solid #e0e0e0;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: rgba(255, 140, 50, 0.1);" // Very subtle orange
        "}"
        "QMenu {"
        "    background-color: #ffffff;"
        "    color: #4a4a4a;"
        "    border: 1px solid #e0e0e0;"
        "}"
        "QMenu::item:selected {"
        "    background-color: rgba(255, 140, 50, 0.15);" // Subtle orange highlight
        "}"
        "QToolBar {"
        "    background-color: #f5f5f5;"
        "    border-bottom: 1px solid #e0e0e0;"
        "    spacing: 3px;"
        "}"
        "QToolButton {"
        "    background-color: transparent;"
        "    color: #4a4a4a;"
        "    border: none;"
        "    padding: 5px;"
        "}"
        "QToolButton:hover {"
        "    background-color: rgba(255, 140, 50, 0.1);"
        "}"
        "QToolButton:pressed {"
        "    background-color: rgba(255, 140, 50, 0.2);"
        "}"
        "QTabWidget::pane {"
        "    border: 1px solid #e0e0e0;"
        "    background-color: #ffffff;"
        "}"
        "QTabBar::tab {"
        "    background-color: #f0f0f0;"
        "    color: #4a4a4a;"
        "    border: 1px solid #e0e0e0;"
        "    padding: 6px 12px;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: #ffffff;"
        "    border-bottom-color: #ffffff;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "    background-color: rgba(255, 140, 50, 0.08);"
        "}"
        "QPlainTextEdit {"
        "    background-color: #ffffff;"
        "    color: #2a2a2a;"
        "    border: none;"
        "    selection-background-color: rgba(255, 140, 50, 0.3);" // Orange selection
        "}"
        "QComboBox {"
        "    background-color: #ffffff;"
        "    color: #4a4a4a;"
        "    border: 1px solid #d0d0d0;"
        "    padding: 3px;"
        "}"
        "QComboBox:hover {"
        "    border-color: rgba(255, 140, 50, 0.5);"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
        "QLabel {"
        "    color: #4a4a4a;"
        "}"
    );

    // Setup internationalization
    QTranslator translator;

    // Get system locale
    QString locale = QLocale::system().name(); // e.g. "en_US", "de_DE", "fr_FR"

    // Try to load translation file
    QString translationFile = QString("bearbeiten_%1.qm").arg(locale);

    // Look for translations in multiple paths
    QStringList searchPaths;
    searchPaths << QDir::currentPath() + "/translations"
                << QApplication::applicationDirPath() + "/translations"
                << ":/translations"; // Qt resource system

    bool translationLoaded = false;
    for (const QString &path : searchPaths) {
        QString fullPath = path + "/" + translationFile;
        if (translator.load(fullPath)) {
            app.installTranslator(&translator);
            translationLoaded = true;
            break;
        }
    }

    // Fallback to language-only code if country-specific translation not found
    if (!translationLoaded && locale.contains('_')) {
        QString languageCode = locale.split('_').first();
        translationFile = QString("bearbeiten_%1.qm").arg(languageCode);

        for (const QString &path : searchPaths) {
            QString fullPath = path + "/" + translationFile;
            if (translator.load(fullPath)) {
                app.installTranslator(&translator);
                break;
            }
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}