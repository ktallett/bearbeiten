#include "mainwindow.h"
#include <QTextStream>
#include <QStringConverter>
#include <QStandardPaths>
#include <QToolBar>
#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), mainSplitter(nullptr), editorSplitter(nullptr), leftTabWidget(nullptr), rightTabWidget(nullptr),
      tabWidget(nullptr), projectPanel(nullptr), outlinePanel(nullptr), breadcrumbBar(nullptr), mainToolBar(nullptr), languageComboBox(nullptr), syntaxHighlighter(nullptr),
      lineCountLabel(nullptr), wordCountLabel(nullptr), characterCountLabel(nullptr), encodingLabel(nullptr),
      cursorPositionLabel(nullptr), selectionInfoLabel(nullptr), fileSizeLabel(nullptr),
      activeTabInfoMap(nullptr), recentFilesMenu(nullptr), currentViewMode(ViewMode::Single), focusedTabWidget(nullptr),
      projectPanelVisible(false), outlinePanelVisible(false), isSmallScreen(false), autoSaveTimer(nullptr), autoSaveEnabled(true), autoSaveInterval(30), autoSaveAction(nullptr),
      autoRestoreSessionEnabled(true),
      isDarkTheme(false), themeAction(nullptr), lineWrapEnabled(true), wordWrapMode(true), showColumnRuler(false), showWrapIndicator(true), wrapColumn(80),
      lineWrapAction(nullptr), wordWrapAction(nullptr), columnRulerAction(nullptr), wrapIndicatorAction(nullptr),
      minimapEnabled(false), minimapAction(nullptr),
      indentationGuidesEnabled(true), activeIndentHighlightEnabled(true), indentationGuidesAction(nullptr), activeIndentHighlightAction(nullptr),
      trimWhitespaceOnSave(true), autoIndentEnabled(true), autoCloseBracketsEnabled(true), smartBackspaceEnabled(true),
      findDialog(nullptr), findInFilesDialog(nullptr), goToLineDialog(nullptr), symbolSearchDialog(nullptr), characterInspector(nullptr), commandPalette(nullptr)
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
    loadRecentFiles();

    setWindowTitle(tr("Bearbeiten"));
    resize(800, 600);

    // Create first tab
    createNewTab();

    // Auto-restore session if enabled
    autoRestoreSession();
}

