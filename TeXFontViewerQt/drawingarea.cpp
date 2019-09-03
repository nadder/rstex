/*

Copyright (C) 2018 by Richard Sandberg.

*/

#include <QPaintEvent>
#include <QPainter>
#include <QBitmap>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include "drawingarea.h"

using std::max;
using std::min;

DrawingArea::DrawingArea(QWidget *parent) : QWidget(parent),cur_char(-1),zoom_factor(1),
    userxStart(10), useryStart(10), xStart(0), yStart(0), show_ref(false),
    show_grid(false), show_chardx(false), show_charwd(false), width(0), height(0)
{

}

void DrawingArea::sizeDrawingArea()
{

    int x_off = char_info[cur_char].x_off;
    int y_off = char_info[cur_char].y_off;

    int up_left_x = 0;
    int up_left_y = 0;
    int origin_x = up_left_x + x_off*zoom_factor;
    int origin_y = up_left_y + (y_off+1)*zoom_factor;
    int ppx_esc = int(round(origin_x + char_info[cur_char].horz_esc*zoom_factor));
    int ppx = int(round(origin_x + char_info[cur_char].tfm_width*zoom_factor));
    int min_x = std::min(origin_x, 0);
    int max_x = std::max(width, ppx_esc);
    max_x = std::max(max_x, ppx);
    int min_y = std::min(origin_y, 0);
    int max_y = std::max(height, origin_y);
    xStart = yStart = 0;
    if (min_x < 0) {
        xStart = -min_x;


    }
    if (min_y < 0) {
        yStart = -min_y;

    }

    int drawingAreaWidth = max_x - min_x;
    int drawingAreaHeight = max_y - min_y;

    this->resize(drawingAreaWidth+userxStart+20, drawingAreaHeight+useryStart+20);

}

void DrawingArea::SetZoomBuf()
{
    zoomed_buf.clear();
    width = char_info[cur_char].width;
    height = char_info[cur_char].height;
    if (width == 0 && height == 0)
        return; // empty raster
    zoom_raster(zoom_factor,
                width,
                height,
                image_raster[eight_bits(cur_char)],
                zoomed_buf);
    width *= zoom_factor;
    height *= zoom_factor;

    // monochrome bitmap needs to have each scan line byte aligned
    const int nfill = (8 - width % 8)%8;
    std::vector<eight_bits> new_buf;
    if (nfill > 0) {
        const unsigned new_size = unsigned(1*((width+7)/8)*height);
        new_buf.resize(new_size);
        for (int yy = 0; yy < height; yy++) {
            for (int xx = 0; xx < width; xx++) {
                bool bitval = get_image_raster_bit(unsigned(xx + yy*width), zoomed_buf);
                set_image_raster_bit(bitval, unsigned(xx + (width+nfill)*yy), new_buf);
            }
            for (int pp = 0; pp < nfill; pp++) { // fill with zeros to make the row multiple of 8
                set_image_raster_bit(0, unsigned((width+nfill)*yy + width + pp), new_buf);
            }
        }
        //free(zoomed_buf);
        zoomed_buf = new_buf;
    }


    sizeDrawingArea();


}



void draw_grid(QPainter& painter, int start_x, int start_y, int end_x, int end_y, int grid_spacing)
{
    if (grid_spacing < 4) return;
    QColor clr(122,122,122);
    painter.setPen(clr);

    int cur_y = start_y;
    while (cur_y <= end_y) {
        painter.drawLine(start_x, cur_y, end_x+1,cur_y);
        cur_y += grid_spacing;
    }

    int cur_x = start_x;
    while (cur_x <= end_x) {
        painter.drawLine(cur_x, start_y, cur_x,end_y+1);
        cur_x += grid_spacing;
    }
}


void DrawingArea::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setBackgroundMode(Qt::TransparentMode);

    if (num_chars < 1 || (width == 0 && height == 0))
        return;

    QBitmap bitmap = QBitmap::fromData(QSize(width, height), zoomed_buf.data(), QImage::Format_Mono);

    painter.setPen(curFgColor);

    painter.drawPixmap(xStart+userxStart,yStart+useryStart,bitmap);

    int x_off = char_info[cur_char].x_off;
    int y_off = char_info[cur_char].y_off;
    int up_left_x = xStart+userxStart;
    int up_left_y = yStart+useryStart;
    int origin_x = up_left_x + x_off*zoom_factor;
    int origin_y = up_left_y + (y_off+1)*zoom_factor;

    if (show_grid)
    {
        draw_grid(painter, up_left_x, up_left_y, up_left_x + char_info[cur_char].width *zoom_factor, up_left_y + char_info[cur_char].height*zoom_factor, zoom_factor);
    }

    int ppx_esc = int(round(origin_x + char_info[cur_char].horz_esc*zoom_factor));

    if (show_chardx) {

        QColor escColor(0,0,0);
        QPen pen;
        pen.setWidth(1);
        pen.setColor(escColor);
        painter.setPen(pen);

        painter.drawRect(origin_x,up_left_y, ppx_esc-origin_x, char_info[cur_char].height*zoom_factor);

    }
    int ppx = int(round(origin_x + char_info[cur_char].tfm_width*zoom_factor));
    if (show_charwd) {
        QColor wdColor(30,220,10);
        QPen pen;
        pen.setStyle(Qt::DashLine);
        pen.setColor(wdColor);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawRect(origin_x,up_left_y, ppx - origin_x, char_info[cur_char].height*zoom_factor);
    }


    if (show_ref) {
        int circleWidth = 1*zoom_factor;
        if (circleWidth < 7) circleWidth = 7;
        if (circleWidth > 19) circleWidth = 19;
        if (circleWidth % 2 == 0) circleWidth++;

        painter.setBrush(Qt::red);
        painter.setPen(Qt::red);
        painter.drawEllipse(QPoint(origin_x, origin_y), circleWidth/2,circleWidth/2);

    }


}
