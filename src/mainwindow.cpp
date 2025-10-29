#include "mainwindow.h"
#include <QTextStream>
#include <QStringConverter>
#include <QStandardPaths>
#include <QToolBar>
#include <QDebug>
#include <QFile>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), mainSplitter(nullptr), editorSplitter(nullptr), leftTabWidget(nullptr), rightTabWidget(nullptr),
      tabWidget(nullptr), projectPanel(nullptr), outlinePanel(nullptr), breadcrumbBar(nullptr), mainToolBar(nullptr), languageComboBox(nullptr), syntaxHighlighter(nullptr),
      lineCountLabel(nullptr), wordCountLabel(nullptr), characterCountLabel(nullptr),
      activeTabInfoMap(nullptr), currentViewMode(ViewMode::Single), focusedTabWidget(nullptr),
      projectPanelVisible(false), outlinePanelVisible(false), isSmallScreen(false), autoSaveTimer(nullptr), autoSaveEnabled(true), autoSaveInterval(30), autoSaveAction(nullptr),
      isDarkTheme(false), themeAction(nullptr), lineWrapEnabled(true), wordWrapMode(true), showColumnRuler(false), showWrapIndicator(true), wrapColumn(80),
      lineWrapAction(nullptr), wordWrapAction(nullptr), columnRulerAction(nullptr), wrapIndicatorAction(nullptr),
      minimapEnabled(false), minimapAction(nullptr),
      indentationGuidesEnabled(true), activeIndentHighlightEnabled(true), indentationGuidesAction(nullptr), activeIndentHighlightAction(nullptr),
      trimWhitespaceOnSave(true), autoIndentEnabled(true), autoCloseBracketsEnabled(true), smartBackspaceEnabled(true),
      findDialog(nullptr), goToLineDialog(nullptr), symbolSearchDialog(nullptr)
{
    detectScreenSize();

    // Load theme preference early (before loading stylesheet)
    QSettings settings;
    isDarkTheme = settings.value("isDarkTheme", false).toBool();

    loadStyleSheet();
    setupEditor();
    setupMenus();
    setupToolBar();
    setupStatusBar();
    setupAutoSave();
    setupResponsiveUI();
    loadSettings();

    setWindowTitle(tr("Bearbeiten"));
    resize(800, 600);

    // Create first tab
    createNewTab();
}

MainWindow::~MainWindow()
{
    saveSettings();
    if (autoSaveTimer) {
        autoSaveTimer->stop();
        delete autoSaveTimer;
    }
}

