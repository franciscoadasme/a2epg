#ifndef APPROGRESSBAR_H
#define APPROGRESSBAR_H

#include <QProgressBar>
#include "apworker.h"

class APProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit APProgressBar(QWidget *parent = 0);

    void bindToWorker(APWorker *worker);

public slots:
    void updateMaximum(int maximum);
    void workDidEnd();
};

#endif // APPROGRESSBAR_H
