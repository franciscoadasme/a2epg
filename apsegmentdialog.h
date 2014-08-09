#ifndef APSEGMENTDIALOG_H
#define APSEGMENTDIALOG_H

#include <QtGui>
#include <QtWidgets>

#include "epsegment.h"

namespace Ui {
class APSegmentDialog;
}

class APSegmentDialog : public QDialog
{
    Q_OBJECT

public:
    static APSegmentDialog *instance();
    static int runForSegment(EPSegment *segment);
    static int askAboutCollisions(EPSegment *segment, QList<EPSegment *> collidedSegments);

public slots:
    void accept();
    void valueDidChanged(QString);

protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

private:
    Ui::APSegmentDialog *ui;
    static APSegmentDialog *_instance;

    EPSegment *_segment;

    explicit APSegmentDialog(QWidget *parent = 0);
    ~APSegmentDialog();
};

#endif // APSEGMENTDIALOG_H