MainWindow::~MainWindow()
{
    // Auto-save session before closing
    autoSaveSession();

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

    // Recent Files submenu
    recentFilesMenu = new QMenu(tr("Open &Recent"), this);
    fileMenu->addMenu(recentFilesMenu);
    updateRecentFilesMenu();

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

    QAction *findInFilesAction = new QAction(tr("Find in &Files..."), this);
    findInFilesAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    connect(findInFilesAction, &QAction::triggered, this, &MainWindow::showFindInFilesDialog);
    editMenu->addAction(findInFilesAction);

    editMenu->addSeparator();

    QAction *goToLineAction = new QAction(tr("&Go to Line..."), this);
    goToLineAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(goToLineAction, &QAction::triggered, this, &MainWindow::showGoToLineDialog);
    editMenu->addAction(goToLineAction);

    QAction *goToSymbolAction = new QAction(tr("Go to &Symbol..."), this);
    goToSymbolAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(goToSymbolAction, &QAction::triggered, this, &MainWindow::showSymbolSearchDialog);
    editMenu->addAction(goToSymbolAction);

    QAction *commandPaletteAction = new QAction(tr("Command &Palette..."), this);
    commandPaletteAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    commandPaletteAction->setToolTip(tr("Show command palette"));
    connect(commandPaletteAction, &QAction::triggered, this, &MainWindow::showCommandPalette);
    editMenu->addAction(commandPaletteAction);

    editMenu->addSeparator();

    QAction *characterInspectorAction = new QAction(tr("&Inspect Character..."), this);
    characterInspectorAction->setShortcut(QKeySequence("Ctrl+Shift+I"));
    characterInspectorAction->setToolTip(tr("Show Unicode information for character at cursor"));
    connect(characterInspectorAction, &QAction::triggered, this, &MainWindow::showCharacterInspector);
    editMenu->addAction(characterInspectorAction);

    editMenu->addSeparator();

    QAction *toggleBookmarkAction = new QAction(tr("Toggle &Bookmark"), this);
    toggleBookmarkAction->setShortcut(QKeySequence(Qt::Key_F2));
    toggleBookmarkAction->setToolTip(tr("Toggle bookmark on current line"));
    connect(toggleBookmarkAction, &QAction::triggered, this, &MainWindow::toggleBookmark);
    editMenu->addAction(toggleBookmarkAction);

    QAction *nextBookmarkAction = new QAction(tr("Next Bookmar&k"), this);
    nextBookmarkAction->setShortcut(QKeySequence(Qt::Key_F3));
    nextBookmarkAction->setToolTip(tr("Go to next bookmark"));
    connect(nextBookmarkAction, &QAction::triggered, this, &MainWindow::goToNextBookmark);
    editMenu->addAction(nextBookmarkAction);

    QAction *prevBookmarkAction = new QAction(tr("Pre&vious Bookmark"), this);
    prevBookmarkAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3));
    prevBookmarkAction->setToolTip(tr("Go to previous bookmark"));
    connect(prevBookmarkAction, &QAction::triggered, this, &MainWindow::goToPreviousBookmark);
    editMenu->addAction(prevBookmarkAction);

    QAction *clearBookmarksAction = new QAction(tr("Clear All Bookmarks"), this);
    clearBookmarksAction->setToolTip(tr("Remove all bookmarks from document"));
    connect(clearBookmarksAction, &QAction::triggered, this, &MainWindow::clearAllBookmarks);
    editMenu->addAction(clearBookmarksAction);

    editMenu->addSeparator();

    QAction *duplicateLineAction = new QAction(tr("&Duplicate Line"), this);
    duplicateLineAction->setShortcut(QKeySequence("Ctrl+D"));
    duplicateLineAction->setToolTip(tr("Duplicate current line or selection"));
    connect(duplicateLineAction, &QAction::triggered, this, &MainWindow::duplicateLine);
    editMenu->addAction(duplicateLineAction);

    QAction *deleteLineAction = new QAction(tr("Delete Li&ne"), this);
    deleteLineAction->setShortcut(QKeySequence("Ctrl+Shift+K"));
    deleteLineAction->setToolTip(tr("Delete current line"));
    connect(deleteLineAction, &QAction::triggered, this, &MainWindow::deleteLine);
    editMenu->addAction(deleteLineAction);

    QAction *moveLineUpAction = new QAction(tr("Move Line &Up"), this);
    moveLineUpAction->setShortcut(QKeySequence("Alt+Up"));
    moveLineUpAction->setToolTip(tr("Move current line up"));
    connect(moveLineUpAction, &QAction::triggered, this, &MainWindow::moveLineUp);
    editMenu->addAction(moveLineUpAction);

    QAction *moveLineDownAction = new QAction(tr("Move Line &Down"), this);
    moveLineDownAction->setShortcut(QKeySequence("Alt+Down"));
    moveLineDownAction->setToolTip(tr("Move current line down"));
    connect(moveLineDownAction, &QAction::triggered, this, &MainWindow::moveLineDown);
    editMenu->addAction(moveLineDownAction);

    editMenu->addSeparator();

    QAction *sortAscAction = new QAction(tr("Sort Lines &Ascending"), this);
    sortAscAction->setToolTip(tr("Sort lines alphabetically (A-Z)"));
    connect(sortAscAction, &QAction::triggered, this, &MainWindow::sortLinesAscending);
    editMenu->addAction(sortAscAction);

    QAction *sortDescAction = new QAction(tr("Sort Lines D&escending"), this);
    sortDescAction->setToolTip(tr("Sort lines reverse alphabetically (Z-A)"));
    connect(sortDescAction, &QAction::triggered, this, &MainWindow::sortLinesDescending);
    editMenu->addAction(sortDescAction);

    editMenu->addSeparator();

    QAction *toggleLineCommentAction = new QAction(tr("Toggle Line &Comment"), this);
    toggleLineCommentAction->setShortcut(QKeySequence("Ctrl+/"));
    toggleLineCommentAction->setToolTip(tr("Toggle line comment on current line or selection"));
    connect(toggleLineCommentAction, &QAction::triggered, this, &MainWindow::toggleLineComment);
    editMenu->addAction(toggleLineCommentAction);

    QAction *toggleBlockCommentAction = new QAction(tr("Toggle &Block Comment"), this);
    toggleBlockCommentAction->setShortcut(QKeySequence("Ctrl+Shift+/"));
    toggleBlockCommentAction->setToolTip(tr("Toggle block comment on selection"));
    connect(toggleBlockCommentAction, &QAction::triggered, this, &MainWindow::toggleBlockComment);
    editMenu->addAction(toggleBlockCommentAction);

    // Session menu
    QMenu *sessionMenu = menuBar()->addMenu(tr("&Session"));

    QAction *saveSessionAction = new QAction(tr("&Save Session"), this);
    saveSessionAction->setToolTip(tr("Save current workspace session"));
    connect(saveSessionAction, &QAction::triggered, this, &MainWindow::saveSession);
    sessionMenu->addAction(saveSessionAction);

    QAction *saveSessionAsAction = new QAction(tr("Save Session &As..."), this);
    saveSessionAsAction->setToolTip(tr("Save current workspace session to a new file"));
    connect(saveSessionAsAction, &QAction::triggered, this, &MainWindow::saveSessionAs);
    sessionMenu->addAction(saveSessionAsAction);

    QAction *loadSessionAction = new QAction(tr("&Load Session..."), this);
    loadSessionAction->setToolTip(tr("Load a saved workspace session"));
    connect(loadSessionAction, &QAction::triggered, this, &MainWindow::loadSession);
    sessionMenu->addAction(loadSessionAction);

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

    QAction *changeEncodingAction = new QAction(tr("Change &Encoding..."), this);
    changeEncodingAction->setToolTip(tr("Change file encoding"));
    connect(changeEncodingAction, &QAction::triggered, this, &MainWindow::changeEncoding);
    viewMenu->addAction(changeEncodingAction);

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

    // Get current tab encoding
    int currentIndex = tabWidget->currentIndex();
    EncodingManager::Encoding encoding = EncodingManager::Encoding::UTF8;
    if (currentIndex >= 0 && activeTabInfoMap->contains(currentIndex)) {
        encoding = (*activeTabInfoMap)[currentIndex].encoding;
    }

    // Encode text using the selected encoding
    QString text = editor->toPlainText();
    QByteArray encodedData = EncodingManager::encode(text, encoding, false);

    // Check if encoding failed (incompatible characters)
    if (encodedData.isEmpty() && !text.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::warning(this,
            tr("Encoding Error"),
            tr("The document contains characters incompatible with %1.\n\n"
               "Would you like to change the encoding or cancel the save?")
            .arg(EncodingManager::encodingName(encoding)),
            QMessageBox::Save | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            // Save with lossy encoding (replace incompatible chars)
            encodedData = EncodingManager::encode(text, encoding, true);
        } else {
            return false;
        }
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Bearbeiten",
            QString("Cannot write file %1:\n%2")
            .arg(fileName)
            .arg(file.errorString()));
        return false;
    }

    // Write BOM if applicable
    QByteArray bom = EncodingManager::getBOM(encoding);
    if (!bom.isEmpty() && encoding != EncodingManager::Encoding::UTF8) {
        // Only write BOM for non-UTF8 encodings (UTF-8 BOM is optional and often avoided)
        file.write(bom);
    }

    file.write(encodedData);
    file.close();

    setCurrentFile(fileName);
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Bearbeiten",
            QString("Cannot read file %1:\n%2")
            .arg(fileName)
            .arg(file.errorString()));
        return;
    }

    // Read file as raw bytes for encoding detection
    QByteArray data = file.readAll();
    file.close();

    // Detect encoding
    EncodingManager::Encoding detectedEncoding = EncodingManager::detectEncoding(data);

    // Decode file content using detected encoding
    QString content = EncodingManager::decode(data, detectedEncoding);

    // Create new tab for this file
    createNewTab(fileName);

    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->setPlainText(content);

        setCurrentFile(fileName);

        // Add to recent files
        addToRecentFiles(fileName);

        // Store detected encoding
        int currentTabIndex = tabWidget->currentIndex();
        if (currentTabIndex >= 0 && activeTabInfoMap->contains(currentTabIndex)) {
            (*activeTabInfoMap)[currentTabIndex].encoding = detectedEncoding;
            updateEncodingLabel();

            // Auto-detect and set syntax highlighting based on file extension
            JsonSyntaxHighlighter *highlighter = (*activeTabInfoMap)[currentTabIndex].highlighter;
            if (highlighter) {
                highlighter->setLanguageFromFilename(fileName);

                // Update the combo box to show the detected language
                QString detectedLanguage = highlighter->getCurrentLanguage();
                if (!detectedLanguage.isEmpty()) {
                    // Set language in editor for comment operations
                    editor->setCurrentLanguage(detectedLanguage);

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

    // Create cursor position label
    cursorPositionLabel = new QLabel(tr("Ln 1, Col 1"));
    cursorPositionLabel->setStyleSheet("QLabel { padding: 0 8px; }");
    cursorPositionLabel->setMinimumWidth(100);
    statusBar()->addPermanentWidget(cursorPositionLabel);

    // Create selection info label
    selectionInfoLabel = new QLabel(tr(""));
    selectionInfoLabel->setStyleSheet("QLabel { padding: 0 8px; }");
    selectionInfoLabel->setMinimumWidth(80);
    statusBar()->addPermanentWidget(selectionInfoLabel);

    // Create file size label
    fileSizeLabel = new QLabel(tr("0 bytes"));
    fileSizeLabel->setStyleSheet("QLabel { padding: 0 8px; }");
    fileSizeLabel->setMinimumWidth(80);
    statusBar()->addPermanentWidget(fileSizeLabel);

    // Create encoding label with clickable behavior
    encodingLabel = new QLabel(tr("UTF-8"));
    encodingLabel->setToolTip(tr("Click to change encoding"));
    encodingLabel->setCursor(Qt::PointingHandCursor);
    encodingLabel->setStyleSheet("QLabel { padding: 0 8px; }");

    // Make it clickable using mouse tracking
    encodingLabel->installEventFilter(this);
    encodingLabel->setProperty("isEncodingLabel", true);

    statusBar()->addPermanentWidget(encodingLabel);
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

        // Also set language in editor for comment operations
        CodeEditor *editor = getCurrentEditor();
        if (editor) {
            editor->setCurrentLanguage(languageName);
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
        updateStatusBar(); // Update line/word count and other status info
    });

    // Connect cursor position changed signal to update breadcrumb and cursor position
    connect(editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateBreadcrumbSymbol);
    connect(editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateCursorPosition);

    // Connect selection changed signal to update selection info
    connect(editor, &CodeEditor::selectionChanged, this, &MainWindow::updateSelectionInfo);

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

                // Set language in editor for comment operations
                CodeEditor *editor = getCurrentEditor();
                if (editor) {
                    editor->setCurrentLanguage(currentLanguage);
                }

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

        // Restore bookmarks for the new tab
        if (activeTabInfoMap->contains(index)) {
            CodeEditor *editor = getCurrentEditor();
            if (editor) {
                editor->setBookmarks((*activeTabInfoMap)[index].bookmarks);
            }
        }
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

void MainWindow::showFindInFilesDialog()
{
    if (!findInFilesDialog) {
        findInFilesDialog = new FindInFilesDialog(this);
        connect(findInFilesDialog, &FindInFilesDialog::fileOpenRequested,
                this, &MainWindow::openFileFromFindInFiles);
    }

    // Set default directory to the current file's directory or project directory
    QString defaultDir;
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        QString currentFile = getFilePathAt(tabWidget->currentIndex());
        if (!currentFile.isEmpty()) {
            QFileInfo fileInfo(currentFile);
            defaultDir = fileInfo.absolutePath();
        }
    }

    if (defaultDir.isEmpty()) {
        defaultDir = QDir::currentPath();
    }

    findInFilesDialog->setSearchDirectory(defaultDir);

    // If text is selected, use it as the search text
    if (editor && editor->textCursor().hasSelection()) {
        findInFilesDialog->setSearchText(editor->textCursor().selectedText());
    }

    findInFilesDialog->show();
    findInFilesDialog->raise();
    findInFilesDialog->activateWindow();
}

void MainWindow::openFileFromFindInFiles(const QString &filePath, int lineNumber)
{
    // Open the file
    loadFile(filePath);

    // Jump to the specified line
    CodeEditor *editor = getCurrentEditor();
    if (editor && lineNumber > 0) {
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
        editor->setTextCursor(cursor);
        editor->centerCursor();
        editor->setFocus();
    }
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

    // Update encoding label
    updateEncodingLabel();

    // Update cursor position, selection info, and file size
    updateCursorPosition();
    updateSelectionInfo();
    updateFileSize();
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

void MainWindow::showCharacterInspector()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Create character inspector if it doesn't exist
    if (!characterInspector) {
        characterInspector = new CharacterInspector(this);
    }

    // Get the cursor position
    QTextCursor cursor = editor->textCursor();
    int position = cursor.position();

    // Get the document text
    QString text = editor->toPlainText();

    // Show the character inspector with the character at cursor
    characterInspector->inspectCharacterAtPosition(text, position);
    characterInspector->show();
    characterInspector->raise();
    characterInspector->activateWindow();
}

QList<QAction*> MainWindow::getAllActions()
{
    QList<QAction*> actions;

    // Get all actions from menu bar
    QMenuBar *menuBar = this->menuBar();
    if (menuBar) {
        // Iterate through all top-level menus
        QList<QAction*> menuActions = menuBar->actions();
        for (QAction *menuAction : menuActions) {
            QMenu *menu = menuAction->menu();
            if (menu) {
                // Recursively get all actions from this menu
                QList<QAction*> menuActionsList = menu->actions();
                for (QAction *action : menuActionsList) {
                    if (action->menu()) {
                        // If this action has a submenu, get its actions too
                        actions.append(action->menu()->actions());
                    } else {
                        actions.append(action);
                    }
                }
            }
        }
    }

    return actions;
}

void MainWindow::showCommandPalette()
{
    // Create command palette if it doesn't exist
    if (!commandPalette) {
        commandPalette = new CommandPalette(this);
    }

    // Get all actions from menus
    QList<QAction*> actions = getAllActions();

    // Set the actions in the palette
    commandPalette->setActions(actions);

    // Show the palette
    commandPalette->show();
    commandPalette->raise();
    commandPalette->activateWindow();
}

void MainWindow::updateEncodingLabel()
{
    if (!encodingLabel) {
        return;
    }

    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0 && activeTabInfoMap->contains(currentIndex)) {
        EncodingManager::Encoding encoding = (*activeTabInfoMap)[currentIndex].encoding;
        QString encodingName = EncodingManager::encodingName(encoding);
        encodingLabel->setText(encodingName);
    } else {
        encodingLabel->setText(tr("UTF-8"));
    }
}

