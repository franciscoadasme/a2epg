#ifndef APSTATUSCONTROLLER_H
#define APSTATUSCONTROLLER_H

#include <QObject>
#include <QStatusBar>
#include <QProgressBar>

#include "apworker.h"
#include "approgressbar.h"

class APStatusController : public QObject
{
    Q_OBJECT
public:
    static APStatusController *shared();
    static QStatusBar *statusBar();
    static APProgressBar *progressBar();

    static void bindToWorker(APWorker *worker);
    static void setMessage(QString msg, int timeout = 5 * 1000);
    static void setMessageWithIndeterminateProgress(QString msg);
    static void reset();

public slots:
    void workDidBegin(QString msg);
    void workDidEnd(bool, QObject *, QString msg);

protected:
    explicit APStatusController(QObject *parent = 0);

private:
    static APStatusController *_instance;
    static QStatusBar *_statusBar;
    static APProgressBar *_progressBar;
};

#endif // APSTATUSCONTROLLER_H
