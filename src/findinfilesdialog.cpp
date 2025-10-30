#include "findinfilesdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>

// SearchWorker implementation
SearchWorker::SearchWorker(QObject *parent)
    : QObject(parent), m_caseSensitive(false), m_wholeWords(false), m_useRegex(false)
{
}

void SearchWorker::setSearchParameters(const QString &searchText, const QString &directory,
                                       const QStringList &filePatterns, bool caseSensitive,
                                       bool wholeWords, bool useRegex)
{
    m_searchText = searchText;
    m_directory = directory;
    m_filePatterns = filePatterns;
    m_caseSensitive = caseSensitive;
    m_wholeWords = wholeWords;
    m_useRegex = useRegex;
}

void SearchWorker::performSearch()
{
    // Get list of files to search
    QList<QString> files = getFilesToSearch(m_directory, m_filePatterns);
    int totalFiles = files.size();
    int totalMatches = 0;

    // Search each file
    for (int i = 0; i < files.size(); ++i) {
        emit searchProgress(i + 1, totalFiles);

        // Check if thread should stop
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        searchInFile(files[i]);
    }

    emit searchComplete(totalMatches);
}

void SearchWorker::searchInFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    int lineNumber = 0;
    QString pattern = m_searchText;

    // Prepare search pattern
    QRegularExpression regex;
    if (m_useRegex) {
        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!m_caseSensitive) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        regex.setPattern(pattern);
        regex.setPatternOptions(options);

        if (!regex.isValid()) {
            return; // Invalid regex
        }
    } else if (m_wholeWords) {
        QString escapedPattern = QRegularExpression::escape(pattern);
        pattern = QString("\\b%1\\b").arg(escapedPattern);
        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!m_caseSensitive) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        regex.setPattern(pattern);
        regex.setPatternOptions(options);
    }

    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine();

        bool found = false;
        if (m_useRegex || m_wholeWords) {
            found = regex.match(line).hasMatch();
        } else {
            found = line.contains(m_searchText, m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }

        if (found) {
            emit resultFound(filePath, lineNumber, line.trimmed());
        }
    }

    file.close();
}

QList<QString> SearchWorker::getFilesToSearch(const QString &directory, const QStringList &patterns)
{
    QList<QString> files;
    QDirIterator it(directory, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);

        if (matchesPattern(fileInfo.fileName(), patterns)) {
            files.append(filePath);
        }
    }

    return files;
}

bool SearchWorker::matchesPattern(const QString &fileName, const QStringList &patterns)
{
    if (patterns.isEmpty() || (patterns.size() == 1 && patterns[0].trimmed().isEmpty())) {
        return true; // No filter, match all
    }

    for (const QString &pattern : patterns) {
        QString trimmedPattern = pattern.trimmed();
        if (trimmedPattern.isEmpty()) {
            continue;
        }

        // Convert wildcard pattern to regex
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(trimmedPattern),
                                QRegularExpression::CaseInsensitiveOption);
        if (regex.match(fileName).hasMatch()) {
            return true;
        }
    }

    return false;
}

// FindInFilesDialog implementation
FindInFilesDialog::FindInFilesDialog(QWidget *parent)
    : QDialog(parent), searchThread(nullptr), searchWorker(nullptr), isSearching(false)
{
    setupUI();
    setWindowTitle(tr("Find in Files"));
    resize(800, 600);
}

FindInFilesDialog::~FindInFilesDialog()
{
    stopSearch();
}

void FindInFilesDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Search options group
    QGroupBox *optionsGroup = new QGroupBox(tr("Search Options"), this);
    QFormLayout *formLayout = new QFormLayout(optionsGroup);

    // Search text
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText(tr("Enter search text..."));
    formLayout->addRow(tr("Find:"), searchEdit);

    // Directory
    QHBoxLayout *dirLayout = new QHBoxLayout();
    directoryEdit = new QLineEdit(this);
    directoryEdit->setPlaceholderText(tr("Select directory..."));
    browseButton = new QPushButton(tr("Browse..."), this);
    dirLayout->addWidget(directoryEdit);
    dirLayout->addWidget(browseButton);
    formLayout->addRow(tr("Directory:"), dirLayout);

    // File patterns
    filePatternEdit = new QLineEdit(this);
    filePatternEdit->setPlaceholderText(tr("*.cpp *.h *.txt (leave empty for all files)"));
    formLayout->addRow(tr("File Patterns:"), filePatternEdit);

    // Checkboxes
    QHBoxLayout *checkLayout = new QHBoxLayout();
    caseSensitiveCheck = new QCheckBox(tr("Case Sensitive"), this);
    wholeWordsCheck = new QCheckBox(tr("Whole Words"), this);
    useRegexCheck = new QCheckBox(tr("Use Regex"), this);
    checkLayout->addWidget(caseSensitiveCheck);
    checkLayout->addWidget(wholeWordsCheck);
    checkLayout->addWidget(useRegexCheck);
    checkLayout->addStretch();
    formLayout->addRow("", checkLayout);

    mainLayout->addWidget(optionsGroup);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    findButton = new QPushButton(tr("Find"), this);
    findButton->setDefault(true);
    stopButton = new QPushButton(tr("Stop"), this);
    stopButton->setEnabled(false);
    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(findButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    // Results tree
    QLabel *resultsLabel = new QLabel(tr("Results:"), this);
    mainLayout->addWidget(resultsLabel);

    resultsTree = new QTreeWidget(this);
    resultsTree->setHeaderLabels(QStringList() << tr("File") << tr("Line") << tr("Text"));
    resultsTree->setColumnWidth(0, 300);
    resultsTree->setColumnWidth(1, 60);
    resultsTree->setRootIsDecorated(false);
    resultsTree->setAlternatingRowColors(true);
    mainLayout->addWidget(resultsTree);

    // Status label
    statusLabel = new QLabel(tr("Ready"), this);
    mainLayout->addWidget(statusLabel);

    // Connect signals
    connect(findButton, &QPushButton::clicked, this, &FindInFilesDialog::onFindClicked);
    connect(stopButton, &QPushButton::clicked, this, &FindInFilesDialog::onStopClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(browseButton, &QPushButton::clicked, this, &FindInFilesDialog::onBrowseClicked);
    connect(resultsTree, &QTreeWidget::itemDoubleClicked, this, &FindInFilesDialog::onResultClicked);
}

void FindInFilesDialog::setSearchDirectory(const QString &directory)
{
    directoryEdit->setText(directory);
}

void FindInFilesDialog::setSearchText(const QString &text)
{
    searchEdit->setText(text);
}

void FindInFilesDialog::onFindClicked()
{
    QString searchText = searchEdit->text();
    QString directory = directoryEdit->text();

    if (searchText.isEmpty()) {
        QMessageBox::warning(this, tr("Find in Files"), tr("Please enter text to search for."));
        return;
    }

    if (directory.isEmpty() || !QDir(directory).exists()) {
        QMessageBox::warning(this, tr("Find in Files"), tr("Please select a valid directory."));
        return;
    }

    startSearch();
}

void FindInFilesDialog::onStopClicked()
{
    stopSearch();
}

void FindInFilesDialog::startSearch()
{
    // Clear previous results
    resultsTree->clear();
    statusLabel->setText(tr("Searching..."));
    progressBar->setVisible(true);
    progressBar->setValue(0);
    isSearching = true;
    findButton->setEnabled(false);
    stopButton->setEnabled(true);

    // Parse file patterns
    QString patternText = filePatternEdit->text().trimmed();
    QStringList patterns;
    if (!patternText.isEmpty()) {
        patterns = patternText.split(' ', Qt::SkipEmptyParts);
    }

    // Create worker thread
    searchThread = new QThread(this);
    searchWorker = new SearchWorker();
    searchWorker->moveToThread(searchThread);

    // Set search parameters
    searchWorker->setSearchParameters(searchEdit->text(), directoryEdit->text(),
                                     patterns, caseSensitiveCheck->isChecked(),
                                     wholeWordsCheck->isChecked(), useRegexCheck->isChecked());

    // Connect signals
    connect(searchThread, &QThread::started, searchWorker, &SearchWorker::performSearch);
    connect(searchWorker, &SearchWorker::searchProgress, this, &FindInFilesDialog::onSearchProgress);
    connect(searchWorker, &SearchWorker::resultFound, this, &FindInFilesDialog::onResultFound);
    connect(searchWorker, &SearchWorker::searchComplete, this, &FindInFilesDialog::onSearchComplete);
    connect(searchThread, &QThread::finished, searchWorker, &QObject::deleteLater);

    // Start search
    searchThread->start();
}

void FindInFilesDialog::stopSearch()
{
    if (searchThread && searchThread->isRunning()) {
        searchThread->requestInterruption();
        searchThread->quit();
        searchThread->wait();
        searchThread->deleteLater();
        searchThread = nullptr;
        searchWorker = nullptr;
    }

    isSearching = false;
    findButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    statusLabel->setText(tr("Search stopped"));
}

void FindInFilesDialog::onResultClicked(QTreeWidgetItem *item, int column)
{
    if (!item) {
        return;
    }

    QString filePath = item->data(0, Qt::UserRole).toString();
    int lineNumber = item->text(1).toInt();

    emit fileOpenRequested(filePath, lineNumber);
}

void FindInFilesDialog::onBrowseClicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                         directoryEdit->text(),
                                                         QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty()) {
        directoryEdit->setText(directory);
    }
}

void FindInFilesDialog::onSearchProgress(int current, int total)
{
    progressBar->setMaximum(total);
    progressBar->setValue(current);
    statusLabel->setText(tr("Searching... (%1 of %2 files)").arg(current).arg(total));
}

void FindInFilesDialog::onResultFound(const QString &filePath, int lineNumber, const QString &lineText)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(resultsTree);

    // Show relative path if possible
    QString displayPath = filePath;
    QString baseDir = directoryEdit->text();
    if (filePath.startsWith(baseDir)) {
        displayPath = filePath.mid(baseDir.length());
        if (displayPath.startsWith('/') || displayPath.startsWith('\\')) {
            displayPath = displayPath.mid(1);
        }
    }

    item->setText(0, displayPath);
    item->setText(1, QString::number(lineNumber));
    item->setText(2, lineText);
    item->setData(0, Qt::UserRole, filePath); // Store full path

    resultsTree->addTopLevelItem(item);
}

void FindInFilesDialog::onSearchComplete(int totalMatches)
{
    isSearching = false;
    findButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);

    int matchCount = resultsTree->topLevelItemCount();
    statusLabel->setText(tr("Search complete. Found %1 match(es) in %2 file(s).")
                        .arg(matchCount)
                        .arg(matchCount)); // This could be improved to count unique files

    // Clean up thread
    if (searchThread) {
        searchThread->quit();
        searchThread->wait();
        searchThread->deleteLater();
        searchThread = nullptr;
        searchWorker = nullptr;
    }
}
