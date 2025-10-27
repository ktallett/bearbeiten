#ifndef MINIMAP_H
#define MINIMAP_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTextDocument>

class CodeEditor;

class Minimap : public QWidget
{
    Q_OBJECT

public:
    explicit Minimap(CodeEditor *editor, QWidget *parent = nullptr);

    // Configuration
    void setShowSyntax(bool show);
    void setWidth(int width);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void updateMinimap();
    void onEditorScrolled();
    void onEditorTextChanged();

private:
    CodeEditor *editor;
    int minimapWidth;
    bool showSyntax;

    // Rendering helpers
    void drawDocument(QPainter &painter);
    void drawVisibleRegion(QPainter &painter);
    QRect getVisibleRegionRect() const;
    int documentLineToMinimapY(int lineNumber) const;
    int minimapYToDocumentLine(int y) const;
    double getScaleFactor() const;
};

#endif // MINIMAP_H
