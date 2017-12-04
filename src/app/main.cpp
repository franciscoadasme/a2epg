#include "mainwindow.h"
#include <QApplication>
#include <QTime>

#include <QDebug>
#include "utils/color.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    MainWindow w;
    w.show();

    return a.exec();
}