void MainWindow::setupEditor()
{
    // Create main horizontal splitter (project panel | editor area)
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    // Create project panel
    projectPanel = new ProjectPanel();
    projectPanel->setMaximumWidth(300);
    projectPanel->setMinimumWidth(200);
    mainSplitter->addWidget(projectPanel);

    // Connect project panel signals
    connect(projectPanel, &ProjectPanel::fileRequested, this, &MainWindow::openProjectFromPanel);

    // Create editor container with breadcrumb
    QWidget *editorContainer = new QWidget();
    QVBoxLayout *editorLayout = new QVBoxLayout(editorContainer);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    // Create breadcrumb bar
    breadcrumbBar = new BreadcrumbBar();
    editorLayout->addWidget(breadcrumbBar);

    // Create editor splitter for side-by-side view
    editorSplitter = new QSplitter(Qt::Horizontal);
    editorLayout->addWidget(editorSplitter);

    mainSplitter->addWidget(editorContainer);

    // Create outline panel
    outlinePanel = new OutlinePanel();
    outlinePanel->setMaximumWidth(300);
    outlinePanel->setMinimumWidth(200);
    mainSplitter->addWidget(outlinePanel);

    // Connect outline panel signals
    connect(outlinePanel, &OutlinePanel::symbolClicked, this, &MainWindow::jumpToSymbolFromOutline);

    // Create left tab widget (main/single view)
    leftTabWidget = new QTabWidget();
    leftTabWidget->setTabsClosable(true);
    leftTabWidget->setMovable(true);
    leftTabWidget->setDocumentMode(true);
    editorSplitter->addWidget(leftTabWidget);

    // Create right tab widget (for split view)
    rightTabWidget = new QTabWidget();
    rightTabWidget->setTabsClosable(true);
    rightTabWidget->setMovable(true);
    rightTabWidget->setDocumentMode(true);
    editorSplitter->addWidget(rightTabWidget);

    // Initially hide the right pane, project panel, and outline panel
    rightTabWidget->hide();
    projectPanel->hide();
    outlinePanel->hide();

    // Set initial active tab widget
    tabWidget = leftTabWidget;
    focusedTabWidget = leftTabWidget;
    activeTabInfoMap = &leftTabInfoMap;

    // Setup splitters
    mainSplitter->setSizes({250, 600, 250});
    editorSplitter->setSizes({400, 400});
    mainSplitter->setCollapsible(0, true);  // Project panel can be collapsed
    mainSplitter->setCollapsible(1, false); // Editor area cannot be collapsed
    mainSplitter->setCollapsible(2, true);  // Outline panel can be collapsed
    editorSplitter->setCollapsible(0, false);
    editorSplitter->setCollapsible(1, false);

    setupSplitView();

    // Setup JSON-based syntax highlighter (will be created per tab)
    syntaxHighlighter = new JsonSyntaxHighlighter();
    syntaxHighlighter->loadLanguages("languages");
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *newAction = new QAction(tr("&New"), this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(tr("&Open"), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);

    fileMenu->addSeparator();

    QAction *saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApp);
    fileMenu->addAction(exitAction);

    // Edit menu for undo/redo functionality
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction *undoAction = new QAction(tr("&Undo"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    editMenu->addAction(undoAction);

    QAction *redoAction = new QAction(tr("&Redo"), this);
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);
    editMenu->addAction(redoAction);

    editMenu->addSeparator();

    QAction *cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &MainWindow::cut);
    editMenu->addAction(cutAction);

    QAction *copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MainWindow::copy);
    editMenu->addAction(copyAction);

    QAction *pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &MainWindow::paste);
    editMenu->addAction(pasteAction);

    editMenu->addSeparator();

    QAction *findAction = new QAction(tr("&Find..."), this);
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::showFindDialog);
    editMenu->addAction(findAction);

    QAction *replaceAction = new QAction(tr("&Replace..."), this);
    replaceAction->setShortcut(QKeySequence::Replace);
    connect(replaceAction, &QAction::triggered, this, &MainWindow::showReplaceDialog);
    editMenu->addAction(replaceAction);

    editMenu->addSeparator();

    QAction *goToLineAction = new QAction(tr("&Go to Line..."), this);
    goToLineAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(goToLineAction, &QAction::triggered, this, &MainWindow::showGoToLineDialog);
    editMenu->addAction(goToLineAction);

    QAction *goToSymbolAction = new QAction(tr("Go to &Symbol..."), this);
    goToSymbolAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(goToSymbolAction, &QAction::triggered, this, &MainWindow::showSymbolSearchDialog);
    editMenu->addAction(goToSymbolAction);

    // View menu for split functionality
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    QAction *toggleSplitAction = new QAction(tr("&Toggle Split View"), this);
    toggleSplitAction->setShortcut(QKeySequence("Ctrl+\\"));
    connect(toggleSplitAction, &QAction::triggered, this, &MainWindow::toggleSplitView);
    viewMenu->addAction(toggleSplitAction);

    QAction *splitHorizontalAction = new QAction(tr("Split &Horizontally"), this);
    splitHorizontalAction->setShortcut(QKeySequence("Ctrl+Shift+H"));
    connect(splitHorizontalAction, &QAction::triggered, this, &MainWindow::splitHorizontally);
    viewMenu->addAction(splitHorizontalAction);

    QAction *splitVerticalAction = new QAction(tr("Split &Vertically"), this);
    splitVerticalAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
    connect(splitVerticalAction, &QAction::triggered, this, &MainWindow::splitVertically);
    viewMenu->addAction(splitVerticalAction);

    viewMenu->addSeparator();

    QAction *closeSplitAction = new QAction(tr("&Close Split View"), this);
    closeSplitAction->setShortcut(QKeySequence("Ctrl+Shift+W"));
    connect(closeSplitAction, &QAction::triggered, this, &MainWindow::closeSplitView);
    viewMenu->addAction(closeSplitAction);

    viewMenu->addSeparator();

    QAction *toggleProjectAction = new QAction(tr("&Project Panel"), this);
    toggleProjectAction->setShortcut(QKeySequence("Ctrl+Shift+E"));
    toggleProjectAction->setCheckable(true);
    toggleProjectAction->setChecked(projectPanelVisible);
    connect(toggleProjectAction, &QAction::triggered, this, &MainWindow::toggleProjectPanel);
    viewMenu->addAction(toggleProjectAction);

    QAction *toggleOutlineAction = new QAction(tr("&Outline Panel"), this);
    toggleOutlineAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    toggleOutlineAction->setCheckable(true);
    toggleOutlineAction->setChecked(outlinePanelVisible);
    connect(toggleOutlineAction, &QAction::triggered, this, &MainWindow::toggleOutlinePanel);
    viewMenu->addAction(toggleOutlineAction);

    viewMenu->addSeparator();

    themeAction = new QAction(tr("&Dark Theme"), this);
    themeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    themeAction->setCheckable(true);
    themeAction->setChecked(isDarkTheme);
    connect(themeAction, &QAction::triggered, this, &MainWindow::toggleTheme);
    viewMenu->addAction(themeAction);

    viewMenu->addSeparator();

    lineWrapAction = new QAction(tr("&Line Wrap"), this);
    lineWrapAction->setShortcut(QKeySequence("Alt+Z"));
    lineWrapAction->setCheckable(true);
    lineWrapAction->setChecked(lineWrapEnabled);
    connect(lineWrapAction, &QAction::triggered, this, &MainWindow::toggleLineWrap);
    viewMenu->addAction(lineWrapAction);

    wordWrapAction = new QAction(tr("Word Wrap Mode"), this);
    wordWrapAction->setCheckable(true);
    wordWrapAction->setChecked(wordWrapMode);
    wordWrapAction->setToolTip(tr("Wrap at word boundaries (unchecked = wrap anywhere)"));
    connect(wordWrapAction, &QAction::triggered, this, &MainWindow::toggleWordWrapMode);
    viewMenu->addAction(wordWrapAction);

    wrapIndicatorAction = new QAction(tr("Show Wrap Indicators"), this);
    wrapIndicatorAction->setCheckable(true);
    wrapIndicatorAction->setChecked(showWrapIndicator);
    wrapIndicatorAction->setToolTip(tr("Show arrow indicators for wrapped lines"));
    connect(wrapIndicatorAction, &QAction::triggered, this, &MainWindow::toggleWrapIndicator);
    viewMenu->addAction(wrapIndicatorAction);

    columnRulerAction = new QAction(tr("Show Column Ruler"), this);
    columnRulerAction->setCheckable(true);
    columnRulerAction->setChecked(showColumnRuler);
    columnRulerAction->setToolTip(tr("Show vertical ruler at wrap column"));
    connect(columnRulerAction, &QAction::triggered, this, &MainWindow::toggleColumnRuler);
    viewMenu->addAction(columnRulerAction);

    QAction *setWrapColumnAction = new QAction(tr("Set Wrap Column..."), this);
    setWrapColumnAction->setToolTip(tr("Set the column position for the ruler (default: 80)"));
    connect(setWrapColumnAction, &QAction::triggered, this, &MainWindow::setWrapColumn);
    viewMenu->addAction(setWrapColumnAction);

    viewMenu->addSeparator();

    QAction *foldAction = new QAction(tr("&Fold Block"), this);
    foldAction->setShortcut(QKeySequence("Ctrl+Shift+["));
    connect(foldAction, &QAction::triggered, this, &MainWindow::foldCurrentBlock);
    viewMenu->addAction(foldAction);

    QAction *unfoldAction = new QAction(tr("&Unfold Block"), this);
    unfoldAction->setShortcut(QKeySequence("Ctrl+Shift+]"));
    connect(unfoldAction, &QAction::triggered, this, &MainWindow::unfoldCurrentBlock);
    viewMenu->addAction(unfoldAction);

    QAction *foldAllAction = new QAction(tr("Fold &All"), this);
    foldAllAction->setShortcut(QKeySequence("Ctrl+K, Ctrl+0"));
    connect(foldAllAction, &QAction::triggered, this, &MainWindow::foldAll);
    viewMenu->addAction(foldAllAction);

    QAction *unfoldAllAction = new QAction(tr("Unfold A&ll"), this);
    unfoldAllAction->setShortcut(QKeySequence("Ctrl+K, Ctrl+J"));
    connect(unfoldAllAction, &QAction::triggered, this, &MainWindow::unfoldAll);
    viewMenu->addAction(unfoldAllAction);

    viewMenu->addSeparator();

    minimapAction = new QAction(tr("Show &Minimap"), this);
    minimapAction->setCheckable(true);
    minimapAction->setChecked(minimapEnabled);
    minimapAction->setToolTip(tr("Show document minimap sidebar"));
    connect(minimapAction, &QAction::triggered, this, &MainWindow::toggleMinimap);
    viewMenu->addAction(minimapAction);

    viewMenu->addSeparator();

    indentationGuidesAction = new QAction(tr("Show &Indentation Guides"), this);
    indentationGuidesAction->setCheckable(true);
    indentationGuidesAction->setChecked(indentationGuidesEnabled);
    indentationGuidesAction->setToolTip(tr("Show vertical lines at indentation levels"));
    connect(indentationGuidesAction, &QAction::triggered, this, &MainWindow::toggleIndentationGuides);
    viewMenu->addAction(indentationGuidesAction);

    activeIndentHighlightAction = new QAction(tr("Highlight Active Indent"), this);
    activeIndentHighlightAction->setCheckable(true);
    activeIndentHighlightAction->setChecked(activeIndentHighlightEnabled);
    activeIndentHighlightAction->setToolTip(tr("Highlight the indent level at cursor position"));
    connect(activeIndentHighlightAction, &QAction::triggered, this, &MainWindow::toggleActiveIndentHighlight);
    viewMenu->addAction(activeIndentHighlightAction);

    // Tools menu for auto-save
    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));

    autoSaveAction = new QAction(tr("&Auto Save"), this);
    autoSaveAction->setCheckable(true);
    autoSaveAction->setChecked(autoSaveEnabled);
    connect(autoSaveAction, &QAction::triggered, this, &MainWindow::toggleAutoSave);
    toolsMenu->addAction(autoSaveAction);
}

