/*

Copyright (C) 2018 by Richard Sandberg.

*/

#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    if (argc > 1) {
        w.do_open_filename(argv[1]);
    }
    return a.exec();
}
