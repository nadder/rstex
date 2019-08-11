#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include "read_fonts.h"
class DrawingArea : public QWidget
{
    Q_OBJECT
public:
    int cur_char;
    int zoom_factor;
    int userxStart, useryStart;
    int xStart, yStart;
    bool show_ref;
    bool show_grid;
    bool show_chardx;
    bool show_charwd;
    void sizeDrawingArea();
    explicit DrawingArea(QWidget *parent = nullptr);
    void SetZoomBuf();
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<eight_bits> zoomed_buf;
    int width, height;
public:
    QColor curFgColor;
private:
signals:

public slots:
};

#endif // DRAWINGAREA_H
