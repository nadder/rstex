#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScrollBar>
#include "aboutdialog.h"
#include "read_fonts.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    drawingarea = new DrawingArea;
    scrollarea = new QScrollArea;
    scrollarea->setWidget(drawingarea);

    scrollarea->viewport()->installEventFilter(this);

    setCentralWidget(scrollarea);
    InitStatusbar();

}



void MainWindow::InitStatusbar()
{
    QStatusBar *pStatusBar = statusBar();
    labelZoom = new QLabel("100%");
    labelZoom->setMinimumSize(labelZoom->sizeHint());
    pStatusBar->addWidget(labelZoom);

    labelCharIndex = new QLabel("");
    labelCharIndex->setMinimumSize(labelCharIndex->sizeHint());
    pStatusBar->addWidget(labelCharIndex);

    labelWidth = new QLabel("");
    labelWidth->setMinimumSize(labelWidth->sizeHint());
    pStatusBar->addWidget(labelWidth);

    labelRes = new QLabel("");
    labelRes->setMinimumSize(labelRes->sizeHint());
    pStatusBar->addWidget(labelRes);

    labelDsgSz = new QLabel("");
    labelDsgSz->setMinimumSize(labelDsgSz->sizeHint());
    pStatusBar->addWidget(labelDsgSz);

    labelOffset = new QLabel("");
    labelOffset->setMinimumSize(labelOffset->sizeHint());
    pStatusBar->addWidget(labelOffset);


    UpdateStatusbar();

}


void MainWindow::UpdateStatusbar()
{

    labelZoom->setText(QString::number(drawingarea->zoom_factor*100) + "%");

    if (drawingarea->cur_char >= 0)
    {
        labelCharIndex->setText(QString("ord:%1, code:%2").arg(drawingarea->cur_char).arg(char_info[drawingarea->cur_char].code));
        QString qs;
        qs.sprintf("charwd: px:%.2f,pt:%.2f,chardx:%.0f", char_info[drawingarea->cur_char].tfm_width,
                char_info[drawingarea->cur_char].tfm_width/char_info[drawingarea->cur_char].hppp,
                char_info[drawingarea->cur_char].horz_esc);
        labelWidth->setText(qs);
        qs = QString::number(char_info[drawingarea->cur_char].hppp*72.27, 'f', 2);

        labelRes->setText(QString("ppi: ") + qs);
        qs = QString::number(char_info[drawingarea->cur_char].design_size, 'f', 2);
        labelDsgSz->setText(QString("designsz:")+qs);
        qs.sprintf("xoff:%d, yoff:%d, w:%d, h:%d", char_info[drawingarea->cur_char].x_off,
                   char_info[drawingarea->cur_char].y_off,
                   char_info[drawingarea->cur_char].width,
                   char_info[drawingarea->cur_char].height);
        labelOffset->setText(qs);
    }
    else {
        labelCharIndex->setText("");
        labelWidth->setText("");
        labelRes->setText("");
        labelDsgSz->setText("");
        labelOffset->setText("");
    }

}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel && QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        QWheelEvent*p = static_cast<QWheelEvent*>(event);
        if (p->delta() > 0)
            on_actionzoomin_triggered();
        else {
            on_actionzoomout_triggered();
        }
        return true;
    }
    else {
        return QObject::eventFilter(obj, event);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    drawingarea->sizeDrawingArea();
}

