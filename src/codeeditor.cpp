#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QMouseEvent>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), compactMode(false)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::matchBrackets);

    updateLineNumberAreaWidth(0);
    matchBrackets();

    setTabStopDistance(40);

    // Set font using design spec monospace stack
    QFont font;
    QStringList fontFamilies = {"JetBrains Mono", "SF Mono", "Consolas", "Monaco", "Liberation Mono", "Courier New"};

    for (const QString &family : fontFamilies) {
        font.setFamily(family);
        if (font.exactMatch()) {
            break;
        }
    }

    font.setPointSize(14); // Design spec font size for code
    font.setStyleHint(QFont::Monospace);
    setFont(font);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // Add space for fold indicator (12 pixels) + line numbers + padding
    int space = 12 + 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space + (compactMode ? 4 : 10); // Less padding in compact mode
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    // This method is kept for compatibility but functionality
    // has been merged into matchBrackets() for efficiency
    matchBrackets();
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    // Use design spec color for line number area background
    painter.fillRect(event->rect(), QColor(250, 250, 250)); // --color-bg-secondary: #FAFAFA

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            // Use design spec color for line numbers: --color-fg-secondary: #696D79
            painter.setPen(QColor(105, 109, 121));

            // Draw fold indicator if foldable
            if (isFoldable(blockNumber)) {
                int indicatorSize = 8;
                int indicatorX = 2;
                int indicatorY = top + (fontMetrics().height() - indicatorSize) / 2;

                painter.save();
                painter.setRenderHint(QPainter::Antialiasing);

                if (isBlockFolded(blockNumber)) {
                    // Draw collapsed indicator (right-pointing triangle)
                    painter.setBrush(QColor(105, 109, 121));
                    painter.setPen(Qt::NoPen);
                    QPolygon triangle;
                    triangle << QPoint(indicatorX, indicatorY)
                            << QPoint(indicatorX, indicatorY + indicatorSize)
                            << QPoint(indicatorX + indicatorSize, indicatorY + indicatorSize / 2);
                    painter.drawPolygon(triangle);
                } else {
                    // Draw expanded indicator (down-pointing triangle)
                    painter.setBrush(QColor(105, 109, 121));
                    painter.setPen(Qt::NoPen);
                    QPolygon triangle;
                    triangle << QPoint(indicatorX, indicatorY)
                            << QPoint(indicatorX + indicatorSize, indicatorY)
                            << QPoint(indicatorX + indicatorSize / 2, indicatorY + indicatorSize);
                    painter.drawPolygon(triangle);
                }

                painter.restore();
            }

            painter.drawText(12, top, lineNumberArea->width() - 17, fontMetrics().height(),
                           Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::setCompactMode(bool compact)
{
    compactMode = compact;

    // Adjust tab stop distance for compact mode
    setTabStopDistance(compact ? 30 : 40);

    // Update line number area width
    updateLineNumberAreaWidth(0);
}

bool CodeEditor::isOpeningBracket(QChar c)
{
    return c == '(' || c == '[' || c == '{' || c == '<';
}

bool CodeEditor::isClosingBracket(QChar c)
{
    return c == ')' || c == ']' || c == '}' || c == '>';
}

QChar CodeEditor::getMatchingBracket(QChar c)
{
    switch (c.toLatin1()) {
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        case '{': return '}';
        case '}': return '{';
        case '<': return '>';
        case '>': return '<';
        default: return QChar();
    }
}

CodeEditor::BracketInfo CodeEditor::findMatchingBracket(QChar bracket, int position, bool forward)
{
    QString text = toPlainText();
    QChar match = getMatchingBracket(bracket);
    int depth = 1;
    int step = forward ? 1 : -1;
    int i = position + step;

    while (i >= 0 && i < text.length()) {
        QChar current = text[i];

        if (current == bracket) {
            depth++;
        } else if (current == match) {
            depth--;
            if (depth == 0) {
                return {match, i};
            }
        }

        i += step;
    }

    return {QChar(), -1}; // No match found
}

void CodeEditor::matchBrackets()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    // First, add current line highlighting
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(68, 130, 180, 20); // rgba(68, 130, 180, 0.08)
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    // Now add bracket matching
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QString text = toPlainText();

    // Check character before cursor
    if (pos > 0) {
        QChar charBefore = text[pos - 1];
        if (isOpeningBracket(charBefore) || isClosingBracket(charBefore)) {
            bool forward = isOpeningBracket(charBefore);
            BracketInfo matchInfo = findMatchingBracket(charBefore, pos - 1, forward);

            if (matchInfo.position != -1) {
                // Matched bracket - highlight both
                QColor matchColor = QColor(68, 130, 180, 60); // Blue with more opacity

                // Highlight bracket before cursor
                QTextEdit::ExtraSelection selection1;
                QTextCursor cursor1 = textCursor();
                cursor1.setPosition(pos - 1);
                cursor1.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                selection1.cursor = cursor1;
                selection1.format.setBackground(matchColor);
                extraSelections.append(selection1);

                // Highlight matching bracket
                QTextEdit::ExtraSelection selection2;
                QTextCursor cursor2 = textCursor();
                cursor2.setPosition(matchInfo.position);
                cursor2.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                selection2.cursor = cursor2;
                selection2.format.setBackground(matchColor);
                extraSelections.append(selection2);
            } else {
                // Unmatched bracket - highlight in red
                QColor unmatchedColor = QColor(239, 83, 80, 80); // Red

                QTextEdit::ExtraSelection selection;
                QTextCursor cursor1 = textCursor();
                cursor1.setPosition(pos - 1);
                cursor1.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                selection.cursor = cursor1;
                selection.format.setBackground(unmatchedColor);
                extraSelections.append(selection);
            }
        }
    }

    // Check character after cursor
    if (pos < text.length()) {
        QChar charAfter = text[pos];
        if (isOpeningBracket(charAfter) || isClosingBracket(charAfter)) {
            bool forward = isOpeningBracket(charAfter);
            BracketInfo matchInfo = findMatchingBracket(charAfter, pos, forward);

            if (matchInfo.position != -1) {
                // Only highlight if we didn't already highlight this pair
                bool alreadyHighlighted = false;
                if (pos > 0) {
                    QChar charBefore = text[pos - 1];
                    if (isOpeningBracket(charBefore) || isClosingBracket(charBefore)) {
                        alreadyHighlighted = true;
                    }
                }

                if (!alreadyHighlighted) {
                    QColor matchColor = QColor(68, 130, 180, 60);

                    // Highlight bracket after cursor
                    QTextEdit::ExtraSelection selection1;
                    QTextCursor cursor1 = textCursor();
                    cursor1.setPosition(pos);
                    cursor1.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    selection1.cursor = cursor1;
                    selection1.format.setBackground(matchColor);
                    extraSelections.append(selection1);

                    // Highlight matching bracket
                    QTextEdit::ExtraSelection selection2;
                    QTextCursor cursor2 = textCursor();
                    cursor2.setPosition(matchInfo.position);
                    cursor2.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    selection2.cursor = cursor2;
                    selection2.format.setBackground(matchColor);
                    extraSelections.append(selection2);
                }
            }
        }
    }

    setExtraSelections(extraSelections);
}

