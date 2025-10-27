#include "minimap.h"
#include "codeeditor.h"
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QDebug>

Minimap::Minimap(CodeEditor *editor, QWidget *parent)
    : QWidget(parent)
    , editor(editor)
    , minimapWidth(120)
    , showSyntax(false)
{
    setMinimumWidth(minimapWidth);
    setMaximumWidth(minimapWidth);

    // Connect to editor signals
    connect(editor, &QPlainTextEdit::textChanged, this, &Minimap::onEditorTextChanged);
    connect(editor->verticalScrollBar(), &QScrollBar::valueChanged, this, &Minimap::onEditorScrolled);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &Minimap::updateMinimap);

    // Enable mouse tracking for hover effects
    setMouseTracking(true);

    // Set background color
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(250, 250, 250)); // Light background
    setPalette(pal);
}

void Minimap::setShowSyntax(bool show)
{
    showSyntax = show;
    update();
}

void Minimap::setWidth(int width)
{
    minimapWidth = width;
    setMinimumWidth(width);
    setMaximumWidth(width);
    update();
}

QSize Minimap::sizeHint() const
{
    return QSize(minimapWidth, height());
}

void Minimap::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false); // Faster rendering

    // Fill background
    painter.fillRect(rect(), QColor(250, 250, 250));

    if (!editor) {
        return;
    }

    // Draw the document
    drawDocument(painter);

    // Draw visible region overlay
    drawVisibleRegion(painter);

    // Draw border
    painter.setPen(QColor(224, 224, 224));
    painter.drawLine(0, 0, 0, height());
}

void Minimap::drawDocument(QPainter &painter)
{
    QTextDocument *doc = editor->document();
    if (!doc) {
        return;
    }

    int totalLines = doc->blockCount();
    if (totalLines == 0) {
        return;
    }

    // Calculate scale factor
    double scaleFactor = getScaleFactor();
    int charWidth = 1; // Minimap character width (very small)
    int lineHeight = 1; // Minimap line height (1 pixel per line for dense view)

    // Draw each line as a simplified representation
    QTextBlock block = doc->begin();
    int lineNumber = 0;

    while (block.isValid()) {
        if (!block.isVisible()) {
            block = block.next();
            lineNumber++;
            continue;
        }

        int y = documentLineToMinimapY(lineNumber);

        if (y >= 0 && y < height()) {
            QString lineText = block.text();
            int textLength = qMin(lineText.length(), minimapWidth / charWidth);

            if (textLength > 0) {
                // Draw simplified line representation
                // For now, just draw a gray bar for non-empty lines
                int barWidth = qMin(textLength * charWidth, minimapWidth - 4);

                if (!lineText.trimmed().isEmpty()) {
                    painter.fillRect(2, y, barWidth, lineHeight, QColor(100, 100, 100, 50));
                }
            }
        }

        block = block.next();
        lineNumber++;
    }
}

void Minimap::drawVisibleRegion(QPainter &painter)
{
    QRect visibleRect = getVisibleRegionRect();

    // Draw semi-transparent overlay for visible region
    painter.fillRect(visibleRect, QColor(68, 130, 180, 40)); // Blue tint

    // Draw border around visible region
    painter.setPen(QPen(QColor(68, 130, 180), 2));
    painter.drawRect(visibleRect);
}

QRect Minimap::getVisibleRegionRect() const
{
    if (!editor) {
        return QRect();
    }

    QTextDocument *doc = editor->document();
    if (!doc) {
        return QRect();
    }

    // Get first and last visible blocks
    QTextBlock firstBlock = editor->getFirstVisibleBlock();
    int firstLine = firstBlock.blockNumber();

    // Estimate last visible line
    int viewportHeight = editor->viewport()->height();
    QFontMetrics fm(editor->font());
    int linesPerPage = viewportHeight / fm.lineSpacing();
    int lastLine = qMin(firstLine + linesPerPage, doc->blockCount() - 1);

    // Convert to minimap coordinates
    int topY = documentLineToMinimapY(firstLine);
    int bottomY = documentLineToMinimapY(lastLine);
    int height = qMax(bottomY - topY, 20); // Minimum 20 pixels

    return QRect(0, topY, width(), height);
}

int Minimap::documentLineToMinimapY(int lineNumber) const
{
    if (!editor) {
        return 0;
    }

    QTextDocument *doc = editor->document();
    if (!doc) {
        return 0;
    }

    int totalLines = doc->blockCount();
    if (totalLines == 0) {
        return 0;
    }

    // Map line number to minimap Y coordinate
    double ratio = (double)lineNumber / (double)totalLines;
    return (int)(ratio * height());
}

int Minimap::minimapYToDocumentLine(int y) const
{
    if (!editor) {
        return 0;
    }

    QTextDocument *doc = editor->document();
    if (!doc) {
        return 0;
    }

    int totalLines = doc->blockCount();
    if (totalLines == 0) {
        return 0;
    }

    // Map minimap Y to line number
    double ratio = (double)y / (double)height();
    return qBound(0, (int)(ratio * totalLines), totalLines - 1);
}

double Minimap::getScaleFactor() const
{
    if (!editor) {
        return 1.0;
    }

    QTextDocument *doc = editor->document();
    if (!doc) {
        return 1.0;
    }

    int totalLines = doc->blockCount();
    if (totalLines == 0) {
        return 1.0;
    }

    // Calculate how many lines fit in the minimap height
    return (double)height() / (double)totalLines;
}

void Minimap::mousePressEvent(QMouseEvent *event)
{
    if (!editor) {
        return;
    }

    // Convert mouse Y to document line
    int lineNumber = minimapYToDocumentLine(event->pos().y());

    // Scroll editor to that line
    QTextBlock block = editor->document()->findBlockByNumber(lineNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        editor->setTextCursor(cursor);
        editor->centerCursor();
    }

    update();
}

void Minimap::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        // Dragging - scroll continuously
        mousePressEvent(event);
    }
}

void Minimap::wheelEvent(QWheelEvent *event)
{
    // Forward scroll events to editor
    if (editor) {
        QApplication::sendEvent(editor->verticalScrollBar(), event);
    }
}

void Minimap::updateMinimap()
{
    update();
}

void Minimap::onEditorScrolled()
{
    update();
}

void Minimap::onEditorTextChanged()
{
    update();
}