void MainWindow::changeEncoding()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) {
        return;
    }

    // Get current encoding
    EncodingManager::Encoding currentEncoding = EncodingManager::Encoding::UTF8;
    if (activeTabInfoMap->contains(currentIndex)) {
        currentEncoding = (*activeTabInfoMap)[currentIndex].encoding;
    }

    QString currentEncodingName = EncodingManager::encodingName(currentEncoding);

    // Show dialog to select new encoding
    QStringList encodings = EncodingManager::supportedEncodings();
    bool ok;
    QString selectedEncoding = QInputDialog::getItem(this,
                                                      tr("Change Encoding"),
                                                      tr("Select file encoding:"),
                                                      encodings,
                                                      encodings.indexOf(currentEncodingName),
                                                      false,
                                                      &ok);

    if (ok && !selectedEncoding.isEmpty()) {
        EncodingManager::Encoding newEncoding = EncodingManager::encodingFromName(selectedEncoding);

        // Check if current content is compatible with new encoding
        CodeEditor *editor = getCurrentEditor();
        if (editor) {
            QString text = editor->toPlainText();
            if (!EncodingManager::isCompatible(text, newEncoding)) {
                QList<QPair<int, QChar>> incompatible = EncodingManager::findIncompatibleCharacters(text, newEncoding);

                QString warningMsg = tr("The current document contains %1 character(s) that cannot be represented in %2.\n\n"
                                       "If you save with this encoding, these characters will be replaced with '?'.\n\n"
                                       "Do you want to continue?")
                                       .arg(incompatible.size())
                                       .arg(selectedEncoding);

                QMessageBox::StandardButton reply = QMessageBox::warning(this,
                                                                         tr("Encoding Compatibility Warning"),
                                                                         warningMsg,
                                                                         QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::No) {
                    return;
                }
            }
        }

        // Update encoding for current tab
        if (activeTabInfoMap->contains(currentIndex)) {
            (*activeTabInfoMap)[currentIndex].encoding = newEncoding;
            updateEncodingLabel();

            // Mark document as modified since encoding change
            setTabModified(currentIndex, true);
        }
    }
}

