#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QSet>

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

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

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