void MainWindow::setupToolBar()
{
    mainToolBar = addToolBar(tr("Main"));
    mainToolBar->setMovable(false);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // Project panel toggle action
    QAction *toggleProjectAction = new QAction("📁", this);
    toggleProjectAction->setToolTip(tr("Toggle Project Panel (Ctrl+Shift+E)"));
    toggleProjectAction->setCheckable(true);
    toggleProjectAction->setChecked(projectPanelVisible);
    connect(toggleProjectAction, &QAction::triggered, this, &MainWindow::toggleProjectPanel);
    mainToolBar->addAction(toggleProjectAction);

    mainToolBar->addSeparator();

    // Add syntax language chooser to toolbar
    QLabel *languageLabel = new QLabel(tr("Syntax: "));
    mainToolBar->addWidget(languageLabel);

    languageComboBox = new QComboBox();
    languageComboBox->setMinimumWidth(100);
    languageComboBox->setToolTip(tr("Select syntax highlighting language"));

    // Add language options from JSON configurations
    languageComboBox->addItem(tr("None"), "");
    QStringList availableLanguages = syntaxHighlighter->getAvailableLanguages();
    for (const QString &language : availableLanguages) {
        languageComboBox->addItem(language, language);
    }

    connect(languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);
    mainToolBar->addWidget(languageComboBox);
}

void MainWindow::newFile()
{
    createNewTab();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("Text Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

bool MainWindow::saveFile()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return false;

    QString currentFile = getFilePathAt(currentIndex);
    if (currentFile.isEmpty()) {
        return saveAsFile();
    } else {
        return saveDocument(currentFile);
    }
}

bool MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save File"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("Text Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        return saveDocument(fileName);
    }
    return false;
}

void MainWindow::exitApp()
{
    if (maybeSave()) {
        QApplication::quit();
    }
}

bool MainWindow::maybeSave()
{
    // Check all tabs for unsaved changes
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (!maybeSaveTab(i)) {
            return false; // User cancelled
        }
    }
    return true;
}

