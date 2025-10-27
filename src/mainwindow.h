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
#include "projectpanel.h"

enum class ViewMode {
    Single,
    SideBySide
};

struct TabInfo {
    QString filePath;
    JsonSyntaxHighlighter *highlighter;

    TabInfo() : highlighter(nullptr) {}
    TabInfo(const QString &path, JsonSyntaxHighlighter *hl) : filePath(path), highlighter(hl) {}
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

    // Auto-save slots
    void autoSave();
    void onTextChanged();
    void toggleAutoSave();

    // Theme slots
    void toggleTheme();

    // Line wrap slots
    void toggleLineWrap();
    void setLineWrapMode(int mode);

    // Code folding slots
    void foldCurrentBlock();
    void unfoldCurrentBlock();
    void foldAll();
    void unfoldAll();

    // Find/Replace slots
    void performFind(const QString &text, bool forward, bool caseSensitive, bool wholeWords, bool useRegex);
    void performReplace(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex);
    void performReplaceAll(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool useRegex);

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
    QAction *lineWrapAction;

    // Find/Replace components
    FindDialog *findDialog;
};

#endif // MAINWINDOW_H