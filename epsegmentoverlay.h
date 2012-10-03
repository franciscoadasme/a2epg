#ifndef EPSEGMENTOVERLAY_H
#define EPSEGMENTOVERLAY_H

#include <QWidget>
#include <QLabel>

#include "epsegment.h"

class EPSegmentOverlay : public QWidget
{
    Q_OBJECT
public:
	explicit EPSegmentOverlay(QWidget *parent = 0);

signals:

public slots:
    void setSegment(EPSegment *);
    void forceToUpdateGeometry();

protected:
	void paintEvent(QPaintEvent *);

private:
	EPSegment *_segment;
};

#endif // EPSEGMENTOVERLAY_H