bool MainWindow::saveDocument(const QString &fileName)
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return false;

    // Trim trailing whitespace if enabled
    if (trimWhitespaceOnSave) {
        editor->trimTrailingWhitespace();
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Bearbeiten",
            QString("Cannot write file %1:\n%2")
            .arg(fileName)
            .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << editor->toPlainText();

    setCurrentFile(fileName);
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Bearbeiten",
            QString("Cannot read file %1:\n%2")
            .arg(fileName)
            .arg(file.errorString()));
        return;
    }

    // Create new tab for this file
    createNewTab(fileName);

    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        editor->setPlainText(in.readAll());

        setCurrentFile(fileName);

        // Auto-detect and set syntax highlighting based on file extension
        int currentTabIndex = tabWidget->currentIndex();
        if (currentTabIndex >= 0 && activeTabInfoMap->contains(currentTabIndex)) {
            JsonSyntaxHighlighter *highlighter = (*activeTabInfoMap)[currentTabIndex].highlighter;
            if (highlighter) {
                highlighter->setLanguageFromFilename(fileName);

                // Update the combo box to show the detected language
                QString detectedLanguage = highlighter->getCurrentLanguage();
                if (!detectedLanguage.isEmpty()) {
                    for (int i = 0; i < languageComboBox->count(); ++i) {
                        if (languageComboBox->itemData(i).toString().toLower() == detectedLanguage.toLower()) {
                            languageComboBox->setCurrentIndex(i);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0) {
        setFilePathAt(currentIndex, fileName);
        setTabModified(currentIndex, false);
    }
}

void MainWindow::loadStyleSheet()
{
    // Choose stylesheet based on theme
    QString styleFileName = isDarkTheme ? ":/src/stylesheet-dark.qss" : ":/src/stylesheet.qss";
    QFile styleFile(styleFileName);

    if (!styleFile.exists()) {
        // Try relative path for development
        styleFile.setFileName(isDarkTheme ? "src/stylesheet-dark.qss" : "src/stylesheet.qss");
    }

    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "Failed to load stylesheet:" << styleFile.fileName();
    }
}

void MainWindow::setupStatusBar()
{
    // Create status bar - styling is handled by the stylesheet
    statusBar()->showMessage(tr("Ready"));

    // Create line and word count widget
    QWidget *statusWidget = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(6);

    lineCountLabel = new QLabel(tr("Lines: 1"));
    wordCountLabel = new QLabel(tr("Words: 0"));
    characterCountLabel = new QLabel(tr("Characters: 0"));

    statusLayout->addWidget(lineCountLabel);
    statusLayout->addWidget(new QLabel(" | "));
    statusLayout->addWidget(wordCountLabel);
    statusLayout->addWidget(new QLabel(" | "));
    statusLayout->addWidget(characterCountLabel);

    // Add the widget to the status bar
    statusBar()->addPermanentWidget(statusWidget);
}

void MainWindow::onLanguageChanged(int index)
{
    int currentTabIndex = tabWidget->currentIndex();
    if (currentTabIndex >= 0 && languageComboBox && activeTabInfoMap->contains(currentTabIndex)) {
        QString languageName = languageComboBox->itemData(index).toString();
        JsonSyntaxHighlighter *highlighter = (*activeTabInfoMap)[currentTabIndex].highlighter;
        if (highlighter) {
            highlighter->setLanguage(languageName);
        }
    }
}

// Tab management methods
void MainWindow::createNewTab(const QString &fileName)
{
    CodeEditor *editor = new CodeEditor();

    // Apply responsive settings based on screen size
    QFont font = editor->font();
    if (isSmallScreen) {
        font.setPointSize(9);
        editor->setCompactMode(true);
    } else {
        font.setPointSize(11);
        editor->setCompactMode(false);
    }
    editor->setFont(font);

    // Apply line wrap settings
    QPlainTextEdit::LineWrapMode wrapMode = lineWrapEnabled ?
        QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap;
    editor->setLineWrapMode(wrapMode);

    QTextOption::WrapMode wordWrap = wordWrapMode ?
        QTextOption::WordWrap : QTextOption::WrapAnywhere;
    editor->setWordWrapMode(wordWrap);

    editor->setShowWrapIndicator(showWrapIndicator);
    editor->setShowColumnRuler(showColumnRuler);
    editor->setWrapColumn(wrapColumn);

    // Apply smart editing settings
    editor->setAutoIndent(autoIndentEnabled);
    editor->setAutoCloseBrackets(autoCloseBracketsEnabled);
    editor->setSmartBackspace(smartBackspaceEnabled);

    // Apply indentation guide settings
    editor->setShowIndentationGuides(indentationGuidesEnabled);
    editor->setHighlightActiveIndent(activeIndentHighlightEnabled);

    // Create syntax highlighter for this tab
    JsonSyntaxHighlighter *highlighter = new JsonSyntaxHighlighter(editor->document());
    highlighter->loadLanguages("languages");
    highlighter->setTheme(isDarkTheme);

    // Create minimap for this tab
    Minimap *minimap = new Minimap(editor);
    minimap->setVisible(minimapEnabled);

    // Create container widget with editor and minimap
    QWidget *container = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(editor);
    layout->addWidget(minimap);
    container->setLayout(layout);

    QString tabTitle = fileName.isEmpty() ? tr("Untitled") : QFileInfo(fileName).fileName();
    int index = tabWidget->addTab(container, tabTitle);

    // Store tab information
    (*activeTabInfoMap)[index] = TabInfo(fileName, highlighter, minimap);

    // Connect text changed signal for this editor
    connect(editor, &CodeEditor::textChanged, [this, index]() {
        setTabModified(index, true);
        onTextChanged(); // Trigger auto-save timer reset
        updateStatusBar(); // Update line/word count
    });

    // Connect cursor position changed signal to update breadcrumb
    connect(editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateBreadcrumbSymbol);

    // Set this tab as current
    tabWidget->setCurrentIndex(index);
}

void MainWindow::closeTab(int index)
{
    if (index < 0 || index >= tabWidget->count()) return;

    if (maybeSaveTab(index)) {
        QWidget *widget = tabWidget->widget(index);

        // Clean up highlighter
        if (activeTabInfoMap->contains(index)) {
            delete (*activeTabInfoMap)[index].highlighter;
            activeTabInfoMap->remove(index);
        }

        tabWidget->removeTab(index);
        delete widget;

        // Update indices in activeTabInfoMap (shift them down)
        QMap<int, TabInfo> newTabInfoMap;
        for (auto it = activeTabInfoMap->begin(); it != activeTabInfoMap->end(); ++it) {
            int oldIndex = it.key();
            if (oldIndex > index) {
                newTabInfoMap[oldIndex - 1] = it.value();
            } else {
                newTabInfoMap[oldIndex] = it.value();
            }
        }
        *activeTabInfoMap = newTabInfoMap;

        // If no tabs left, create a new one
        if (tabWidget->count() == 0) {
            createNewTab();
        }
    }
}

void MainWindow::closeOtherTabs(int index)
{
    // Close tabs after the specified index (in reverse order)
    for (int i = tabWidget->count() - 1; i > index; --i) {
        closeTab(i);
    }

    // Close tabs before the specified index (in reverse order)
    for (int i = index - 1; i >= 0; --i) {
        closeTab(i);
    }
}

void MainWindow::closeAllTabs()
{
    for (int i = tabWidget->count() - 1; i >= 0; --i) {
        closeTab(i);
    }
}

CodeEditor* MainWindow::getCurrentEditor()
{
    QWidget *container = tabWidget->currentWidget();
    if (!container) {
        return nullptr;
    }

    // The container has a layout with the editor as the first widget
    QLayout *layout = container->layout();
    if (layout && layout->count() > 0) {
        return qobject_cast<CodeEditor*>(layout->itemAt(0)->widget());
    }

    return nullptr;
}

CodeEditor* MainWindow::getEditorAt(int index)
{
    QWidget *container = tabWidget->widget(index);
    if (!container) {
        return nullptr;
    }

    // The container has a layout with the editor as the first widget
    QLayout *layout = container->layout();
    if (layout && layout->count() > 0) {
        return qobject_cast<CodeEditor*>(layout->itemAt(0)->widget());
    }

    return nullptr;
}

QString MainWindow::getFilePathAt(int index)
{
    return activeTabInfoMap->value(index).filePath;
}

void MainWindow::setFilePathAt(int index, const QString &filePath)
{
    if (activeTabInfoMap->contains(index)) {
        (*activeTabInfoMap)[index].filePath = filePath;
        updateTabTitle(index);
    }
}

bool MainWindow::isTabModified(int index)
{
    QString title = tabWidget->tabText(index);
    return title.endsWith(" *");
}

void MainWindow::setTabModified(int index, bool modified)
{
    QString title = tabWidget->tabText(index);
    title = title.replace(" *", ""); // Remove existing asterisk

    if (modified) {
        title += " *";
    }

    tabWidget->setTabText(index, title);

    // Update window title if this is the current tab
    if (index == tabWidget->currentIndex()) {
        QString filePath = getFilePathAt(index);
        QString fileName = filePath.isEmpty() ? "Untitled" : QFileInfo(filePath).fileName();
        setWindowTitle(QString("Bearbeiten - %1%2").arg(fileName).arg(modified ? " *" : ""));
    }
}

void MainWindow::updateTabTitle(int index)
{
    QString filePath = getFilePathAt(index);
    QString fileName = filePath.isEmpty() ? "Untitled" : QFileInfo(filePath).fileName();
    bool modified = isTabModified(index);

    tabWidget->setTabText(index, fileName + (modified ? " *" : ""));

    // Update window title if this is the current tab
    if (index == tabWidget->currentIndex()) {
        setWindowTitle(QString("Bearbeiten - %1%2").arg(fileName).arg(modified ? " *" : ""));
    }
}

bool MainWindow::maybeSaveTab(int tabIndex)
{
    if (!isTabModified(tabIndex)) {
        return true;
    }

    QString fileName = getFilePathAt(tabIndex);
    QString displayName = fileName.isEmpty() ? "Untitled" : QFileInfo(fileName).fileName();

    QMessageBox::StandardButton result = QMessageBox::warning(this,
        "Bearbeiten",
        QString("The document '%1' has been modified.\nDo you want to save your changes?").arg(displayName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (result) {
    case QMessageBox::Save:
        {
            int currentTab = tabWidget->currentIndex();
            tabWidget->setCurrentIndex(tabIndex);
            bool saved = saveFile() != false;
            tabWidget->setCurrentIndex(currentTab);
            return saved;
        }
    case QMessageBox::Cancel:
        return false;
    default:
        return true;
    }
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
        updateTabTitle(index);

        // Update language combo box for current tab
        if (activeTabInfoMap->contains(index)) {
            JsonSyntaxHighlighter *highlighter = (*activeTabInfoMap)[index].highlighter;
            if (highlighter) {
                QString currentLanguage = highlighter->getCurrentLanguage();

                // Update combo box selection
                for (int i = 0; i < languageComboBox->count(); ++i) {
                    if (languageComboBox->itemData(i).toString().toLower() == currentLanguage.toLower()) {
                        languageComboBox->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }

        // Update status bar for the new tab
        updateStatusBar();

        // Update outline panel for the new tab
        updateOutlinePanel();

        // Update breadcrumb bar for the new tab
        updateBreadcrumb();
    }
}

void MainWindow::onTabCloseRequested(int index)
{
    closeTab(index);
}

void MainWindow::onTabContextMenu(const QPoint &pos)
{
    int tabIndex = tabWidget->tabBar()->tabAt(pos);
    if (tabIndex >= 0) {
        QMenu contextMenu;

        QAction *closeAction = contextMenu.addAction("Close Tab");
        QAction *closeOthersAction = contextMenu.addAction("Close Other Tabs");
        QAction *closeAllAction = contextMenu.addAction("Close All Tabs");

        closeOthersAction->setEnabled(tabWidget->count() > 1);

        QAction *selectedAction = contextMenu.exec(tabWidget->tabBar()->mapToGlobal(pos));

        if (selectedAction == closeAction) {
            closeTab(tabIndex);
        } else if (selectedAction == closeOthersAction) {
            closeOtherTabs(tabIndex);
        } else if (selectedAction == closeAllAction) {
            closeAllTabs();
        }
    }
}

// Split view implementation
void MainWindow::setupSplitView()
{
    // Connect signals for both tab widgets
    auto connectTabWidget = [this](QTabWidget *widget, QMap<int, TabInfo> *infoMap) {
        connect(widget, &QTabWidget::currentChanged, [this, widget](int index) {
            if (widget == focusedTabWidget) {
                onTabChanged(index);
            }
        });

        connect(widget, &QTabWidget::tabCloseRequested, [this, widget](int index) {
            // Temporarily switch context for close operation
            QTabWidget *oldActive = tabWidget;
            QMap<int, TabInfo> *oldMap = activeTabInfoMap;

            tabWidget = widget;
            activeTabInfoMap = (widget == leftTabWidget) ? &leftTabInfoMap : &rightTabInfoMap;

            closeTab(index);

            // Restore context
            tabWidget = oldActive;
            activeTabInfoMap = oldMap;
        });

        widget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(widget->tabBar(), &QTabBar::customContextMenuRequested, [this, widget](const QPoint &pos) {
            // Temporarily switch context for context menu
            QTabWidget *oldActive = tabWidget;
            QMap<int, TabInfo> *oldMap = activeTabInfoMap;

            tabWidget = widget;
            activeTabInfoMap = (widget == leftTabWidget) ? &leftTabInfoMap : &rightTabInfoMap;

            onTabContextMenu(pos);

            // Restore context
            tabWidget = oldActive;
            activeTabInfoMap = oldMap;
        });

        // Focus tracking
        connect(widget, &QTabWidget::currentChanged, [this, widget](int) {
            focusedTabWidget = widget;
            tabWidget = widget;
            activeTabInfoMap = (widget == leftTabWidget) ? &leftTabInfoMap : &rightTabInfoMap;
            syncLanguageComboBox();
        });
    };

    connectTabWidget(leftTabWidget, &leftTabInfoMap);
    connectTabWidget(rightTabWidget, &rightTabInfoMap);
}

void MainWindow::toggleSplitView()
{
    if (currentViewMode == ViewMode::Single) {
        splitVertically();
    } else {
        closeSplitView();
    }
}

void MainWindow::splitHorizontally()
{
    if (currentViewMode == ViewMode::Single) {
        editorSplitter->setOrientation(Qt::Vertical);
        rightTabWidget->show();
        currentViewMode = ViewMode::SideBySide;

        // Create a new tab in the right pane if it's empty
        if (rightTabWidget->count() == 0) {
            QTabWidget *oldActive = tabWidget;
            QMap<int, TabInfo> *oldMap = activeTabInfoMap;

            tabWidget = rightTabWidget;
            activeTabInfoMap = &rightTabInfoMap;

            createNewTab();

            tabWidget = oldActive;
            activeTabInfoMap = oldMap;
        }

        updateViewMode();
    }
}

void MainWindow::splitVertically()
{
    if (currentViewMode == ViewMode::Single) {
        editorSplitter->setOrientation(Qt::Horizontal);
        rightTabWidget->show();
        currentViewMode = ViewMode::SideBySide;

        // Create a new tab in the right pane if it's empty
        if (rightTabWidget->count() == 0) {
            QTabWidget *oldActive = tabWidget;
            QMap<int, TabInfo> *oldMap = activeTabInfoMap;

            tabWidget = rightTabWidget;
            activeTabInfoMap = &rightTabInfoMap;

            createNewTab();

            tabWidget = oldActive;
            activeTabInfoMap = oldMap;
        }

        updateViewMode();
    }
}

void MainWindow::closeSplitView()
{
    if (currentViewMode == ViewMode::SideBySide) {
        rightTabWidget->hide();
        currentViewMode = ViewMode::Single;

        // Switch focus back to left pane
        focusedTabWidget = leftTabWidget;
        tabWidget = leftTabWidget;
        activeTabInfoMap = &leftTabInfoMap;

        updateViewMode();
        syncLanguageComboBox();
    }
}

void MainWindow::onSplitterFocusChanged()
{
    syncLanguageComboBox();
}

void MainWindow::updateViewMode()
{
    if (currentViewMode == ViewMode::SideBySide) {
        editorSplitter->setSizes({400, 400});
    }
}

QTabWidget* MainWindow::getActiveTabWidget()
{
    return focusedTabWidget ? focusedTabWidget : leftTabWidget;
}

QTabWidget* MainWindow::getInactiveTabWidget()
{
    return (focusedTabWidget == leftTabWidget) ? rightTabWidget : leftTabWidget;
}

void MainWindow::syncLanguageComboBox()
{
    if (!focusedTabWidget || !languageComboBox) return;

    int currentIndex = focusedTabWidget->currentIndex();
    if (currentIndex < 0) return;

    QMap<int, TabInfo> *currentMap = (focusedTabWidget == leftTabWidget) ? &leftTabInfoMap : &rightTabInfoMap;

    if (currentMap->contains(currentIndex)) {
        JsonSyntaxHighlighter *highlighter = (*currentMap)[currentIndex].highlighter;
        if (highlighter) {
            QString currentLanguage = highlighter->getCurrentLanguage();

            // Update combo box selection
            for (int i = 0; i < languageComboBox->count(); ++i) {
                if (languageComboBox->itemData(i).toString().toLower() == currentLanguage.toLower()) {
                    languageComboBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void MainWindow::setupAutoSave()
{
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(false);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
}

void MainWindow::startAutoSaveTimer()
{
    if (autoSaveTimer && autoSaveEnabled) {
        autoSaveTimer->start(autoSaveInterval * 1000); // Convert to milliseconds
    }
}

void MainWindow::stopAutoSaveTimer()
{
    if (autoSaveTimer) {
        autoSaveTimer->stop();
    }
}

void MainWindow::onTextChanged()
{
    // Reset the auto-save timer when text changes
    if (autoSaveEnabled) {
        stopAutoSaveTimer();
        startAutoSaveTimer();
    }

    // Update outline panel (with debouncing via timer)
    updateOutlinePanel();
}

void MainWindow::autoSave()
{
    if (!autoSaveEnabled) return;

    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;

    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return;

    QString filePath = getFilePathAt(currentIndex);

    // Only auto-save if the file has been saved before (has a file path)
    if (!filePath.isEmpty() && isTabModified(currentIndex)) {
        saveDocument(filePath);
        setTabModified(currentIndex, false);
        updateTabTitle(currentIndex);
    }
}

void MainWindow::toggleAutoSave()
{
    autoSaveEnabled = !autoSaveEnabled;
    if (autoSaveAction) {
        autoSaveAction->setChecked(autoSaveEnabled);
    }

    if (autoSaveEnabled) {
        startAutoSaveTimer();
    } else {
        stopAutoSaveTimer();
    }

    saveSettings();
}

void MainWindow::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    if (themeAction) {
        themeAction->setChecked(isDarkTheme);
    }

    loadStyleSheet();

    // Update all syntax highlighters with new theme
    for (auto it = leftTabInfoMap.begin(); it != leftTabInfoMap.end(); ++it) {
        if (it.value().highlighter) {
            it.value().highlighter->setTheme(isDarkTheme);
        }
    }

    for (auto it = rightTabInfoMap.begin(); it != rightTabInfoMap.end(); ++it) {
        if (it.value().highlighter) {
            it.value().highlighter->setTheme(isDarkTheme);
        }
    }

    saveSettings();
}

void MainWindow::toggleLineWrap()
{
    lineWrapEnabled = !lineWrapEnabled;
    if (lineWrapAction) {
        lineWrapAction->setChecked(lineWrapEnabled);
    }

    // Apply to all open editors
    QPlainTextEdit::LineWrapMode mode = lineWrapEnabled ?
        QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap;

    // Update left tab widget editors
    for (int i = 0; i < leftTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(leftTabWidget->widget(i));
        if (editor) {
            editor->setLineWrapMode(mode);
        }
    }

    // Update right tab widget editors (if in split view)
    for (int i = 0; i < rightTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(rightTabWidget->widget(i));
        if (editor) {
            editor->setLineWrapMode(mode);
        }
    }

    saveSettings();
}

void MainWindow::setLineWrapMode(int mode)
{
    // This method allows for future expansion to support different wrap modes
    // For now, we just toggle between wrap and no-wrap
    lineWrapEnabled = (mode != 0);
    toggleLineWrap();
}

void MainWindow::toggleWordWrapMode()
{
    wordWrapMode = !wordWrapMode;
    if (wordWrapAction) {
        wordWrapAction->setChecked(wordWrapMode);
    }

    // Apply to all open editors
    QTextOption::WrapMode wrapMode = wordWrapMode ?
        QTextOption::WordWrap : QTextOption::WrapAnywhere;

    // Update left tab widget editors
    for (int i = 0; i < leftTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(leftTabWidget->widget(i));
        if (editor) {
            editor->setWordWrapMode(wrapMode);
        }
    }

    // Update right tab widget editors (if in split view)
    for (int i = 0; i < rightTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(rightTabWidget->widget(i));
        if (editor) {
            editor->setWordWrapMode(wrapMode);
        }
    }

    saveSettings();
}

void MainWindow::toggleColumnRuler()
{
    showColumnRuler = !showColumnRuler;
    if (columnRulerAction) {
        columnRulerAction->setChecked(showColumnRuler);
    }

    // Apply to all open editors
    for (int i = 0; i < leftTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(leftTabWidget->widget(i));
        if (editor) {
            editor->setShowColumnRuler(showColumnRuler);
        }
    }

    for (int i = 0; i < rightTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(rightTabWidget->widget(i));
        if (editor) {
            editor->setShowColumnRuler(showColumnRuler);
        }
    }

    saveSettings();
}

void MainWindow::toggleWrapIndicator()
{
    showWrapIndicator = !showWrapIndicator;
    if (wrapIndicatorAction) {
        wrapIndicatorAction->setChecked(showWrapIndicator);
    }

    // Apply to all open editors
    for (int i = 0; i < leftTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(leftTabWidget->widget(i));
        if (editor) {
            editor->setShowWrapIndicator(showWrapIndicator);
        }
    }

    for (int i = 0; i < rightTabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(rightTabWidget->widget(i));
        if (editor) {
            editor->setShowWrapIndicator(showWrapIndicator);
        }
    }

    saveSettings();
}

void MainWindow::setWrapColumn()
{
    bool ok;
    int newColumn = QInputDialog::getInt(this, tr("Set Wrap Column"),
                                         tr("Column number (characters):"),
                                         wrapColumn, 40, 200, 1, &ok);
    if (ok) {
        wrapColumn = newColumn;

        // Apply to all open editors
        for (int i = 0; i < leftTabWidget->count(); ++i) {
            CodeEditor *editor = qobject_cast<CodeEditor*>(leftTabWidget->widget(i));
            if (editor) {
                editor->setWrapColumn(wrapColumn);
            }
        }

        for (int i = 0; i < rightTabWidget->count(); ++i) {
            CodeEditor *editor = qobject_cast<CodeEditor*>(rightTabWidget->widget(i));
            if (editor) {
                editor->setWrapColumn(wrapColumn);
            }
        }

        saveSettings();
    }
}

void MainWindow::foldCurrentBlock()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Get the current line number
    QTextCursor cursor = editor->textCursor();
    int lineNumber = cursor.blockNumber();

    // Toggle fold at current line
    editor->toggleFold(lineNumber);
}

void MainWindow::unfoldCurrentBlock()
{
    // Same as fold - toggleFold handles both
    foldCurrentBlock();
}

void MainWindow::foldAll()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->foldAll();
    }
}

void MainWindow::unfoldAll()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->unfoldAll();
    }
}

void MainWindow::toggleMinimap()
{
    minimapEnabled = !minimapEnabled;
    if (minimapAction) {
        minimapAction->setChecked(minimapEnabled);
    }

    // Apply to all open editors
    for (auto it = leftTabInfoMap.begin(); it != leftTabInfoMap.end(); ++it) {
        if (it.value().minimap) {
            it.value().minimap->setVisible(minimapEnabled);
        }
    }

    for (auto it = rightTabInfoMap.begin(); it != rightTabInfoMap.end(); ++it) {
        if (it.value().minimap) {
            it.value().minimap->setVisible(minimapEnabled);
        }
    }

    saveSettings();
}

void MainWindow::toggleIndentationGuides()
{
    indentationGuidesEnabled = !indentationGuidesEnabled;
    if (indentationGuidesAction) {
        indentationGuidesAction->setChecked(indentationGuidesEnabled);
    }

    // Apply to all open editors in left tab widget
    for (int i = 0; i < leftTabWidget->count(); ++i) {
        QWidget *container = leftTabWidget->widget(i);
        if (container) {
            CodeEditor *editor = container->findChild<CodeEditor*>();
            if (editor) {
                editor->setShowIndentationGuides(indentationGuidesEnabled);
            }
        }
    }

    // Apply to all open editors in right tab widget
    for (int i = 0; i < rightTabWidget->count(); ++i) {
        QWidget *container = rightTabWidget->widget(i);
        if (container) {
            CodeEditor *editor = container->findChild<CodeEditor*>();
            if (editor) {
                editor->setShowIndentationGuides(indentationGuidesEnabled);
            }
        }
    }

    saveSettings();
}

void MainWindow::toggleActiveIndentHighlight()
{
    activeIndentHighlightEnabled = !activeIndentHighlightEnabled;
    if (activeIndentHighlightAction) {
        activeIndentHighlightAction->setChecked(activeIndentHighlightEnabled);
    }

    // Apply to all open editors in left tab widget
    for (int i = 0; i < leftTabWidget->count(); ++i) {
        QWidget *container = leftTabWidget->widget(i);
        if (container) {
            CodeEditor *editor = container->findChild<CodeEditor*>();
            if (editor) {
                editor->setHighlightActiveIndent(activeIndentHighlightEnabled);
            }
        }
    }

    // Apply to all open editors in right tab widget
    for (int i = 0; i < rightTabWidget->count(); ++i) {
        QWidget *container = rightTabWidget->widget(i);
        if (container) {
            CodeEditor *editor = container->findChild<CodeEditor*>();
            if (editor) {
                editor->setHighlightActiveIndent(activeIndentHighlightEnabled);
            }
        }
    }

    saveSettings();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("autoSaveEnabled", autoSaveEnabled);
    settings.setValue("autoSaveInterval", autoSaveInterval);
    settings.setValue("isDarkTheme", isDarkTheme);
    settings.setValue("lineWrapEnabled", lineWrapEnabled);
    settings.setValue("wordWrapMode", wordWrapMode);
    settings.setValue("showColumnRuler", showColumnRuler);
    settings.setValue("showWrapIndicator", showWrapIndicator);
    settings.setValue("wrapColumn", wrapColumn);
    settings.setValue("minimapEnabled", minimapEnabled);
    settings.setValue("indentationGuidesEnabled", indentationGuidesEnabled);
    settings.setValue("activeIndentHighlightEnabled", activeIndentHighlightEnabled);
    settings.setValue("trimWhitespaceOnSave", trimWhitespaceOnSave);
    settings.setValue("autoIndentEnabled", autoIndentEnabled);
    settings.setValue("autoCloseBracketsEnabled", autoCloseBracketsEnabled);
    settings.setValue("smartBackspaceEnabled", smartBackspaceEnabled);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    autoSaveEnabled = settings.value("autoSaveEnabled", true).toBool();
    autoSaveInterval = settings.value("autoSaveInterval", 30).toInt();
    lineWrapEnabled = settings.value("lineWrapEnabled", true).toBool();
    wordWrapMode = settings.value("wordWrapMode", true).toBool();
    showColumnRuler = settings.value("showColumnRuler", false).toBool();
    showWrapIndicator = settings.value("showWrapIndicator", true).toBool();
    wrapColumn = settings.value("wrapColumn", 80).toInt();
    minimapEnabled = settings.value("minimapEnabled", false).toBool();
    indentationGuidesEnabled = settings.value("indentationGuidesEnabled", true).toBool();
    activeIndentHighlightEnabled = settings.value("activeIndentHighlightEnabled", true).toBool();
    trimWhitespaceOnSave = settings.value("trimWhitespaceOnSave", true).toBool();
    autoIndentEnabled = settings.value("autoIndentEnabled", true).toBool();
    autoCloseBracketsEnabled = settings.value("autoCloseBracketsEnabled", true).toBool();
    smartBackspaceEnabled = settings.value("smartBackspaceEnabled", true).toBool();
    // isDarkTheme already loaded in constructor before stylesheet

    if (autoSaveAction) {
        autoSaveAction->setChecked(autoSaveEnabled);
    }

    if (themeAction) {
        themeAction->setChecked(isDarkTheme);
    }

    if (lineWrapAction) {
        lineWrapAction->setChecked(lineWrapEnabled);
    }

    if (wordWrapAction) {
        wordWrapAction->setChecked(wordWrapMode);
    }

    if (columnRulerAction) {
        columnRulerAction->setChecked(showColumnRuler);
    }

    if (minimapAction) {
        minimapAction->setChecked(minimapEnabled);
    }

    if (wrapIndicatorAction) {
        wrapIndicatorAction->setChecked(showWrapIndicator);
    }

    if (autoSaveEnabled) {
        startAutoSaveTimer();
    }
}

void MainWindow::undo()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->undo();
    }
}

void MainWindow::redo()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->redo();
    }
}

void MainWindow::cut()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->cut();
    }
}

