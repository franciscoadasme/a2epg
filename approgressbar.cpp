#include "approgressbar.h"

#include <QDebug>

APProgressBar::APProgressBar(QWidget *parent) :
    QProgressBar(parent)
{
    setMinimum(0);
}

void APProgressBar::bindToWorker(APWorker *worker)
{
    connect(worker, SIGNAL(workLengthDidChange(int)),
            this, SLOT(updateMaximum(int)));
    connect(worker, SIGNAL(progressDidChange(int)),
            this, SLOT(setValue(int)));
    connect(worker, SIGNAL(workDidEnd()),
            this, SLOT(workDidEnd()));
}

void APProgressBar::workDidEnd()
{
    setDisabled(true);
}

void APProgressBar::updateMaximum(int maximum)
{
    setMaximum(maximum);
}