void MainWindow::onEncodingLabelClicked()
{
    changeEncoding();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Handle click on encoding label
    if (obj == encodingLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onEncodingLabelClicked();
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// Bookmark implementation

void MainWindow::toggleBookmark()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Toggle bookmark in editor
    editor->toggleBookmark();

    // Save bookmarks to tab info
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0 && activeTabInfoMap->contains(currentIndex)) {
        (*activeTabInfoMap)[currentIndex].bookmarks = editor->getBookmarks();
    }
}

void MainWindow::goToNextBookmark()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->goToNextBookmark();
    }
}

void MainWindow::goToPreviousBookmark()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->goToPreviousBookmark();
    }
}

void MainWindow::clearAllBookmarks()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    // Clear bookmarks in editor
    editor->clearAllBookmarks();

    // Clear bookmarks in tab info
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0 && activeTabInfoMap->contains(currentIndex)) {
        (*activeTabInfoMap)[currentIndex].bookmarks.clear();
    }
}

// Line operation implementation

void MainWindow::duplicateLine()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->duplicateLine();
    }
}

void MainWindow::deleteLine()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->deleteLine();
    }
}

void MainWindow::moveLineUp()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->moveLineUp();
    }
}

void MainWindow::moveLineDown()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->moveLineDown();
    }
}

