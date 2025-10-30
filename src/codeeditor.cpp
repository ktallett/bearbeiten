#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), compactMode(false),
    showWrapIndicator(true), showColumnRuler(false), wrapColumn(80),
    autoIndent(true), autoCloseBrackets(true), smartBackspace(true),
    showIndentationGuides(true), highlightActiveIndent(true)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::matchBrackets);

    updateLineNumberAreaWidth(0);
    matchBrackets();

    setTabStopDistance(40);

    // Enable word wrap mode by default (wraps at word boundaries)
    setWordWrapMode(QTextOption::WordWrap);

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

            // Draw bookmark indicator if bookmarked
            if (bookmarkedLines.contains(blockNumber)) {
                int indicatorSize = 8;
                int indicatorX = lineNumberArea->width() - indicatorSize - 4;
                int indicatorY = top + (fontMetrics().height() - indicatorSize) / 2;

                painter.save();
                painter.setRenderHint(QPainter::Antialiasing);

                // Draw bookmark as a blue circle
                painter.setBrush(QColor(66, 135, 245)); // Blue bookmark color
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(indicatorX, indicatorY, indicatorSize, indicatorSize);

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

void CodeEditor::setShowWrapIndicator(bool show)
{
    showWrapIndicator = show;
    viewport()->update();
}

void CodeEditor::setWrapColumn(int column)
{
    wrapColumn = column;
    if (showColumnRuler) {
        viewport()->update();
    }
}

void CodeEditor::setShowColumnRuler(bool show)
{
    showColumnRuler = show;
    viewport()->update();
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    // Call base class paint event first
    QPlainTextEdit::paintEvent(event);

    // Draw indentation guides first (so they appear behind text)
    if (showIndentationGuides) {
        QPainter painter(viewport());
        drawIndentationGuides(painter);
    }

    // Draw column ruler if enabled
    if (showColumnRuler && wrapColumn > 0) {
        QPainter painter(viewport());

        // Calculate column position in pixels
        QFontMetrics metrics(font());
        int columnX = metrics.horizontalAdvance(QString(wrapColumn, ' ')) + contentOffset().x();

        // Draw a subtle vertical line
        painter.setPen(QPen(QColor(105, 109, 121, 50), 1, Qt::DashLine)); // --color-fg-secondary with transparency
        painter.drawLine(columnX, 0, columnX, viewport()->height());
    }

    // Draw wrap indicators if enabled and line wrap is on
    if (showWrapIndicator && lineWrapMode() != QPlainTextEdit::NoWrap) {
        QPainter painter(viewport());
        painter.setPen(QColor(105, 109, 121, 100)); // Subtle color for wrap indicators

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QTextLayout *layout = block.layout();

                // If this block has multiple lines (wrapped), draw indicators
                if (layout && layout->lineCount() > 1) {
                    for (int i = 1; i < layout->lineCount(); ++i) {
                        QTextLine line = layout->lineAt(i);
                        int lineY = top + qRound(line.y() + line.height() / 2);

                        // Draw a small arrow/chevron at the left edge
                        int indicatorX = 2;
                        painter.drawText(indicatorX, lineY, "↪"); // Wrap indicator symbol
                    }
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }

    // Draw extra cursors for multiple cursor mode
    if (!extraCursors.isEmpty()) {
        QPainter painter(viewport());

        // Draw all extra cursors
        for (const QTextCursor &cursor : extraCursors) {
            QRect rect = QPlainTextEdit::cursorRect(cursor);
            if (!rect.isNull()) {
                // Draw cursor line
                painter.setPen(QPen(palette().color(QPalette::Text), 2));
                painter.drawLine(rect.topLeft(), rect.bottomLeft());

                // If cursor has selection, draw highlight
                if (cursor.hasSelection()) {
                    // This will be handled by Qt's default selection rendering
                    // We just ensure the cursor itself is visible
                }
            }
        }
    }
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

// Smart editing features implementation

void CodeEditor::setAutoIndent(bool enable)
{
    autoIndent = enable;
}

void CodeEditor::setAutoCloseBrackets(bool enable)
{
    autoCloseBrackets = enable;
}

void CodeEditor::setSmartBackspace(bool enable)
{
    smartBackspace = enable;
}

QString CodeEditor::getIndentationOfLine(const QString &text)
{
    QString indent;
    for (QChar c : text) {
        if (c == ' ' || c == '\t') {
            indent += c;
        } else {
            break;
        }
    }
    return indent;
}

bool CodeEditor::isAutoClosingChar(QChar c)
{
    return c == '(' || c == '[' || c == '{' || c == '"' || c == '\'';
}

QChar CodeEditor::getClosingChar(QChar c)
{
    switch (c.toLatin1()) {
        case '(': return ')';
        case '[': return ']';
        case '{': return '}';
        case '"': return '"';
        case '\'': return '\'';
        default: return QChar();
    }
}

void CodeEditor::handleAutoIndent()
{
    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();
    QTextBlock previousBlock = currentBlock.previous();

    if (previousBlock.isValid()) {
        QString previousText = previousBlock.text();
        QString indent = getIndentationOfLine(previousText);

        // If previous line ends with opening bracket, add extra indentation
        QString trimmedPrev = previousText.trimmed();
        if (!trimmedPrev.isEmpty() && (trimmedPrev.endsWith('{') || trimmedPrev.endsWith('(') || trimmedPrev.endsWith('['))) {
            // Add one level of indentation (4 spaces or 1 tab)
            if (indent.contains('\t')) {
                indent += '\t';
            } else {
                indent += "    ";
            }
        }

        if (!indent.isEmpty()) {
            cursor.insertText(indent);
            setTextCursor(cursor);
        }
    }
}

void CodeEditor::handleAutoCloseBracket(QChar openChar)
{
    QChar closeChar = getClosingChar(openChar);
    if (closeChar.isNull()) {
        return;
    }

    QTextCursor cursor = textCursor();

    // For quotes, check if we're next to the same quote (don't double it)
    if (openChar == '"' || openChar == '\'') {
        int pos = cursor.position();
        QString text = toPlainText();

        // If next character is the same quote, just move cursor forward
        if (pos < text.length() && text[pos] == openChar) {
            cursor.movePosition(QTextCursor::Right);
            setTextCursor(cursor);
            return;
        }
    }

    // Insert the closing character and move cursor back
    cursor.insertText(QString(openChar) + QString(closeChar));
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

void CodeEditor::handleSmartBackspace()
{
    QTextCursor cursor = textCursor();

    // Only apply smart backspace at the beginning of a line (in indentation)
    int positionInBlock = cursor.positionInBlock();
    QString currentLineText = cursor.block().text();
    QString beforeCursor = currentLineText.left(positionInBlock);

    // Check if we're only in whitespace
    if (beforeCursor.trimmed().isEmpty() && !beforeCursor.isEmpty()) {
        int indentLevel = getIndentLevel(beforeCursor);
        int spacesPerIndent = 4; // Standard indent

        // Calculate how many spaces to delete (one indent level or to previous tab stop)
        int deleteCount = indentLevel % spacesPerIndent;
        if (deleteCount == 0) {
            deleteCount = spacesPerIndent;
        }

        // Don't delete more than we have
        deleteCount = qMin(deleteCount, positionInBlock);

        // Delete the calculated number of characters
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, deleteCount);
        cursor.removeSelectedText();
        setTextCursor(cursor);
    } else {
        // Normal backspace behavior
        cursor.deletePreviousChar();
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    // Handle Ctrl+D - Select next occurrence
    if (event->key() == Qt::Key_D && event->modifiers() == Qt::ControlModifier) {
        selectNextOccurrence();
        event->accept();
        return;
    }

    // Handle Alt+Shift+Up - Add cursor above
    if (event->key() == Qt::Key_Up &&
        (event->modifiers() & Qt::AltModifier) &&
        (event->modifiers() & Qt::ShiftModifier)) {
        addCursorAbove();
        event->accept();
        return;
    }

    // Handle Alt+Shift+Down - Add cursor below
    if (event->key() == Qt::Key_Down &&
        (event->modifiers() & Qt::AltModifier) &&
        (event->modifiers() & Qt::ShiftModifier)) {
        addCursorBelow();
        event->accept();
        return;
    }

    // Handle Escape - Clear multiple cursors
    if (event->key() == Qt::Key_Escape && !extraCursors.isEmpty()) {
        clearExtraCursors();
        event->accept();
        return;
    }

    // If we have multiple cursors, handle text input specially
    if (!extraCursors.isEmpty()) {
        // Handle backspace at all cursors
        if (event->key() == Qt::Key_Backspace) {
            removeTextAtAllCursors(1);
            event->accept();
            return;
        }

        // Handle delete at all cursors
        if (event->key() == Qt::Key_Delete) {
            QTextCursor mainCursor = textCursor();
            mainCursor.beginEditBlock();

            for (int i = extraCursors.size() - 1; i >= 0; --i) {
                QTextCursor cursor = extraCursors[i];
                if (cursor.hasSelection()) {
                    cursor.removeSelectedText();
                } else {
                    cursor.deleteChar();
                }
                extraCursors[i] = cursor;
            }

            mainCursor.endEditBlock();
            if (!extraCursors.isEmpty()) {
                setTextCursor(extraCursors.last());
            }
            viewport()->update();
            event->accept();
            return;
        }

        // Handle regular text input at all cursors
        if (!event->text().isEmpty() && event->text()[0].isPrint()) {
            insertTextAtAllCursors(event->text());
            event->accept();
            return;
        }

        // Handle Enter/Return at all cursors
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            insertTextAtAllCursors("\n");
            event->accept();
            return;
        }

        // For other keys (like arrows), clear multiple cursors and handle normally
        if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
            event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_Home || event->key() == Qt::Key_End) {
            clearExtraCursors();
            // Fall through to default handling
        }
    }

    // Handle Return/Enter for auto-indent (single cursor mode)
    if (autoIndent && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        QPlainTextEdit::keyPressEvent(event); // Insert newline first
        handleAutoIndent();
        return;
    }

    // Handle Backspace for smart backspace (single cursor mode)
    if (smartBackspace && event->key() == Qt::Key_Backspace && !textCursor().hasSelection()) {
        handleSmartBackspace();
        return;
    }

    // Handle auto-closing brackets and quotes (single cursor mode)
    if (autoCloseBrackets && event->text().length() == 1) {
        QChar typedChar = event->text()[0];
        if (isAutoClosingChar(typedChar) && !textCursor().hasSelection()) {
            handleAutoCloseBracket(typedChar);
            return;
        }
    }

    // Default behavior for all other keys
    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::trimTrailingWhitespace()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // Save current cursor position
    int originalPosition = cursor.position();
    int originalBlockNumber = cursor.blockNumber();

    // Iterate through all blocks
    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        QString text = block.text();

        // Find trailing whitespace
        int endPos = text.length();
        while (endPos > 0 && (text[endPos - 1] == ' ' || text[endPos - 1] == '\t')) {
            endPos--;
        }

        // If there's trailing whitespace, remove it
        if (endPos < text.length()) {
            QTextCursor blockCursor(block);
            blockCursor.movePosition(QTextCursor::EndOfBlock);
            blockCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, text.length() - endPos);
            blockCursor.removeSelectedText();
        }

        block = block.next();
    }

    // Restore cursor position (approximately, as positions may have shifted)
    cursor.movePosition(QTextCursor::Start);
    for (int i = 0; i < originalBlockNumber; ++i) {
        cursor.movePosition(QTextCursor::NextBlock);
    }

    cursor.endEditBlock();
    setTextCursor(cursor);
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

// ============================================================================
// Multiple Cursors Implementation
// ============================================================================

void CodeEditor::clearExtraCursors()
{
    extraCursors.clear();
    lastSearchText.clear();
    viewport()->update();
}

void CodeEditor::addCursorAtPosition(const QTextCursor &cursor)
{
    // Check if cursor already exists at this position
    for (const QTextCursor &existing : extraCursors) {
        if (existing.position() == cursor.position()) {
            return; // Already have a cursor here
        }
    }

    // Add the main cursor if this is the first extra cursor
    if (extraCursors.isEmpty()) {
        extraCursors.append(textCursor());
    }

    extraCursors.append(cursor);
    sortCursors();
    mergeCursors();
    viewport()->update();
}

void CodeEditor::selectNextOccurrence()
{
    QTextCursor mainCursor = textCursor();
    QString selectedText = mainCursor.selectedText();

    // If nothing is selected, select the word under cursor
    if (selectedText.isEmpty()) {
        mainCursor.select(QTextCursor::WordUnderCursor);
        selectedText = mainCursor.selectedText();
        if (selectedText.isEmpty()) {
            return; // Nothing to select
        }
        setTextCursor(mainCursor);
        lastSearchText = selectedText;
        return;
    }

    // Store the search text from first selection
    if (lastSearchText.isEmpty()) {
        lastSearchText = selectedText;
    }

    // Find next occurrence after the last cursor
    int searchStart = mainCursor.selectionEnd();
    if (!extraCursors.isEmpty()) {
        searchStart = extraCursors.last().selectionEnd();
    }

    QTextDocument *doc = document();
    QTextCursor searchCursor(doc);
    searchCursor.setPosition(searchStart);

    // Search forward
    searchCursor = doc->find(lastSearchText, searchCursor);

    // If not found forward, wrap around from the beginning
    if (searchCursor.isNull()) {
        searchCursor = QTextCursor(doc);
        searchCursor = doc->find(lastSearchText, searchCursor);
    }

    if (!searchCursor.isNull()) {
        // Add the current main cursor to extra cursors if this is the first addition
        if (extraCursors.isEmpty()) {
            extraCursors.append(mainCursor);
        }

        extraCursors.append(searchCursor);
        sortCursors();
        mergeCursors();

        // Set the main cursor to the last one for visual feedback
        setTextCursor(searchCursor);
        viewport()->update();
    }
}

void CodeEditor::addCursorAbove()
{
    QTextCursor mainCursor = textCursor();
    QTextBlock currentBlock = mainCursor.block();
    int columnPos = mainCursor.position() - currentBlock.position();

    // Move to previous block
    QTextBlock aboveBlock = currentBlock.previous();
    if (!aboveBlock.isValid()) {
        return; // Already at first line
    }

    // Create cursor at same column position in the line above
    QTextCursor newCursor(aboveBlock);
    int targetPos = qMin(columnPos, aboveBlock.length() - 1);
    newCursor.setPosition(aboveBlock.position() + targetPos);

    // Add the main cursor to extra cursors if this is the first multi-cursor operation
    if (extraCursors.isEmpty()) {
        extraCursors.append(mainCursor);
    }

    extraCursors.append(newCursor);
    setTextCursor(newCursor); // Move main cursor up
    sortCursors();
    mergeCursors();
    viewport()->update();
}

void CodeEditor::addCursorBelow()
{
    QTextCursor mainCursor = textCursor();
    QTextBlock currentBlock = mainCursor.block();
    int columnPos = mainCursor.position() - currentBlock.position();

    // Move to next block
    QTextBlock belowBlock = currentBlock.next();
    if (!belowBlock.isValid()) {
        return; // Already at last line
    }

    // Create cursor at same column position in the line below
    QTextCursor newCursor(belowBlock);
    int targetPos = qMin(columnPos, belowBlock.length() - 1);
    newCursor.setPosition(belowBlock.position() + targetPos);

    // Add the main cursor to extra cursors if this is the first multi-cursor operation
    if (extraCursors.isEmpty()) {
        extraCursors.append(mainCursor);
    }

    extraCursors.append(newCursor);
    setTextCursor(newCursor); // Move main cursor down
    sortCursors();
    mergeCursors();
    viewport()->update();
}

void CodeEditor::sortCursors()
{
    std::sort(extraCursors.begin(), extraCursors.end(),
              [](const QTextCursor &a, const QTextCursor &b) {
                  return a.position() < b.position();
              });
}

void CodeEditor::mergeCursors()
{
    if (extraCursors.size() < 2) {
        return;
    }

    QList<QTextCursor> merged;
    merged.append(extraCursors.first());

    for (int i = 1; i < extraCursors.size(); ++i) {
        QTextCursor &last = merged.last();
        const QTextCursor &current = extraCursors[i];

        // Check if cursors overlap or are at the same position
        if (current.position() <= last.selectionEnd() &&
            current.position() >= last.selectionStart()) {
            // Merge by extending the selection
            int newEnd = qMax(last.selectionEnd(), current.selectionEnd());
            int newStart = qMin(last.selectionStart(), current.selectionStart());
            last.setPosition(newStart);
            last.setPosition(newEnd, QTextCursor::KeepAnchor);
        } else {
            merged.append(current);
        }
    }

    extraCursors = merged;
}

void CodeEditor::insertTextAtAllCursors(const QString &text)
{
    if (extraCursors.isEmpty()) {
        return; // Normal single cursor behavior handled by default
    }

    // Start edit block for undo
    QTextCursor mainCursor = textCursor();
    mainCursor.beginEditBlock();

    // Insert text at all cursors, starting from the last to preserve positions
    for (int i = extraCursors.size() - 1; i >= 0; --i) {
        QTextCursor cursor = extraCursors[i];
        cursor.insertText(text);
        extraCursors[i] = cursor;
    }

    mainCursor.endEditBlock();

    // Update main cursor to last position
    if (!extraCursors.isEmpty()) {
        setTextCursor(extraCursors.last());
    }

    viewport()->update();
}

void CodeEditor::removeTextAtAllCursors(int length)
{
    if (extraCursors.isEmpty()) {
        return; // Normal single cursor behavior handled by default
    }

    QTextCursor mainCursor = textCursor();
    mainCursor.beginEditBlock();

    // Remove text at all cursors, starting from the last to preserve positions
    for (int i = extraCursors.size() - 1; i >= 0; --i) {
        QTextCursor cursor = extraCursors[i];

        // Remove selection if any, otherwise remove characters
        if (cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            // Remove 'length' characters before cursor
            for (int j = 0; j < length && !cursor.atBlockStart(); ++j) {
                cursor.deletePreviousChar();
            }
        }

        extraCursors[i] = cursor;
    }

    mainCursor.endEditBlock();

    // Update main cursor to last position
    if (!extraCursors.isEmpty()) {
        setTextCursor(extraCursors.last());
    }

    viewport()->update();
}

void CodeEditor::mousePressEvent(QMouseEvent *event)
{
    // Check for Ctrl+Click to add cursor
    if (event->modifiers() & Qt::ControlModifier) {
        QTextCursor cursor = cursorForPosition(event->pos());
        addCursorAtPosition(cursor);
        event->accept();
        return;
    }

    // Clear extra cursors on normal click
    if (!extraCursors.isEmpty()) {
        clearExtraCursors();
    }

    QPlainTextEdit::mousePressEvent(event);
}

// ============================================================================
// Indentation Guides Implementation
// ============================================================================

void CodeEditor::setShowIndentationGuides(bool show)
{
    showIndentationGuides = show;
    viewport()->update();
}

void CodeEditor::setHighlightActiveIndent(bool highlight)
{
    highlightActiveIndent = highlight;
    viewport()->update();
}

int CodeEditor::getBlockIndentLevel(const QTextBlock &block)
{
    if (!block.isValid()) {
        return 0;
    }

    QString text = block.text();
    int indentLevel = 0;
    int tabWidth = 4; // Standard tab width

    for (int i = 0; i < text.length(); ++i) {
        if (text[i] == ' ') {
            indentLevel++;
        } else if (text[i] == '\t') {
            indentLevel += tabWidth;
        } else {
            // First non-whitespace character found
            break;
        }
    }

    return indentLevel;
}

int CodeEditor::getActiveIndentLevel()
{
    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();

    if (!currentBlock.isValid()) {
        return 0;
    }

    // Get the indent level of the current line
    int currentIndent = getBlockIndentLevel(currentBlock);

    // If we're on a line with no indentation, look at surrounding lines
    if (currentIndent == 0) {
        // Check previous non-empty lines
        QTextBlock prevBlock = currentBlock.previous();
        while (prevBlock.isValid() && prevBlock.text().trimmed().isEmpty()) {
            prevBlock = prevBlock.previous();
        }
        if (prevBlock.isValid()) {
            currentIndent = getBlockIndentLevel(prevBlock);
        }
    }

    return currentIndent;
}

void CodeEditor::drawIndentationGuides(QPainter &painter)
{
    if (!showIndentationGuides) {
        return;
    }

    QFontMetrics metrics(font());
    int spaceWidth = metrics.horizontalAdvance(' ');
    int tabWidth = 4; // Standard tab width
    int indentWidth = spaceWidth * tabWidth;

    // Get active indent level for highlighting
    int activeIndent = highlightActiveIndent ? getActiveIndentLevel() : -1;

    // Color for normal indent guides (subtle)
    QColor normalColor = palette().color(QPalette::Mid);
    normalColor.setAlpha(50);

    // Color for active indent guide (more visible)
    QColor activeColor = palette().color(QPalette::Highlight);
    activeColor.setAlpha(100);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= viewport()->rect().bottom()) {
        if (block.isVisible() && bottom >= viewport()->rect().top()) {
            int blockIndent = getBlockIndentLevel(block);

            // Draw vertical lines for each indent level
            for (int level = indentWidth; level < blockIndent; level += indentWidth) {
                int x = level + contentOffset().x();

                // Check if this is the active indent level
                bool isActive = (highlightActiveIndent &&
                               level >= activeIndent - indentWidth &&
                               level <= activeIndent);

                painter.setPen(isActive ? activeColor : normalColor);
                painter.drawLine(x, top, x, bottom);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

// Bookmark implementation

void CodeEditor::toggleBookmark()
{
    QTextCursor cursor = textCursor();
    int lineNumber = cursor.blockNumber();
    toggleBookmarkAtLine(lineNumber);
}

void CodeEditor::toggleBookmarkAtLine(int lineNumber)
{
    if (bookmarkedLines.contains(lineNumber)) {
        bookmarkedLines.remove(lineNumber);
    } else {
        bookmarkedLines.insert(lineNumber);
    }

    // Trigger repaint of line number area
    lineNumberArea->update();
}

void CodeEditor::clearAllBookmarks()
{
    bookmarkedLines.clear();
    lineNumberArea->update();
}

void CodeEditor::goToNextBookmark()
{
    if (bookmarkedLines.isEmpty()) {
        return;
    }

    int currentLine = textCursor().blockNumber();

    // Find the next bookmark after the current line
    int nextBookmark = -1;
    for (int line : bookmarkedLines) {
        if (line > currentLine) {
            if (nextBookmark == -1 || line < nextBookmark) {
                nextBookmark = line;
            }
        }
    }

    // If no bookmark found after current line, wrap to the first one
    if (nextBookmark == -1) {
        nextBookmark = *std::min_element(bookmarkedLines.begin(), bookmarkedLines.end());
    }

    // Move cursor to the bookmarked line
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, nextBookmark);
    setTextCursor(cursor);
    centerCursor();
}

void CodeEditor::goToPreviousBookmark()
{
    if (bookmarkedLines.isEmpty()) {
        return;
    }

    int currentLine = textCursor().blockNumber();

    // Find the previous bookmark before the current line
    int prevBookmark = -1;
    for (int line : bookmarkedLines) {
        if (line < currentLine) {
            if (prevBookmark == -1 || line > prevBookmark) {
                prevBookmark = line;
            }
        }
    }

    // If no bookmark found before current line, wrap to the last one
    if (prevBookmark == -1) {
        prevBookmark = *std::max_element(bookmarkedLines.begin(), bookmarkedLines.end());
    }

    // Move cursor to the bookmarked line
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, prevBookmark);
    setTextCursor(cursor);
    centerCursor();
}

void CodeEditor::setBookmarks(const QSet<int> &bookmarks)
{
    bookmarkedLines = bookmarks;
    lineNumberArea->update();
}

// Line operations implementation

void CodeEditor::duplicateLine()
{
    QTextCursor cursor = textCursor();

    if (hasMultipleCursors()) {
        // Handle multiple cursors
        cursor.beginEditBlock();

        // Duplicate lines for each cursor
        QList<QTextCursor> allCursors;
        allCursors.append(cursor);
        allCursors.append(extraCursors);

        // Sort by position (reverse order to avoid position shifts)
        std::sort(allCursors.begin(), allCursors.end(), [](const QTextCursor &a, const QTextCursor &b) {
            return a.blockNumber() > b.blockNumber();
        });

        for (QTextCursor &c : allCursors) {
            c.movePosition(QTextCursor::StartOfBlock);
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QString lineText = c.selectedText();
            c.movePosition(QTextCursor::EndOfBlock);
            c.insertText("\n" + lineText);
        }

        cursor.endEditBlock();
    } else {
        // Single cursor
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QString lineText = cursor.selectedText();
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.insertText("\n" + lineText);
        cursor.endEditBlock();
    }

    setTextCursor(cursor);
}

void CodeEditor::deleteLine()
{
    QTextCursor cursor = textCursor();

    if (hasMultipleCursors()) {
        // Handle multiple cursors
        cursor.beginEditBlock();

        // Collect unique line numbers to delete
        QSet<int> linesToDelete;
        linesToDelete.insert(cursor.blockNumber());
        for (const QTextCursor &c : extraCursors) {
            linesToDelete.insert(c.blockNumber());
        }

        // Delete lines in reverse order
        QList<int> sortedLines = linesToDelete.values();
        std::sort(sortedLines.begin(), sortedLines.end(), std::greater<int>());

        for (int lineNumber : sortedLines) {
            QTextBlock block = document()->findBlockByNumber(lineNumber);
            if (block.isValid()) {
                cursor.setPosition(block.position());
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

                // Also delete the newline if not the last line
                if (block.next().isValid()) {
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                } else if (block.previous().isValid()) {
                    // If last line, delete the preceding newline
                    cursor.setPosition(block.position() - 1);
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                }

                cursor.removeSelectedText();
            }
        }

        clearExtraCursors();
        cursor.endEditBlock();
    } else {
        // Single cursor
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        // Also delete the newline if not the last line
        QTextBlock block = cursor.block();
        if (block.next().isValid()) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        } else if (block.previous().isValid()) {
            // If last line, delete the preceding newline
            cursor.setPosition(block.position() - 1);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        }

        cursor.removeSelectedText();
        cursor.endEditBlock();
    }

    setTextCursor(cursor);
}

void CodeEditor::moveLineUp()
{
    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();

    // Can't move up if already at first line
    if (currentBlock.blockNumber() == 0) {
        return;
    }

    cursor.beginEditBlock();

    // Get current line text
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString currentLineText = cursor.selectedText();
    int cursorPositionInLine = textCursor().positionInBlock();

    // Delete current line (including newline)
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    // Move to previous line and insert
    cursor.movePosition(QTextCursor::Up);
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText(currentLineText + "\n");

    // Restore cursor position
    cursor.movePosition(QTextCursor::Up);
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, cursorPositionInLine);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void CodeEditor::moveLineDown()
{
    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();

    // Can't move down if already at last line
    if (!currentBlock.next().isValid()) {
        return;
    }

    cursor.beginEditBlock();

    // Get current line text
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString currentLineText = cursor.selectedText();
    int cursorPositionInLine = textCursor().positionInBlock();

    // Delete current line (including newline)
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    // Move to end of next line and insert
    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.insertText("\n" + currentLineText);

    // Restore cursor position
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, cursorPositionInLine);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void CodeEditor::sortLinesAscending()
{
    QTextCursor cursor = textCursor();

    // Determine range to sort
    int startLine, endLine;

    if (cursor.hasSelection()) {
        // Sort selected lines
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        QTextBlock startBlock = document()->findBlock(start);
        QTextBlock endBlock = document()->findBlock(end);

        startLine = startBlock.blockNumber();
        endLine = endBlock.blockNumber();
    } else {
        // Sort entire document
        startLine = 0;
        endLine = document()->blockCount() - 1;
    }

    // Collect lines
    QStringList lines;
    for (int i = startLine; i <= endLine; ++i) {
        QTextBlock block = document()->findBlockByNumber(i);
        if (block.isValid()) {
            lines.append(block.text());
        }
    }

    // Sort lines
    lines.sort(Qt::CaseInsensitive);

    // Replace lines
    cursor.beginEditBlock();

    for (int i = 0; i < lines.size(); ++i) {
        QTextBlock block = document()->findBlockByNumber(startLine + i);
        if (block.isValid()) {
            cursor.setPosition(block.position());
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor.insertText(lines[i]);
        }
    }

    cursor.endEditBlock();
}

void CodeEditor::sortLinesDescending()
{
    QTextCursor cursor = textCursor();

    // Determine range to sort
    int startLine, endLine;

    if (cursor.hasSelection()) {
        // Sort selected lines
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        QTextBlock startBlock = document()->findBlock(start);
        QTextBlock endBlock = document()->findBlock(end);

        startLine = startBlock.blockNumber();
        endLine = endBlock.blockNumber();
    } else {
        // Sort entire document
        startLine = 0;
        endLine = document()->blockCount() - 1;
    }

    // Collect lines
    QStringList lines;
    for (int i = startLine; i <= endLine; ++i) {
        QTextBlock block = document()->findBlockByNumber(i);
        if (block.isValid()) {
            lines.append(block.text());
        }
    }

    // Sort lines descending
    lines.sort(Qt::CaseInsensitive);
    std::reverse(lines.begin(), lines.end());

    // Replace lines
    cursor.beginEditBlock();

    for (int i = 0; i < lines.size(); ++i) {
        QTextBlock block = document()->findBlockByNumber(startLine + i);
        if (block.isValid()) {
            cursor.setPosition(block.position());
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor.insertText(lines[i]);
        }
    }

    cursor.endEditBlock();
}

// Comment operations implementation

void CodeEditor::setCurrentLanguage(const QString &language)
{
    currentLanguage = language;
}

QString CodeEditor::getLineCommentSyntax() const
{
    // Map languages to their line comment syntax
    static QMap<QString, QString> lineComments = {
        {"c", "//"},
        {"cpp", "//"},
        {"c++", "//"},
        {"java", "//"},
        {"javascript", "//"},
        {"typescript", "//"},
        {"go", "//"},
        {"rust", "//"},
        {"swift", "//"},
        {"kotlin", "//"},
        {"csharp", "//"},
        {"c#", "//"},
        {"php", "//"},
        {"python", "#"},
        {"ruby", "#"},
        {"perl", "#"},
        {"shell", "#"},
        {"bash", "#"},
        {"sh", "#"},
        {"yaml", "#"},
        {"toml", "#"},
        {"r", "#"},
        {"lua", "--"},
        {"sql", "--"},
        {"haskell", "--"},
        {"html", "<!--"},
        {"xml", "<!--"},
        {"css", "/*"},
        {"markdown", "<!--"}
    };

    QString lang = currentLanguage.toLower();
    return lineComments.value(lang, "//"); // Default to C-style
}

QPair<QString, QString> CodeEditor::getBlockCommentSyntax() const
{
    // Map languages to their block comment syntax (start, end)
    static QMap<QString, QPair<QString, QString>> blockComments = {
        {"c", {"/*", "*/"}},
        {"cpp", {"/*", "*/"}},
        {"c++", {"/*", "*/"}},
        {"java", {"/*", "*/"}},
        {"javascript", {"/*", "*/"}},
        {"typescript", {"/*", "*/"}},
        {"go", {"/*", "*/"}},
        {"rust", {"/*", "*/"}},
        {"swift", {"/*", "*/"}},
        {"kotlin", {"/*", "*/"}},
        {"csharp", {"/*", "*/"}},
        {"c#", {"/*", "*/"}},
        {"php", {"/*", "*/"}},
        {"css", {"/*", "*/"}},
        {"python", {"\"\"\"", "\"\"\""}},
        {"html", {"<!--", "-->"}},
        {"xml", {"<!--", "-->"}},
        {"markdown", {"<!--", "-->"}},
        {"lua", {"--[[", "]]"}},
        {"haskell", {"{-", "-}"}}
    };

    QString lang = currentLanguage.toLower();
    return blockComments.value(lang, {"/*", "*/"}); // Default to C-style
}

bool CodeEditor::isLineCommented(const QString &line, const QString &commentSyntax) const
{
    QString trimmed = line.trimmed();
    return trimmed.startsWith(commentSyntax);
}

void CodeEditor::toggleLineComment()
{
    QString commentSyntax = getLineCommentSyntax();
    if (commentSyntax.isEmpty()) {
        return;
    }

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    if (hasMultipleCursors()) {
        // Handle multiple cursors
        QSet<int> linesToToggle;
        linesToToggle.insert(cursor.blockNumber());
        for (const QTextCursor &c : extraCursors) {
            linesToToggle.insert(c.blockNumber());
        }

        // Check if all lines are commented
        bool allCommented = true;
        for (int lineNumber : linesToToggle) {
            QTextBlock block = document()->findBlockByNumber(lineNumber);
            if (block.isValid()) {
                if (!isLineCommented(block.text(), commentSyntax)) {
                    allCommented = false;
                    break;
                }
            }
        }

        // Toggle comments on all lines
        for (int lineNumber : linesToToggle) {
            QTextBlock block = document()->findBlockByNumber(lineNumber);
            if (!block.isValid()) continue;

            cursor.setPosition(block.position());
            cursor.movePosition(QTextCursor::StartOfBlock);

            if (allCommented) {
                // Uncomment: find and remove comment syntax
                QString line = block.text();
                int commentPos = line.indexOf(commentSyntax);
                if (commentPos != -1) {
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, commentPos);
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, commentSyntax.length());
                    // Also remove one space after comment if present
                    if (cursor.position() < block.position() + block.length() - 1) {
                        QChar nextChar = document()->characterAt(cursor.position());
                        if (nextChar == ' ') {
                            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                        }
                    }
                    cursor.removeSelectedText();
                }
            } else {
                // Comment: add comment syntax at start
                cursor.insertText(commentSyntax + " ");
            }
        }
    } else {
        // Single cursor - toggle current line
        cursor.movePosition(QTextCursor::StartOfBlock);
        int startPos = cursor.position();
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QString lineText = cursor.selectedText();

        cursor.setPosition(startPos);

        if (isLineCommented(lineText, commentSyntax)) {
            // Uncomment: find and remove comment syntax
            int commentPos = lineText.indexOf(commentSyntax);
            if (commentPos != -1) {
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, commentPos);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, commentSyntax.length());
                // Also remove one space after comment if present
                if (cursor.position() < startPos + lineText.length()) {
                    QChar nextChar = document()->characterAt(cursor.position());
                    if (nextChar == ' ') {
                        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    }
                }
                cursor.removeSelectedText();
            }
        } else {
            // Comment: add comment syntax at start
            cursor.insertText(commentSyntax + " ");
        }
    }

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void CodeEditor::toggleBlockComment()
{
    QPair<QString, QString> blockSyntax = getBlockCommentSyntax();
    if (blockSyntax.first.isEmpty() || blockSyntax.second.isEmpty()) {
        return;
    }

    QTextCursor cursor = textCursor();

    // If there's a selection, toggle block comment on selection
    if (cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        QString selectedText = cursor.selectedText();

        cursor.beginEditBlock();

        // Check if selection is already block commented
        if (selectedText.startsWith(blockSyntax.first) && selectedText.endsWith(blockSyntax.second)) {
            // Uncomment: remove block comment markers
            cursor.setPosition(start);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, blockSyntax.first.length());
            cursor.removeSelectedText();

            // Adjust end position after removing start marker
            end -= blockSyntax.first.length();

            cursor.setPosition(end - blockSyntax.second.length());
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, blockSyntax.second.length());
            cursor.removeSelectedText();

            cursor.setPosition(start);
        } else {
            // Comment: wrap selection with block comment markers
            cursor.setPosition(start);
            cursor.insertText(blockSyntax.first);
            cursor.setPosition(end + blockSyntax.first.length());
            cursor.insertText(blockSyntax.second);

            // Select the commented region
            cursor.setPosition(start);
            cursor.setPosition(end + blockSyntax.first.length() + blockSyntax.second.length(), QTextCursor::KeepAnchor);
        }

        cursor.endEditBlock();
        setTextCursor(cursor);
    } else {
        // No selection - toggle block comment on current line (same as line comment)
        toggleLineComment();
    }
}