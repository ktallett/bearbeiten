#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QSet>
#include <QList>
#include <QTextCursor>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setCompactMode(bool compact);

    // Line wrapping
    void setShowWrapIndicator(bool show);
    void setWrapColumn(int column);
    void setShowColumnRuler(bool show);
    int getWrapColumn() const { return wrapColumn; }
    bool isShowingColumnRuler() const { return showColumnRuler; }
    bool isShowingWrapIndicator() const { return showWrapIndicator; }

    // Code folding
    void toggleFold(int lineNumber);
    bool isFoldable(int lineNumber);
    void foldAll();
    void unfoldAll();

    // Public accessors for protected methods (for LineNumberArea)
    QTextBlock getFirstVisibleBlock() const;
    QRectF getBlockBoundingGeometry(const QTextBlock &block) const;
    QRectF getBlockBoundingRect(const QTextBlock &block) const;
    QPointF getContentOffset() const;

    // Smart editing features
    void setAutoIndent(bool enable);
    void setAutoCloseBrackets(bool enable);
    void setSmartBackspace(bool enable);
    void trimTrailingWhitespace();
    bool isAutoIndentEnabled() const { return autoIndent; }
    bool isAutoCloseBracketsEnabled() const { return autoCloseBrackets; }
    bool isSmartBackspaceEnabled() const { return smartBackspace; }

    // Multiple cursors features
    void addCursorAtPosition(const QTextCursor &cursor);
    void clearExtraCursors();
    void selectNextOccurrence();
    void addCursorAbove();
    void addCursorBelow();
    bool hasMultipleCursors() const { return !extraCursors.isEmpty(); }
    int cursorCount() const { return extraCursors.isEmpty() ? 1 : extraCursors.size() + 1; }

    // Indentation guides
    void setShowIndentationGuides(bool show);
    void setHighlightActiveIndent(bool highlight);
    bool isShowingIndentationGuides() const { return showIndentationGuides; }
    bool isHighlightingActiveIndent() const { return highlightActiveIndent; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void matchBrackets();

private:
    QWidget *lineNumberArea;
    bool compactMode;

    // Line wrapping settings
    bool showWrapIndicator;
    bool showColumnRuler;
    int wrapColumn;

    // Smart editing settings
    bool autoIndent;
    bool autoCloseBrackets;
    bool smartBackspace;

    // Multiple cursors
    QList<QTextCursor> extraCursors;
    QString lastSearchText;

    // Indentation guides
    bool showIndentationGuides;
    bool highlightActiveIndent;

    struct BracketInfo {
        QChar character;
        int position;
    };

    BracketInfo findMatchingBracket(QChar bracket, int position, bool forward);
    bool isOpeningBracket(QChar c);
    bool isClosingBracket(QChar c);
    QChar getMatchingBracket(QChar c);

    // Code folding helpers
    int getIndentLevel(const QString &text);
    int findFoldEndLine(int startLine);
    void setBlockVisible(int lineNumber, bool visible);
    bool isBlockFolded(int lineNumber);

    // Smart editing helpers
    QString getIndentationOfLine(const QString &text);
    bool isAutoClosingChar(QChar c);
    QChar getClosingChar(QChar c);
    void handleAutoIndent();
    void handleAutoCloseBracket(QChar openChar);
    void handleSmartBackspace();

    // Multiple cursor helpers
    void insertTextAtAllCursors(const QString &text);
    void removeTextAtAllCursors(int length);
    void drawCursor(QPainter &painter, const QTextCursor &cursor);
    void sortCursors();
    void mergeCursors();

    // Indentation guide helpers
    int getBlockIndentLevel(const QTextBlock &block);
    int getActiveIndentLevel();
    void drawIndentationGuides(QPainter &painter);

    QSet<int> foldedBlocks;  // Track which lines are folded
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override;

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H