void MainWindow::sortLinesAscending()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->sortLinesAscending();
    }
}

void MainWindow::sortLinesDescending()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->sortLinesDescending();
    }
}

// Comment operation implementation

void MainWindow::toggleLineComment()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->toggleLineComment();
    }
}

void MainWindow::toggleBlockComment()
{
    CodeEditor *editor = getCurrentEditor();
    if (editor) {
        editor->toggleBlockComment();
    }
}

// Recent files implementation

void MainWindow::addToRecentFiles(const QString &filePath)
{
    // Remove if already exists (to move it to top)
    recentFiles.removeAll(filePath);

    // Add to the front
    recentFiles.prepend(filePath);

    // Limit to MaxRecentFiles
    while (recentFiles.size() > MaxRecentFiles) {
        recentFiles.removeLast();
    }

    // Update menu and save
    updateRecentFilesMenu();
    saveRecentFiles();
}

void MainWindow::updateRecentFilesMenu()
{
    if (!recentFilesMenu) {
        return;
    }

    recentFilesMenu->clear();

    // Add each recent file as an action
    for (int i = 0; i < recentFiles.size(); ++i) {
        const QString &filePath = recentFiles.at(i);

        // Check if file still exists
        if (!QFile::exists(filePath)) {
            continue;
        }

        // Create action with just the filename for display
        QString displayName = QFileInfo(filePath).fileName();
        QAction *action = new QAction(QString("%1. %2").arg(i + 1).arg(displayName), this);
        action->setData(filePath);
        action->setToolTip(filePath); // Show full path in tooltip
        action->setStatusTip(filePath);

        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        recentFilesMenu->addAction(action);
    }

    // Add separator and Clear Recent Files if there are items
    if (!recentFiles.isEmpty()) {
        recentFilesMenu->addSeparator();
        QAction *clearAction = new QAction(tr("Clear Recent Files"), this);
        connect(clearAction, &QAction::triggered, this, &MainWindow::clearRecentFiles);
        recentFilesMenu->addAction(clearAction);
    }

    // Disable menu if empty
    recentFilesMenu->setEnabled(!recentFiles.isEmpty());
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        QString filePath = action->data().toString();
        if (QFile::exists(filePath)) {
            loadFile(filePath);
        } else {
            QMessageBox::warning(this, tr("File Not Found"),
                               tr("The file '%1' no longer exists.").arg(filePath));
            // Remove from recent files
            recentFiles.removeAll(filePath);
            updateRecentFilesMenu();
            saveRecentFiles();
        }
    }
}