void MainWindow::copy()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->copy();
    }
}

void MainWindow::paste()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->paste();
    }
}

void MainWindow::showFindDialog()
{
    if (!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, &FindDialog::findRequested, this, &MainWindow::performFind);
        connect(findDialog, &FindDialog::replaceRequested, this, &MainWindow::performReplace);
        connect(findDialog, &FindDialog::replaceAllRequested, this, &MainWindow::performReplaceAll);
    }

    CodeEditor *editor = getCurrentEditor();
    if (editor && editor->textCursor().hasSelection()) {
        findDialog->setFindText(editor->textCursor().selectedText());
    }

    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    showFindDialog(); // Same dialog handles both find and replace
}

void MainWindow::performFind(const QString &text, bool forward, bool caseSensitive, bool wholeWords, bool useRegex)
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;

    QTextDocument::FindFlags flags = QTextDocument::FindFlags();
    if (!forward) flags |= QTextDocument::FindBackward;
    if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;
    if (wholeWords) flags |= QTextDocument::FindWholeWords;

    QTextCursor cursor = editor->textCursor();
    QTextCursor foundCursor;

    if (useRegex) {
        QRegularExpression regex(text);
        if (caseSensitive) {
            regex.setPatternOptions(QRegularExpression::NoPatternOption);
        } else {
            regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
        foundCursor = editor->document()->find(regex, cursor, flags);
    } else {
        foundCursor = editor->document()->find(text, cursor, flags);
    }

    if (!foundCursor.isNull()) {
        editor->setTextCursor(foundCursor);
        editor->ensureCursorVisible();
    } else {
        // Search from beginning/end if not found
        QTextCursor startCursor = editor->textCursor();
        startCursor.movePosition(forward ? QTextCursor::Start : QTextCursor::End);

        if (useRegex) {
            QRegularExpression regex(text);
            if (caseSensitive) {
                regex.setPatternOptions(QRegularExpression::NoPatternOption);
            } else {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            foundCursor = editor->document()->find(regex, startCursor, flags);
        } else {
            foundCursor = editor->document()->find(text, startCursor, flags);
        }

        if (!foundCursor.isNull()) {
            editor->setTextCursor(foundCursor);
            editor->ensureCursorVisible();
        }
    }
}

void MainWindow::performReplace(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex)
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;

    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        bool matches = false;

        if (useRegex) {
            QRegularExpression regex(findText);
            if (!caseSensitive) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            matches = regex.match(selectedText).hasMatch();
        } else {
            Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
            if (wholeWords) {
                matches = (selectedText.compare(findText, cs) == 0);
            } else {
                matches = selectedText.contains(findText, cs);
            }
        }

        if (matches) {
            cursor.insertText(replaceText);
            // Find next occurrence
            performFind(findText, true, caseSensitive, wholeWords, useRegex);
        }
    } else {
        // No selection, find first occurrence
        performFind(findText, true, caseSensitive, wholeWords, useRegex);
    }
}

