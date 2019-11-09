/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionchardx;
    QAction *actioncharwd;
    QAction *actionref;
    QAction *actiongrid;
    QAction *actionright;
    QAction *actionup;
    QAction *actiondown;
    QAction *actionleft;
    QAction *actionzoomin;
    QAction *actionzoomout;
    QAction *actionnext;
    QAction *actionprev;
    QAction *actionbkcolor;
    QAction *actionfgcolor;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1021, 575);
        MainWindow->setAutoFillBackground(false);
        MainWindow->setStyleSheet(QString::fromUtf8(""));
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionchardx = new QAction(MainWindow);
        actionchardx->setObjectName(QString::fromUtf8("actionchardx"));
        actionchardx->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix1/chardx.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionchardx->setIcon(icon);
        actioncharwd = new QAction(MainWindow);
        actioncharwd->setObjectName(QString::fromUtf8("actioncharwd"));
        actioncharwd->setCheckable(true);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix1/charwd.png"), QSize(), QIcon::Normal, QIcon::Off);
        actioncharwd->setIcon(icon1);
        actionref = new QAction(MainWindow);
        actionref->setObjectName(QString::fromUtf8("actionref"));
        actionref->setCheckable(true);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/new/prefix1/ref.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionref->setIcon(icon2);
        actiongrid = new QAction(MainWindow);
        actiongrid->setObjectName(QString::fromUtf8("actiongrid"));
        actiongrid->setCheckable(true);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/new/prefix1/grid.png"), QSize(), QIcon::Normal, QIcon::Off);
        actiongrid->setIcon(icon3);
        actionright = new QAction(MainWindow);
        actionright->setObjectName(QString::fromUtf8("actionright"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/new/prefix1/right.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionright->setIcon(icon4);
        actionup = new QAction(MainWindow);
        actionup->setObjectName(QString::fromUtf8("actionup"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/new/prefix1/up.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionup->setIcon(icon5);
        actiondown = new QAction(MainWindow);
        actiondown->setObjectName(QString::fromUtf8("actiondown"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/new/prefix1/down.png"), QSize(), QIcon::Normal, QIcon::Off);
        actiondown->setIcon(icon6);
        actionleft = new QAction(MainWindow);
        actionleft->setObjectName(QString::fromUtf8("actionleft"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/new/prefix1/left.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionleft->setIcon(icon7);
        actionzoomin = new QAction(MainWindow);
        actionzoomin->setObjectName(QString::fromUtf8("actionzoomin"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/new/prefix1/zoomin.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionzoomin->setIcon(icon8);
        actionzoomout = new QAction(MainWindow);
        actionzoomout->setObjectName(QString::fromUtf8("actionzoomout"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/new/prefix1/zoomout.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionzoomout->setIcon(icon9);
        actionnext = new QAction(MainWindow);
        actionnext->setObjectName(QString::fromUtf8("actionnext"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/new/prefix1/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionnext->setIcon(icon10);
        actionprev = new QAction(MainWindow);
        actionprev->setObjectName(QString::fromUtf8("actionprev"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/new/prefix1/prev.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionprev->setIcon(icon11);
        actionbkcolor = new QAction(MainWindow);
        actionbkcolor->setObjectName(QString::fromUtf8("actionbkcolor"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/new/prefix1/bkcolor.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionbkcolor->setIcon(icon12);
        actionfgcolor = new QAction(MainWindow);
        actionfgcolor->setObjectName(QString::fromUtf8("actionfgcolor"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/new/prefix1/fgcolor.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionfgcolor->setIcon(icon13);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1021, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setIconSize(QSize(30, 35));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionExit);
        menuHelp->addAction(actionAbout);
        toolBar->addAction(actionchardx);
        toolBar->addAction(actioncharwd);
        toolBar->addAction(actionref);
        toolBar->addAction(actiongrid);
        toolBar->addAction(actionright);
        toolBar->addAction(actionup);
        toolBar->addAction(actiondown);
        toolBar->addAction(actionleft);
        toolBar->addAction(actionzoomin);
        toolBar->addAction(actionzoomout);
        toolBar->addAction(actionnext);
        toolBar->addAction(actionprev);
        toolBar->addAction(actionbkcolor);
        toolBar->addAction(actionfgcolor);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "TeXFontViewerQt", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open...", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
        actionchardx->setText(QCoreApplication::translate("MainWindow", "chardx", nullptr));
        actioncharwd->setText(QCoreApplication::translate("MainWindow", "charwd", nullptr));
        actionref->setText(QCoreApplication::translate("MainWindow", "ref", nullptr));
        actiongrid->setText(QCoreApplication::translate("MainWindow", "grid", nullptr));
        actionright->setText(QCoreApplication::translate("MainWindow", "right", nullptr));
        actionup->setText(QCoreApplication::translate("MainWindow", "up", nullptr));
        actiondown->setText(QCoreApplication::translate("MainWindow", "down", nullptr));
        actionleft->setText(QCoreApplication::translate("MainWindow", "left", nullptr));
        actionzoomin->setText(QCoreApplication::translate("MainWindow", "zoomin", nullptr));
        actionzoomout->setText(QCoreApplication::translate("MainWindow", "zoomout", nullptr));
        actionnext->setText(QCoreApplication::translate("MainWindow", "next", nullptr));
        actionprev->setText(QCoreApplication::translate("MainWindow", "prev", nullptr));
        actionbkcolor->setText(QCoreApplication::translate("MainWindow", "bkcolor", nullptr));
#if QT_CONFIG(tooltip)
        actionbkcolor->setToolTip(QCoreApplication::translate("MainWindow", "bkcolor", nullptr));
#endif // QT_CONFIG(tooltip)
        actionfgcolor->setText(QCoreApplication::translate("MainWindow", "fgcolor", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