void MainWindow::clearRecentFiles()
{
    recentFiles.clear();
    updateRecentFilesMenu();
    saveRecentFiles();
}

void MainWindow::loadRecentFiles()
{
    QSettings settings;
    recentFiles = settings.value("recentFiles").toStringList();

    // Remove any files that no longer exist
    QStringList validFiles;
    for (const QString &file : recentFiles) {
        if (QFile::exists(file)) {
            validFiles.append(file);
        }
    }
    recentFiles = validFiles;

    updateRecentFilesMenu();
}

void MainWindow::saveRecentFiles()
{
    QSettings settings;
    settings.setValue("recentFiles", recentFiles);
}

// Status bar enhancements implementation

void MainWindow::updateCursorPosition()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor || !cursorPositionLabel) {
        return;
    }

    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.positionInBlock() + 1;

    cursorPositionLabel->setText(tr("Ln %1, Col %2").arg(line).arg(column));
}

void MainWindow::updateSelectionInfo()
{
    CodeEditor *editor = getCurrentEditor();
    if (!editor || !selectionInfoLabel) {
        return;
    }

    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        int charCount = selectedText.length();
        int lineCount = selectedText.count(QChar::ParagraphSeparator) + 1;

        if (lineCount > 1) {
            selectionInfoLabel->setText(tr("%1 chars, %2 lines").arg(charCount).arg(lineCount));
        } else {
            selectionInfoLabel->setText(tr("%1 chars").arg(charCount));
        }
    } else {
        selectionInfoLabel->setText(tr(""));
    }
}