void MainWindow::performReplaceAll(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex)
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) return;

    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();

    // Start from beginning
    cursor.movePosition(QTextCursor::Start);
    int replacements = 0;

    QTextDocument::FindFlags flags = QTextDocument::FindFlags();
    if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;
    if (wholeWords) flags |= QTextDocument::FindWholeWords;

    while (true) {
        QTextCursor foundCursor;

        if (useRegex) {
            QRegularExpression regex(findText);
            if (!caseSensitive) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            foundCursor = editor->document()->find(regex, cursor, flags);
        } else {
            foundCursor = editor->document()->find(findText, cursor, flags);
        }

        if (foundCursor.isNull()) {
            break;
        }

        foundCursor.insertText(replaceText);
        cursor = foundCursor;
        replacements++;
    }

    cursor.endEditBlock();

    // Show status message
    if (findDialog) {
        QString message = QString("Replaced %1 occurrence(s)").arg(replacements);
        // Note: We'd need to add a status method to FindDialog to show this
    }
}

void MainWindow::showGoToLineDialog()
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    if (!goToLineDialog) {
        goToLineDialog = new GoToLineDialog(this);
        connect(goToLineDialog, &GoToLineDialog::goToLineRequested, this, &MainWindow::performGoToLine);
        connect(goToLineDialog, &GoToLineDialog::lineNumberChanged, this, &MainWindow::updateLinePreview);
    }

    // Set maximum line number
    int totalLines = editor->document()->blockCount();
    goToLineDialog->setMaximumLine(totalLines);

    // Pre-fill with current line
    int currentLine = editor->textCursor().blockNumber() + 1;
    goToLineDialog->findChild<QLineEdit*>("lineNumberEdit");

    goToLineDialog->show();
    goToLineDialog->raise();
    goToLineDialog->activateWindow();
}

