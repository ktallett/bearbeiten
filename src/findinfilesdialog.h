#ifndef FINDINFILESDIALOG_H
#define FINDINFILESDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTreeWidget>
#include <QLabel>
#include <QProgressBar>
#include <QThread>
#include <QRegularExpression>

// Worker class for background searching
class SearchWorker : public QObject
{
    Q_OBJECT

public:
    explicit SearchWorker(QObject *parent = nullptr);

    void setSearchParameters(const QString &searchText, const QString &directory,
                            const QStringList &filePatterns, bool caseSensitive,
                            bool wholeWords, bool useRegex);

public slots:
    void performSearch();

signals:
    void searchProgress(int current, int total);
    void resultFound(const QString &filePath, int lineNumber, const QString &lineText);
    void searchComplete(int totalMatches);

private:
    QString m_searchText;
    QString m_directory;
    QStringList m_filePatterns;
    bool m_caseSensitive;
    bool m_wholeWords;
    bool m_useRegex;

    void searchInFile(const QString &filePath);
    QList<QString> getFilesToSearch(const QString &directory, const QStringList &patterns);
    bool matchesPattern(const QString &fileName, const QStringList &patterns);
};

class FindInFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindInFilesDialog(QWidget *parent = nullptr);
    ~FindInFilesDialog();

    void setSearchDirectory(const QString &directory);
    void setSearchText(const QString &text);

signals:
    void fileOpenRequested(const QString &filePath, int lineNumber);

private slots:
    void onFindClicked();
    void onStopClicked();
    void onResultClicked(QTreeWidgetItem *item, int column);
    void onBrowseClicked();
    void onSearchProgress(int current, int total);
    void onResultFound(const QString &filePath, int lineNumber, const QString &lineText);
    void onSearchComplete(int totalMatches);

private:
    void setupUI();
    void startSearch();
    void stopSearch();

    QLineEdit *searchEdit;
    QLineEdit *directoryEdit;
    QLineEdit *filePatternEdit;
    QCheckBox *caseSensitiveCheck;
    QCheckBox *wholeWordsCheck;
    QCheckBox *useRegexCheck;
    QPushButton *findButton;
    QPushButton *stopButton;
    QPushButton *browseButton;
    QTreeWidget *resultsTree;
    QLabel *statusLabel;
    QProgressBar *progressBar;

    QThread *searchThread;
    SearchWorker *searchWorker;
    bool isSearching;
};

#endif // FINDINFILESDIALOG_H