void MainWindow::updateFileSize()
{
    if (!fileSizeLabel) {
        return;
    }

    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) {
        fileSizeLabel->setText(tr("0 bytes"));
        return;
    }

    QString filePath = getFilePathAt(currentIndex);

    if (filePath.isEmpty()) {
        // New unsaved file
        CodeEditor *editor = getCurrentEditor();
        if (editor) {
            qint64 size = editor->toPlainText().toUtf8().size();
            fileSizeLabel->setText(formatFileSize(size));
        } else {
            fileSizeLabel->setText(tr("0 bytes"));
        }
    } else {
        // Existing file
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            qint64 size = fileInfo.size();
            fileSizeLabel->setText(formatFileSize(size));
        } else {
            fileSizeLabel->setText(tr("0 bytes"));
        }
    }
}

QString MainWindow::formatFileSize(qint64 bytes)
{
    if (bytes < 1024) {
        return tr("%1 bytes").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return tr("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    } else if (bytes < 1024 * 1024 * 1024) {
        return tr("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        return tr("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
    }
}

// Session management implementation

QJsonObject MainWindow::createSessionData()
{
    QJsonObject sessionData;

    // Save view mode
    sessionData["viewMode"] = static_cast<int>(currentViewMode);

    // Save panel visibility
    sessionData["projectPanelVisible"] = projectPanelVisible;
    sessionData["outlinePanelVisible"] = outlinePanelVisible;

    // Save current tab index
    sessionData["currentTab"] = tabWidget->currentIndex();

    // Save open files with their state
    QJsonArray filesArray;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QString filePath = getFilePathAt(i);
        if (filePath.isEmpty()) {
            continue; // Skip unsaved files
        }

        QJsonObject fileData;
        fileData["path"] = filePath;
        fileData["modified"] = isTabModified(i);

        // Get editor for this tab
        QWidget *tabContainer = tabWidget->widget(i);
        if (tabContainer) {
            CodeEditor *editor = tabContainer->findChild<CodeEditor*>();
            if (editor) {
                // Save cursor position
                QTextCursor cursor = editor->textCursor();
                fileData["cursorLine"] = cursor.blockNumber();
                fileData["cursorColumn"] = cursor.positionInBlock();

                // Save bookmarks
                QSet<int> bookmarks = editor->getBookmarks();
                if (!bookmarks.isEmpty()) {
                    QJsonArray bookmarksArray;
                    for (int bookmark : bookmarks) {
                        bookmarksArray.append(bookmark);
                    }
                    fileData["bookmarks"] = bookmarksArray;
                }

                // Save current language
                fileData["language"] = editor->getCurrentLanguage();
            }
        }

        // Save encoding
        if (activeTabInfoMap->contains(i)) {
            EncodingManager::Encoding encoding = (*activeTabInfoMap)[i].encoding;
            fileData["encoding"] = static_cast<int>(encoding);
        }

        filesArray.append(fileData);
    }
    sessionData["files"] = filesArray;

    return sessionData;
}

void MainWindow::restoreSessionData(const QJsonObject &sessionData)
{
    // Close all existing tabs except the first empty one
    while (tabWidget->count() > 1) {
        closeTab(tabWidget->count() - 1);
    }
    if (tabWidget->count() == 1) {
        QString filePath = getFilePathAt(0);
        if (filePath.isEmpty() && !isTabModified(0)) {
            closeTab(0);
        }
    }

    // Restore view mode
    if (sessionData.contains("viewMode")) {
        currentViewMode = static_cast<ViewMode>(sessionData["viewMode"].toInt());
        // Note: Split view restoration would be handled here
    }

    // Restore panel visibility
    if (sessionData.contains("projectPanelVisible")) {
        bool shouldBeVisible = sessionData["projectPanelVisible"].toBool();
        if (shouldBeVisible != projectPanelVisible) {
            toggleProjectPanel();
        }
    }

    if (sessionData.contains("outlinePanelVisible")) {
        bool shouldBeVisible = sessionData["outlinePanelVisible"].toBool();
        if (shouldBeVisible != outlinePanelVisible) {
            toggleOutlinePanel();
        }
    }

    // Restore files
    if (sessionData.contains("files")) {
        QJsonArray filesArray = sessionData["files"].toArray();
        int targetTabIndex = -1;

        for (const QJsonValue &value : filesArray) {
            QJsonObject fileData = value.toObject();
            QString filePath = fileData["path"].toString();

            if (filePath.isEmpty() || !QFile::exists(filePath)) {
                continue;
            }

            // Load the file
            loadFile(filePath);

            int currentTab = tabWidget->currentIndex();
            if (currentTab >= 0) {
                // Restore cursor position
                if (fileData.contains("cursorLine")) {
                    CodeEditor *editor = getCurrentEditor();
                    if (editor) {
                        int line = fileData["cursorLine"].toInt();
                        int column = fileData["cursorColumn"].toInt();

                        QTextCursor cursor = editor->textCursor();
                        cursor.movePosition(QTextCursor::Start);
                        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
                        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
                        editor->setTextCursor(cursor);
                        editor->centerCursor();
                    }
                }

                // Restore bookmarks
                if (fileData.contains("bookmarks")) {
                    CodeEditor *editor = getCurrentEditor();
                    if (editor) {
                        QJsonArray bookmarksArray = fileData["bookmarks"].toArray();
                        QSet<int> bookmarks;
                        for (const QJsonValue &bm : bookmarksArray) {
                            bookmarks.insert(bm.toInt());
                        }
                        editor->setBookmarks(bookmarks);
                        if (activeTabInfoMap->contains(currentTab)) {
                            (*activeTabInfoMap)[currentTab].bookmarks = bookmarks;
                        }
                    }
                }

                // Restore encoding
                if (fileData.contains("encoding") && activeTabInfoMap->contains(currentTab)) {
                    int encodingValue = fileData["encoding"].toInt();
                    (*activeTabInfoMap)[currentTab].encoding = static_cast<EncodingManager::Encoding>(encodingValue);
                    updateEncodingLabel();
                }

                // Restore language
                if (fileData.contains("language")) {
                    CodeEditor *editor = getCurrentEditor();
                    if (editor) {
                        QString language = fileData["language"].toString();
                        editor->setCurrentLanguage(language);
                    }
                }

                // Mark as unmodified if it was saved
                if (!fileData["modified"].toBool()) {
                    setTabModified(currentTab, false);
                }
            }
        }

        // Restore active tab
        if (sessionData.contains("currentTab")) {
            int targetTab = sessionData["currentTab"].toInt();
            if (targetTab >= 0 && targetTab < tabWidget->count()) {
                tabWidget->setCurrentIndex(targetTab);
            }
        }
    }
}

void MainWindow::saveSession()
{
    if (currentSessionPath.isEmpty()) {
        saveSessionAs();
    } else {
        saveSessionToFile(currentSessionPath);
    }
}

void MainWindow::saveSessionAs()
{
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Session"),
        defaultPath + "/session.json",
        tr("Session Files (*.json);;All Files (*)"));

    if (!fileName.isEmpty()) {
        currentSessionPath = fileName;
        saveSessionToFile(fileName);
    }
}

