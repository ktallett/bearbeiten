#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "codeeditor.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QTabWidget>
#include <QTabBar>
#include <QMenu>
#include <QMap>
#include <QSplitter>
#include <QTimer>
#include <QSettings>
#include <QTextCursor>
#include <QRegularExpression>
#include <QScreen>
#include <QGuiApplication>
#include <QResizeEvent>
#include "jsonsyntaxhighlighter.h"
#include "finddialog.h"
#include "minimap.h"
#include "gotolinedialog.h"
#include "symbolsearchdialog.h"
#include "symbolextractor.h"
#include "outlinepanel.h"
#include "projectpanel.h"
#include "breadcrumbbar.h"

enum class ViewMode {
    Single,
    SideBySide
};

struct TabInfo {
    QString filePath;
    JsonSyntaxHighlighter *highlighter;
    Minimap *minimap;

    TabInfo() : highlighter(nullptr), minimap(nullptr) {}
    TabInfo(const QString &path, JsonSyntaxHighlighter *hl, Minimap *mm = nullptr)
        : filePath(path), highlighter(hl), minimap(mm) {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void exitApp();

    // Edit slots
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void showFindDialog();
    void showReplaceDialog();
    void showGoToLineDialog();
    void showSymbolSearchDialog();

    void onLanguageChanged(int index);
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onTabContextMenu(const QPoint &pos);

    // Split view slots
    void toggleSplitView();
    void splitHorizontally();
    void splitVertically();
    void closeSplitView();
    void onSplitterFocusChanged();

    // Project panel slots
    void toggleProjectPanel();
    void openProjectFromPanel(const QString &filePath);

    // Outline panel slots
    void toggleOutlinePanel();
    void jumpToSymbolFromOutline(int lineNumber);
    void updateOutlinePanel();

    // Auto-save slots
    void autoSave();
    void onTextChanged();
    void toggleAutoSave();

    // Theme slots
    void toggleTheme();

    // Line wrap slots
    void toggleLineWrap();
    void setLineWrapMode(int mode);
    void toggleWordWrapMode();
    void toggleColumnRuler();
    void toggleWrapIndicator();
    void setWrapColumn();

    // Code folding slots
    void foldCurrentBlock();
    void unfoldCurrentBlock();
    void foldAll();
    void unfoldAll();

    // Minimap slots
    void toggleMinimap();

    // Indentation guide slots
    void toggleIndentationGuides();
    void toggleActiveIndentHighlight();

    // Find/Replace slots
    void performFind(const QString &text, bool forward, bool caseSensitive, bool wholeWords, bool useRegex);
    void performReplace(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex);
    void performReplaceAll(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex);

    // Go to Line slots
    void performGoToLine(int lineNumber);
    void updateLinePreview(int lineNumber);

    // Symbol search slots
    void performSymbolJump(int lineNumber);

    // Breadcrumb slots
    void updateBreadcrumb();
    void updateBreadcrumbSymbol();

private:
    void setupMenus();
    void setupEditor();
    void setupStatusBar();
    void setupToolBar();
    void loadStyleSheet();
    bool maybeSave();
    bool maybeSaveTab(int tabIndex);
    bool saveDocument(const QString &fileName);
    void loadFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);

    // Tab management
    void createNewTab(const QString &fileName = QString());
    void closeTab(int index);
    void closeOtherTabs(int index);
    void closeAllTabs();
    CodeEditor* getCurrentEditor();
    CodeEditor* getEditorAt(int index);
    QString getFilePathAt(int index);
    void setFilePathAt(int index, const QString &filePath);
    bool isTabModified(int index);
    void setTabModified(int index, bool modified);
    void updateTabTitle(int index);

    // Split view methods
    void setupSplitView();
    void updateViewMode();
    QTabWidget* getActiveTabWidget();
    QTabWidget* getInactiveTabWidget();
    void syncLanguageComboBox();

    // Auto-save methods
    void setupAutoSave();
    void startAutoSaveTimer();
    void stopAutoSaveTimer();
    void saveSettings();
    void loadSettings();

    // Status bar methods
    void updateStatusBar();

    // Responsive UI methods
    void detectScreenSize();
    void setupResponsiveUI();
    void adaptUIForSmallScreen();
    void adaptUIForLargeScreen();

    // Main widgets
    QSplitter *mainSplitter;
    QSplitter *editorSplitter;
    QTabWidget *leftTabWidget;
    QTabWidget *rightTabWidget;
    QTabWidget *tabWidget; // Points to active tab widget
    ProjectPanel *projectPanel;
    OutlinePanel *outlinePanel;
    BreadcrumbBar *breadcrumbBar;

    // UI components
    QToolBar *mainToolBar;
    QComboBox *languageComboBox;
    JsonSyntaxHighlighter *syntaxHighlighter;
    QLabel *lineCountLabel;
    QLabel *wordCountLabel;
    QLabel *characterCountLabel;

    // Data management
    QMap<int, TabInfo> leftTabInfoMap;
    QMap<int, TabInfo> rightTabInfoMap;
    QMap<int, TabInfo> *activeTabInfoMap; // Points to active map

    // State
    ViewMode currentViewMode;
    QTabWidget *focusedTabWidget;
    bool projectPanelVisible;
    bool outlinePanelVisible;
    bool isSmallScreen;

    // Auto-save components
    QTimer *autoSaveTimer;
    bool autoSaveEnabled;
    int autoSaveInterval; // in seconds
    QAction *autoSaveAction;

    // Theme components
    bool isDarkTheme;
    QAction *themeAction;

    // Line wrap components
    bool lineWrapEnabled;
    bool wordWrapMode;
    bool showColumnRuler;
    bool showWrapIndicator;
    int wrapColumn;
    QAction *lineWrapAction;
    QAction *wordWrapAction;
    QAction *columnRulerAction;
    QAction *wrapIndicatorAction;

    // Minimap components
    bool minimapEnabled;
    QAction *minimapAction;

    // Indentation guide components
    bool indentationGuidesEnabled;
    bool activeIndentHighlightEnabled;
    QAction *indentationGuidesAction;
    QAction *activeIndentHighlightAction;

    // Smart editing components
    bool trimWhitespaceOnSave;
    bool autoIndentEnabled;
    bool autoCloseBracketsEnabled;
    bool smartBackspaceEnabled;

    // Find/Replace components
    FindDialog *findDialog;

    // Go to Line components
    GoToLineDialog *goToLineDialog;

    // Symbol search components
    SymbolSearchDialog *symbolSearchDialog;
    SymbolExtractor symbolExtractor;
};

#endif // MAINWINDOW_H