void MainWindow::keyPressEvent(QKeyEvent* keyevent)
{
    if (keyevent->key() == Qt::Key_N && keyevent->modifiers() == Qt::ShiftModifier) {
        on_actionprev_triggered();
    }
    else if (keyevent->key() == Qt::Key_N && keyevent->modifiers() != Qt::ShiftModifier) {
        on_actionnext_triggered();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionOpen_triggered()
{
    QString the_filename = QFileDialog::getOpenFileName(this,
        tr("Open Font"), "", tr("Font Files (*.*gf *.*pk *.*mf *.*pxl)"));

    if (the_filename.length() > 0) {
        bool mf_file = the_filename.endsWith(".mf",Qt::CaseInsensitive);
        if (mf_file) {
            char cmd[512];
            if (mf_file) {

                sprintf(cmd, "mf \"\\mode=localfont; input %s\"", qPrintable(the_filename));
                system(cmd);
                the_filename = the_filename.left(the_filename.length()-2);
                the_filename.append("600gf");
            }
        }
        if (the_filename.endsWith("pxl", Qt::CaseInsensitive)) {
            drawingarea->cur_char = -1;
            try {
                ReadPXLFile(qPrintable(the_filename));
            }
            catch (...) {
                return;
            }
        }

        if (the_filename.endsWith("gf", Qt::CaseInsensitive)) {
            drawingarea->cur_char = -1;
            try {
                ReadGFFile(qPrintable(the_filename));
            }
            catch (...) {
                return;
            }
        }

        if (the_filename.endsWith("pk", Qt::CaseInsensitive)) {
            drawingarea->cur_char = -1;
            try {
                read_pk_file(qPrintable(the_filename));
            }
            catch (...) {
                return;
            }
        }



        QString sTitle;
        sTitle = QString("%1 - %2").arg(QApplication::applicationName()).arg(the_filename);

        setWindowTitle(sTitle);

        if (num_chars > 0)
            drawingarea->cur_char = 0;
        drawingarea->SetZoomBuf(); // update current buffer

        drawingarea->update();
        UpdateStatusbar();
    }

}

void MainWindow::on_actionnext_triggered()
{
    if (num_chars > 0) {
        drawingarea->cur_char++;
        if (drawingarea->cur_char >= num_chars) drawingarea->cur_char = 0;
        drawingarea->SetZoomBuf(); // update current buffer
        drawingarea->update();
        UpdateStatusbar();
    }
}

void MainWindow::on_actionprev_triggered()
{
    if (num_chars > 0) {
        drawingarea->cur_char--;
        if (drawingarea->cur_char < 0) drawingarea->cur_char+=num_chars;
        drawingarea->SetZoomBuf(); // update current buffer
        drawingarea->update();
        UpdateStatusbar();
    }
}

void MainWindow::on_actionzoomin_triggered()
{
    drawingarea->zoom_factor++;
    if (drawingarea->zoom_factor > 20) drawingarea->zoom_factor = 20;
    drawingarea->SetZoomBuf(); // update current buffer
    drawingarea->update();
    UpdateStatusbar();
    UpdateStatusbar();
}

void MainWindow::on_actionzoomout_triggered()
{
    drawingarea->zoom_factor--;
    if (drawingarea->zoom_factor < 1) drawingarea->zoom_factor = 1;
    drawingarea->SetZoomBuf(); // update current buffer
    drawingarea->update();
    UpdateStatusbar();
    UpdateStatusbar();
}

void MainWindow::on_actionright_triggered()
{
    drawingarea->userxStart+=20;
    drawingarea->sizeDrawingArea();
    drawingarea->update();
}

void MainWindow::on_actionup_triggered()
{
    drawingarea->useryStart-=20;
    drawingarea->sizeDrawingArea();
    drawingarea->update();

}

void MainWindow::on_actiondown_triggered()
{

    drawingarea->useryStart+=20;
    drawingarea->sizeDrawingArea();
    drawingarea->update();
}

void MainWindow::on_actionleft_triggered()
{
    drawingarea->userxStart-=20;
    drawingarea->sizeDrawingArea();
    drawingarea->update();
}

void MainWindow::on_actionref_triggered()
{
    drawingarea->show_ref = !drawingarea->show_ref;
    drawingarea->update();
}

void MainWindow::on_actiongrid_triggered()
{
    drawingarea->show_grid = !drawingarea->show_grid;
    drawingarea->update();
}

void MainWindow::on_actionchardx_triggered()
{
    drawingarea->show_chardx = !drawingarea->show_chardx;
    drawingarea->update();
}

void MainWindow::on_actioncharwd_triggered()
{
    drawingarea->show_charwd = !drawingarea->show_charwd;
    drawingarea->update();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}