void MainWindow::loadSession()
{
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load Session"),
        defaultPath,
        tr("Session Files (*.json);;All Files (*)"));

    if (!fileName.isEmpty()) {
        currentSessionPath = fileName;
        loadSessionFromFile(fileName);
    }
}

void MainWindow::saveSessionToFile(const QString &sessionPath)
{
    QJsonObject sessionData = createSessionData();
    QJsonDocument doc(sessionData);

    QFile file(sessionPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        statusBar()->showMessage(tr("Session saved"), 3000);
    } else {
        QMessageBox::warning(this, tr("Save Session"),
            tr("Could not save session to %1").arg(sessionPath));
    }
}

void MainWindow::loadSessionFromFile(const QString &sessionPath)
{
    QFile file(sessionPath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Session"),
            tr("Could not open session file %1").arg(sessionPath));
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, tr("Load Session"),
            tr("Invalid session file format"));
        return;
    }

    restoreSessionData(doc.object());
    statusBar()->showMessage(tr("Session loaded"), 3000);
}

void MainWindow::autoSaveSession()
{
    if (!autoRestoreSessionEnabled) {
        return;
    }

    QString sessionPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(sessionPath);
    sessionPath += "/autosave-session.json";

    saveSessionToFile(sessionPath);
}

void MainWindow::autoRestoreSession()
{
    if (!autoRestoreSessionEnabled) {
        return;
    }

    QString sessionPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    sessionPath += "/autosave-session.json";

    if (QFile::exists(sessionPath)) {
        loadSessionFromFile(sessionPath);
    }
}