int CodeEditor::getIndentLevel(const QString &text)
{
    int indent = 0;
    for (QChar c : text) {
        if (c == ' ') {
            indent++;
        } else if (c == '\t') {
            indent += 4; // Tab counts as 4 spaces
        } else {
            break;
        }
    }
    return indent;
}

int CodeEditor::findFoldEndLine(int startLine)
{
    QTextBlock startBlock = document()->findBlockByNumber(startLine);
    if (!startBlock.isValid()) {
        return -1;
    }

    QString startText = startBlock.text();
    int startIndent = getIndentLevel(startText);

    // Find the end of the fold region
    int lineCount = document()->blockCount();
    for (int i = startLine + 1; i < lineCount; ++i) {
        QTextBlock block = document()->findBlockByNumber(i);
        if (!block.isValid()) {
            break;
        }

        QString text = block.text().trimmed();
        // Skip empty lines
        if (text.isEmpty()) {
            continue;
        }

        int indent = getIndentLevel(block.text());
        // If we find a line with same or less indentation, that's the end
        if (indent <= startIndent) {
            return i - 1;
        }
    }

    // Fold to the end of document
    return lineCount - 1;
}

bool CodeEditor::isFoldable(int lineNumber)
{
    QTextBlock block = document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) {
        return false;
    }

    QString text = block.text();
    // Empty lines are not foldable
    if (text.trimmed().isEmpty()) {
        return false;
    }

    // Check if next non-empty line has greater indentation
    int currentIndent = getIndentLevel(text);
    int lineCount = document()->blockCount();

    for (int i = lineNumber + 1; i < lineCount; ++i) {
        QTextBlock nextBlock = document()->findBlockByNumber(i);
        if (!nextBlock.isValid()) {
            break;
        }

        QString nextText = nextBlock.text();
        if (nextText.trimmed().isEmpty()) {
            continue;
        }

        int nextIndent = getIndentLevel(nextText);
        return nextIndent > currentIndent;
    }

    return false;
}

