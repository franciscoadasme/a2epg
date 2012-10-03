#ifndef EPSIGNALWIDGET_H
#define EPSIGNALWIDGET_H

#include <QFrame>

#include "epsegmentoverlay.h"
#include "apzoomwidget.h"

class EPSignalWidget : public QFrame
{
    Q_OBJECT
public:
	explicit EPSignalWidget(QWidget *parent = 0);

	bool isOversized();
	uint numberOfPointsThatCanDraw();
	EPSegmentOverlay *overlay();

	void startAddingMode(SegmentType type = All);
	void stopAddingMode();

	void pushDebugPoints(QList<float> points, QColor color = Qt::red);
	void pushDebugLine(QLineF line, QColor color = Qt::red);
	void pushDebugLine(float x1, float y1, float x2, float y2, QColor color = Qt::red);

signals:
	void resized();

public slots:
	void toggleTrackingMode(bool enabled);
	void clearDebugPoints();
	void setVerticalOffset(int value);
	void setRelativeVerticalOffset(int value);

protected:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	bool eventFilter(QObject *object, QEvent *event);

	void paintHorizontalRuler(QPainter *painter);

private:
	EPSegmentOverlay *_overlay;
	APZoomWidget *_zoomWidget;

	float _relativeVerticalOffset;
	int verticalOffset();
	bool isPointAtVerticalAxis(const QPoint & point);

	bool isInAddingMode, isInTrackingMode, isMouseInside, isMovingVerticalAxis;
	SegmentType requestedType;
	EPSegment *temporalSegment;
	int lastMousePosition;

	QList< QPair<QList<float>, QColor> > _debugPoints;
	QList< QPair<QLineF, QColor> > _debugLines;

	void paintMouseLocationInfo(QPainter *painter);
	void paintDebugPoints(QPainter *painter);
	void paintVerticalRuler(QPainter *painter);
	void paintVerticalGrid(QPainter *painter);
};

#endif // EPSIGNALWIDGET_H
