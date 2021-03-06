#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("nosze");
    QCoreApplication::setApplicationName("a2epg");

    MainWindow *w = MainWindow::instance();
    w->show();

    return a.exec();
}