void MainWindow::performGoToLine(int lineNumber)
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Move cursor to the specified line
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);

    editor->setTextCursor(cursor);
    editor->centerCursor();
    editor->setFocus();
}

void MainWindow::updateLinePreview(int lineNumber)
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor || !goToLineDialog) {
        return;
    }

    // Get the text at the specified line
    QTextBlock block = editor->document()->findBlockByLineNumber(lineNumber - 1);
    if (block.isValid()) {
        QString lineText = block.text();
        goToLineDialog->setLinePreview(lineNumber, lineText);
    }
}

void MainWindow::showSymbolSearchDialog()
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    if (!symbolSearchDialog) {
        symbolSearchDialog = new SymbolSearchDialog(this);
        connect(symbolSearchDialog, &SymbolSearchDialog::symbolSelected, this, &MainWindow::performSymbolJump);
    }

    // Extract symbols from the current document using SymbolExtractor service
    QString text = editor->toPlainText();
    QList<SymbolInfo> symbols = symbolExtractor.extractSymbols(text);

    symbolSearchDialog->setSymbols(symbols);
    symbolSearchDialog->clearFilter();
    symbolSearchDialog->show();
    symbolSearchDialog->raise();
    symbolSearchDialog->activateWindow();
}

void MainWindow::performSymbolJump(int lineNumber)
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Move cursor to the specified line
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);

    editor->setTextCursor(cursor);
    editor->centerCursor();
    editor->setFocus();
}

