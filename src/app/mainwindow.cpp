#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include "widgets/preferences/preferenceswidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showPreferences()
{
    PreferencesWidget::sharedInstance()->show();
}
