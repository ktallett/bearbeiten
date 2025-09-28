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