void MainWindow::toggleProjectPanel()
{
    projectPanelVisible = !projectPanelVisible;

    if (projectPanelVisible) {
        projectPanel->show();
        mainSplitter->setSizes({250, 600});
    } else {
        projectPanel->hide();
        mainSplitter->setSizes({0, 800});
    }
}

void MainWindow::openProjectFromPanel(const QString &filePath)
{
    // Open the file in the current active tab widget
    loadFile(filePath);
}

void MainWindow::toggleOutlinePanel()
{
    outlinePanelVisible = !outlinePanelVisible;

    if (outlinePanelVisible) {
        outlinePanel->show();
        updateOutlinePanel();
    } else {
        outlinePanel->hide();
    }
}

void MainWindow::jumpToSymbolFromOutline(int lineNumber)
{
    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Move cursor to the specified line
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);

    editor->setTextCursor(cursor);
    editor->centerCursor();
    editor->setFocus();
}

void MainWindow::updateOutlinePanel()
{
    if (!outlinePanelVisible || !outlinePanel) {
        return;
    }

    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        outlinePanel->clear();
        return;
    }

    QString documentText = editor->toPlainText();
    QString fileName;

    // Get current file name
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0) {
        fileName = tabWidget->tabText(currentIndex);
        // Remove the " *" if modified
        if (fileName.endsWith(" *")) {
            fileName.chop(2);
        }
    }

    outlinePanel->updateOutline(documentText, fileName);
}

void MainWindow::updateStatusBar()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor || !lineCountLabel || !wordCountLabel || !characterCountLabel) {
        return;
    }

    QString text = editor->toPlainText();

    // Count lines
    int lineCount = text.count('\n') + 1;
    if (text.isEmpty()) {
        lineCount = 1;
    }

    // Count words
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int wordCount = words.size();

    // Count characters
    int charCount = text.length();

    // Update labels
    lineCountLabel->setText(tr("Lines: %1").arg(lineCount));
    wordCountLabel->setText(tr("Words: %1").arg(wordCount));
    characterCountLabel->setText(tr("Characters: %1").arg(charCount));
}

// Responsive UI Methods

void MainWindow::detectScreenSize()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QSize screenSize = screen->availableSize();
        int screenWidth = screenSize.width();
        int screenHeight = screenSize.height();

        // Consider small screen if width < 1366 or height < 800
        // MNT Pocket Reform: 1920x1200 but physically 7 inches
        // Also check physical size if available
        qreal physicalWidth = screen->physicalSize().width(); // in mm

        // Small screen criteria:
        // 1. Width < 1366px OR height < 800px (typical small laptop threshold)
        // 2. OR physical width < 200mm (about 8 inches)
        isSmallScreen = (screenWidth < 1366 || screenHeight < 800 ||
                        (physicalWidth > 0 && physicalWidth < 200));

        qDebug() << "Screen size:" << screenWidth << "x" << screenHeight;
        qDebug() << "Physical width:" << physicalWidth << "mm";
        qDebug() << "Small screen mode:" << isSmallScreen;
    }
}

void MainWindow::setupResponsiveUI()
{
    if (isSmallScreen) {
        adaptUIForSmallScreen();
    } else {
        adaptUIForLargeScreen();
    }
}

void MainWindow::adaptUIForSmallScreen()
{
    // Start with project panel hidden on small screens
    projectPanelVisible = false;
    if (projectPanel) {
        projectPanel->hide();
    }

    // Use smaller window size
    resize(1000, 700);

    // Smaller toolbar with more compact layout
    if (mainToolBar) {
        mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        mainToolBar->setIconSize(QSize(16, 16));
    }

    // More compact language combo box
    if (languageComboBox) {
        languageComboBox->setMinimumWidth(80);
        languageComboBox->setMaximumWidth(120);
    }

    // More compact status bar
    if (statusBar()) {
        statusBar()->setSizeGripEnabled(false);
    }

    // Set smaller default font for editor
    if (tabWidget && tabWidget->count() > 0) {
        CodeEditor *editor = getCurrentEditor();
        if (editor) {
            QFont font = editor->font();
            font.setPointSize(9); // Smaller font for small screens
            editor->setFont(font);
            editor->setCompactMode(true);
        }
    }

    // Smaller margins and spacing
    if (centralWidget()) {
        if (QLayout *layout = centralWidget()->layout()) {
            layout->setContentsMargins(2, 2, 2, 2);
            layout->setSpacing(2);
        }
    }
}

void MainWindow::adaptUIForLargeScreen()
{
    // Default settings for larger screens
    resize(1200, 800);

    if (mainToolBar) {
        mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
        mainToolBar->setIconSize(QSize(24, 24));
    }

    // Normal language combo box size
    if (languageComboBox) {
        languageComboBox->setMinimumWidth(100);
        languageComboBox->setMaximumWidth(QWIDGETSIZE_MAX);
    }

    if (statusBar()) {
        statusBar()->setSizeGripEnabled(true);
    }

    // Normal font size
    if (tabWidget && tabWidget->count() > 0) {
        CodeEditor *editor = getCurrentEditor();
        if (editor) {
            QFont font = editor->font();
            font.setPointSize(11);
            editor->setFont(font);
            editor->setCompactMode(false);
        }
    }

    // Normal margins and spacing
    if (centralWidget()) {
        if (QLayout *layout = centralWidget()->layout()) {
            layout->setContentsMargins(4, 4, 4, 4);
            layout->setSpacing(4);
        }
    }
}

void MainWindow::updateBreadcrumb()
{
    if (!breadcrumbBar) {
        return;
    }

    CodeEditor *editor = getCurrentEditor();
    if (!editor) {
        breadcrumbBar->clear();
        return;
    }

    int currentIndex = tabWidget->currentIndex();
    QString filePath = getFilePathAt(currentIndex);

    if (filePath.isEmpty() || filePath == tr("Untitled")) {
        breadcrumbBar->clear();
        return;
    }

    breadcrumbBar->setFilePath(filePath);
    updateBreadcrumbSymbol();
}

void MainWindow::updateBreadcrumbSymbol()
{
    if (!breadcrumbBar) {
        return;
    }

    CodeEditor *editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Get current cursor position
    QTextCursor cursor = editor->textCursor();
    int currentLine = cursor.blockNumber() + 1;

    // Extract symbols from the current document
    QString documentText = editor->toPlainText();
    QList<SymbolInfo> symbols = symbolExtractor.extractSymbols(documentText);

    // Find the symbol that contains the current line
    QString currentSymbolName;
    QString currentSymbolType;

    for (const SymbolInfo &symbol : symbols) {
        if (symbol.lineNumber <= currentLine) {
            // This is a candidate - the cursor is at or after this symbol
            // In the absence of scope information, we'll use the most recent symbol
            currentSymbolName = symbol.name;
            currentSymbolType = symbol.type;
        } else {
            // We've gone past the cursor position
            break;
        }
    }

    if (!currentSymbolName.isEmpty()) {
        breadcrumbBar->setCurrentSymbol(currentSymbolName, currentSymbolType);
    } else {
        // Clear the symbol part but keep the file path
        breadcrumbBar->setCurrentSymbol("", "");
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Auto-hide project panel if window becomes too narrow
    if (isSmallScreen && event->size().width() < 800) {
        if (projectPanelVisible) {
            toggleProjectPanel();
        }
    }
}