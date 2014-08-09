#include "apstatuscontroller.h"

#include "mainwindow.h"
#include <QProgressBar>

APStatusController *APStatusController::_instance = NULL;
QStatusBar *APStatusController::_statusBar = NULL;
APProgressBar *APStatusController::_progressBar = NULL;

APStatusController::APStatusController(QObject *parent) :
    QObject(parent)
{
    _progressBar->setMinimum(0);
    _progressBar->setMaximum(0);
}

void APStatusController::bindToWorker(APWorker *worker)
{
    progressBar()->bindToWorker(worker);
    connect(worker, SIGNAL(workDidBegin(QString)),
            APStatusController::shared(), SLOT(workDidBegin(QString)));
    connect(worker, SIGNAL(workDidEnd(bool,QObject*,QString)),
            APStatusController::shared(), SLOT(workDidEnd(bool,QObject*,QString)));
}

APProgressBar *APStatusController::progressBar()
{
    if (_progressBar == NULL) {
        _progressBar = new APProgressBar(statusBar());
        statusBar()->addPermanentWidget(_progressBar);
    }
    return _progressBar;
}

void APStatusController::reset()
{
    statusBar()->clearMessage();
    progressBar()->hide();
    progressBar()->setMaximum(0);
}

void APStatusController::setMessage(QString msg, int timeout)
{
    statusBar()->showMessage(msg, timeout);
    progressBar()->hide();
}

void APStatusController::setMessageWithIndeterminateProgress(QString msg)
{
    statusBar()->showMessage(msg);
    progressBar()->setMaximum(0); // indeterminate
    progressBar()->show();
}

APStatusController *APStatusController::shared()
{
    if (_instance == NULL)
        _instance = new APStatusController();
    return _instance;
}

QStatusBar *APStatusController::statusBar()
{
    if (_statusBar == NULL)
        _statusBar = MainWindow::instance()->statusBar();
    return _statusBar;
}

void APStatusController::workDidBegin(QString msg)
{
    statusBar()->showMessage(msg);
    progressBar()->show();
}

void APStatusController::workDidEnd(bool success, QObject *, QString msg)
{
    reset();
    APStatusController::setMessage(success ? msg : "Failed.");
}