bool CodeEditor::isBlockFolded(int lineNumber)
{
    return foldedBlocks.contains(lineNumber);
}

void CodeEditor::setBlockVisible(int lineNumber, bool visible)
{
    QTextBlock block = document()->findBlockByNumber(lineNumber);
    if (block.isValid()) {
        block.setVisible(visible);
    }
}

void CodeEditor::toggleFold(int lineNumber)
{
    if (!isFoldable(lineNumber)) {
        return;
    }

    if (foldedBlocks.contains(lineNumber)) {
        // Unfold
        foldedBlocks.remove(lineNumber);

        int endLine = findFoldEndLine(lineNumber);
        for (int i = lineNumber + 1; i <= endLine; ++i) {
            setBlockVisible(i, true);
        }
    } else {
        // Fold
        foldedBlocks.insert(lineNumber);

        int endLine = findFoldEndLine(lineNumber);
        for (int i = lineNumber + 1; i <= endLine; ++i) {
            setBlockVisible(i, false);
        }
    }

    // Update the editor
    viewport()->update();
    lineNumberArea->update();
    document()->markContentsDirty(0, document()->characterCount());
}

void CodeEditor::foldAll()
{
    int lineCount = document()->blockCount();
    for (int i = 0; i < lineCount; ++i) {
        if (isFoldable(i) && !foldedBlocks.contains(i)) {
            toggleFold(i);
        }
    }
}

void CodeEditor::unfoldAll()
{
    // Make all blocks visible
    int lineCount = document()->blockCount();
    for (int i = 0; i < lineCount; ++i) {
        setBlockVisible(i, true);
    }

    foldedBlocks.clear();
    viewport()->update();
    lineNumberArea->update();
    document()->markContentsDirty(0, document()->characterCount());
}

// Public accessors for protected methods
QTextBlock CodeEditor::getFirstVisibleBlock() const
{
    return firstVisibleBlock();
}

QRectF CodeEditor::getBlockBoundingGeometry(const QTextBlock &block) const
{
    return blockBoundingGeometry(block);
}

QRectF CodeEditor::getBlockBoundingRect(const QTextBlock &block) const
{
    return blockBoundingRect(block);
}

QPointF CodeEditor::getContentOffset() const
{
    return contentOffset();
}

// LineNumberArea implementation
void LineNumberArea::mousePressEvent(QMouseEvent *event)
{
    // Calculate which line was clicked
    QTextBlock block = codeEditor->getFirstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(codeEditor->getBlockBoundingGeometry(block).translated(codeEditor->getContentOffset()).top());
    int bottom = top + qRound(codeEditor->getBlockBoundingRect(block).height());

    while (block.isValid()) {
        if (block.isVisible() && top <= event->pos().y() && event->pos().y() < bottom) {
            // Check if click is on the fold indicator area (left 12 pixels)
            if (event->pos().x() < 12 && codeEditor->isFoldable(blockNumber)) {
                codeEditor->toggleFold(blockNumber);
            }
            break;
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(codeEditor->getBlockBoundingRect(block).height());
        ++blockNumber;
    }

    QWidget::mousePressEvent(event);
}