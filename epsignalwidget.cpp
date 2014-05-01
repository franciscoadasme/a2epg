#include "epsignalwidget.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

#include "apviewporthandler.h"
#include "aputils.h"
#include "epsignalscontroller.h"
#include "apsegmentdialog.h"
#include "mainwindow.h"

#define HorizontalRulerHeight 32
#define VerticalHeaderWidth 32

EPSignalWidget::EPSignalWidget(QWidget *parent) :
	QFrame(parent)
{
	setMinimumSize(480, 256);

	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange()),
			this, SLOT(update()));
	connect(APViewportHandler::shared(), SIGNAL(zoomDidChange()),
			this, SLOT(update()));
	connect(APViewportHandler::shared(), SIGNAL(offsetDidChange()),
			this, SLOT(update()));

	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange()), SLOT(clearDebugPoints()));

	_zoomWidget = new APZoomWidget(this);

	_overlay = new EPSegmentOverlay(this);
	connect(this, SIGNAL(resized()),
			_overlay, SLOT(forceToUpdateGeometry()));
	_overlay->installEventFilter(this);

	isInAddingMode = false;
	isInTrackingMode = false;
	isMouseInside = false;
	isMovingVerticalAxis = false;

	temporalSegment = NULL;

	_relativeVerticalOffset = .5;
}

void EPSignalWidget::clearDebugPoints()
{
	_debugPoints.clear();
	_debugLines.clear();
}

void EPSignalWidget::enterEvent(QEvent *)
{
	isMouseInside = true;
	update();
}

bool EPSignalWidget::eventFilter(QObject *, QEvent *event)
{
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		QPoint point = QPoint(mouseEvent->x() + overlay()->x(), mouseEvent->y());
		mouseMoveEvent(new QMouseEvent(mouseEvent->type(), point,
									   mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers()));
		return true;
	}
	return false;
}

bool EPSignalWidget::isOversized()
{
	uint numberOfPointsThatCanBeDrawn = numberOfPointsThatCanDraw();
	return EPSignalsController::activeSignal()->hasMorePointsThan(numberOfPointsThatCanBeDrawn);
}

bool EPSignalWidget::isPointAtVerticalAxis(const QPoint &point)
{
	return qAbs(point.y() - verticalOffset()) < 5;
}

void EPSignalWidget::leaveEvent(QEvent *)
{
	isMouseInside = false;
	update();
	_zoomWidget->slideOut();
}

void EPSignalWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (!EPSignalsController::hasActiveSignal())
		return;

	uint hotspot = APViewportHandler::shared()->transform(event->x());
	lastMousePosition = hotspot;

	if (isInAddingMode) {
		if (temporalSegment) {
			/* Calculate distance from hotspot to each end to check which end should be modified */
			int distanceToStart = abs((int)hotspot - (int)temporalSegment->start());
			int distanceToEnd = abs((int)hotspot - (int)temporalSegment->end());

			if (distanceToStart < distanceToEnd)
				temporalSegment->setStart(hotspot);
			else if (distanceToEnd < distanceToStart)
				temporalSegment->setEnd(hotspot);
			else { /* both end are the same, so use hotspot direction */
				if (hotspot < temporalSegment->start()) {
					temporalSegment->setStart(hotspot);
				} else if (hotspot > temporalSegment->end()) {
					temporalSegment->setEnd(hotspot);
				} else {
					temporalSegment->setStart(hotspot);
					temporalSegment->setEnd(hotspot);
				}
			}
		}
		update();
		return;
	} else if (isInTrackingMode) {
		update();
	} else {
		if (isMovingVerticalAxis) {
			setVerticalOffset(event->y());
		} else {
			if (isPointAtVerticalAxis(event->pos())) setCursor(Qt::OpenHandCursor);
			else unsetCursor();
		}
	}

	if (_zoomWidget->isPointAtHotspot(event->pos()))
		_zoomWidget->slideIn();
	else
		_zoomWidget->slideOut();
}

void EPSignalWidget::mousePressEvent(QMouseEvent *event)
{
	if (isPointAtVerticalAxis(event->pos())) {
		setCursor(Qt::ClosedHandCursor);
		isMovingVerticalAxis = true;
	}
}

void EPSignalWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (!EPSignalsController::hasActiveSignal())
		return;

	if (isInAddingMode) {
		isInTrackingMode = false;

		int hotspot = APViewportHandler::shared()->transform(event->x());
		if (temporalSegment) {
//			temporalSegment->setEnd(hotspot);

      if (temporalSegment->type()->id() != APNotFound
          || APSegmentDialog::instance()->runForSegment(temporalSegment) == QDialog::Accepted) {
				EPSignalsController::activeSignal()->profile()->addSegment(temporalSegment);
				MainWindow::instance()->epsprofileWidget()->setFocusedSegment(temporalSegment);
			} else {
				overlay()->setSegment(NULL);
			}

			temporalSegment = NULL;
			isInAddingMode = false;
			unsetCursor();
			update();
		} else {
			temporalSegment = new EPSegment(requestedType, hotspot, hotspot);
			overlay()->setSegment(temporalSegment);
		}
	} else if (isMovingVerticalAxis) {
		setVerticalOffset(event->y());
		isMovingVerticalAxis = false;
		unsetCursor();
	}
}

uint EPSignalWidget::numberOfPointsThatCanDraw()
{
	return APViewportHandler::numberOfPointsThatCanBeDrawnInWidth(width());
}

EPSegmentOverlay *EPSignalWidget::overlay()
{
	return _overlay;
}

void EPSignalWidget::paintDebugPoints(QPainter *painter)
{
	painter->save();

	for (int i = 0; i < _debugPoints.length(); i++) {
		QPair<QList<float>, QColor> pair = _debugPoints[i];
		painter->setPen(pair.second);
		foreach (int point, pair.first) {
			int x = (point - APViewportHandler::shared()->offset()) * APViewportHandler::shared()->horizontalMultiplier();
			painter->drawLine(x, 0, x, height());
		}
	}

	painter->translate(0, verticalOffset());
	for (int i = 0; i < _debugLines.count(); i++) {

		QPair<QLineF, QColor> pair = _debugLines[i];
		painter->setPen(pair.second);

		float x1 = (pair.first.x1() - APViewportHandler::shared()->offset()) * APViewportHandler::shared()->horizontalMultiplier();
		float x2 = (pair.first.x2() - APViewportHandler::shared()->offset()) * APViewportHandler::shared()->horizontalMultiplier();
		float y1 = pair.first.y1() * APViewportHandler::shared()->verticalMultiplier();
		float y2 = pair.first.y2() * APViewportHandler::shared()->verticalMultiplier();

		painter->drawLine(x1, -y1, x2, -y2);
	}

	painter->restore();
}

void EPSignalWidget::paintEvent(QPaintEvent *paintEvent)
{
	if (!EPSignalsController::hasActiveSignal()) {
		QFrame::paintEvent(paintEvent); // paint frame
		return;
	}

	QPainter *painter = new QPainter(this);

	APUtils::paintVerticalGridInRect(painter, rect());

	painter->save(); {
		painter->translate(0, verticalOffset());
		painter->setPen(Qt::gray);
		painter->drawLine(QPoint(0, 0), QPoint(width(), 0)); // y-axis

		paintVerticalGrid(painter);

		APUtils::paintSignalUsingPainterWithRect(EPSignalsController::activeSignal(), painter, rect());
	} painter->restore();

	paintDebugPoints(painter);

	if (isMouseInside && (isInAddingMode || isInTrackingMode))
		paintMouseLocationInfo(painter);

	paintVerticalRuler(painter);
	paintHorizontalRuler(painter);

	delete painter;
	QFrame::paintEvent(paintEvent); // paint frame
}

void EPSignalWidget::paintVerticalGrid(QPainter *painter)
{
	painter->save();
	painter->setPen(QColor(0, 0, 0, 24));

	for (int i = 1; ; i++) {
		float y = i * APViewportHandler::shared()->verticalMultiplier();

		if (y > height()) break;

		foreach (float at, QList<float>() << y << -y) {
			painter->drawLine(0, at, width(), at);
		}
	}

	painter->restore();
}

void EPSignalWidget::paintVerticalRuler(QPainter *painter)
{
	painter->save();

	// draw background
	painter->setBrush(QColor(245, 245, 245, 166));
	painter->setPen(Qt::NoPen);
	painter->drawRect(1, 0, VerticalHeaderWidth, height());

	// right border
	painter->setPen(QColor(216, 216, 216, 232));
	painter->drawLine(VerticalHeaderWidth + 1, 0, VerticalHeaderWidth + 1, height());

	// draw names
	painter->setPen(QColor(0, 0, 0, 128));

	painter->translate(0, verticalOffset());
	for (int i = 1; ; i++) {
		float y = i * APViewportHandler::shared()->verticalMultiplier();
		if (y > height()) break;

		painter->drawText(QRect(0, y - 10, VerticalHeaderWidth, 20),
						  Qt::AlignCenter | Qt::TextSingleLine,
						  QString::number(-i));
		painter->drawText(QRect(0, -y - 10, VerticalHeaderWidth, 20),
						  Qt::AlignCenter | Qt::TextSingleLine,
						  QString::number(i));
	}

	painter->restore();
}

void EPSignalWidget::paintMouseLocationInfo(QPainter *painter)
{
	painter->save();
	painter->setPen(Qt::black);

	APViewportHandler *viewport = APViewportHandler::shared();

	// re-transform to points using current (possible new) offset
	int x = (lastMousePosition - viewport->offset()) * viewport->horizontalMultiplier();
	QRect frame = QRect(x + 8, 8, 128, 16);

	if (!temporalSegment || isInTrackingMode) // paint line only when start location haven't been set
		painter->drawLine(x, 0, x, height());
	painter->drawText(frame,
					  Qt::AlignVCenter | Qt::AlignLeading,
					  temporalSegment == NULL ? QString::number(transformNumberOfPointsToSeconds(lastMousePosition))
											  : temporalSegment->description());

	painter->restore();
}

void EPSignalWidget::paintHorizontalRuler(QPainter *painter)
{
	painter->save();

	uint pointsPerSecond = 1.0 / APSampleRate;
	APViewportHandler *viewport = APViewportHandler::shared();
	uint offset = viewport->offset();

	// draw background
	painter->setBrush(QColor(245, 245, 245, 166)); // alpha 85%
	painter->setPen(Qt::NoPen);
	painter->drawRect(0, height() - HorizontalRulerHeight, width(), height() - HorizontalRulerHeight);

	// right border
	painter->setPen(QColor(216, 216, 216, 232)); // alpha 85%
	painter->drawLine(0, height() - HorizontalRulerHeight - 1,
					  width(), height() - HorizontalRulerHeight - 1);

	// draw numbers
	painter->setPen(Qt::gray);
	for (uint i = offset; i < offset + width()/viewport->horizontalMultiplier(); ++i) {
		if (i > 0 && i % pointsPerSecond == 0) {
			if (i/100 > 999 && viewport->horizontalMultiplier() < 0.6 && i/100 % 5 != 0) continue;
			if ((i/100 > 99 && i/100 <= 999) && viewport->horizontalMultiplier() < 0.3 && i/100 % 2 != 0) continue;

			painter->drawText(QRect((i - offset) * viewport->horizontalMultiplier() - 20, height() - HorizontalRulerHeight,
									40, HorizontalRulerHeight),
							  Qt::AlignCenter | Qt::TextSingleLine,
							  QString::number(i / 100));
		}
	}

	painter->restore();
}

void EPSignalWidget::pushDebugPoints(QList<float> points, QColor color)
{
	_debugPoints << QPair<QList<float>, QColor>(points, color);
	update();
}

void EPSignalWidget::pushDebugLine(QLineF line, QColor color)
{
	_debugLines << QPair<QLineF, QColor>(line, color);
}

void EPSignalWidget::pushDebugLine(float x1, float y1, float x2, float y2, QColor color)
{
	pushDebugLine(QLineF(QPointF(x1, y1), QPointF(x2, y2)), color);
}

void EPSignalWidget::resizeEvent(QResizeEvent *)
{
	emit resized();
}

void EPSignalWidget::setRelativeVerticalOffset(int value)
{
	_relativeVerticalOffset = value / 100.0;
	update();
}

void EPSignalWidget::setVerticalOffset(int value)
{
	if (value < 25) value = 25;
	else if (value > height() - 25) value = height() - 25;
	setRelativeVerticalOffset((float)value / (float)height() * 100);
}

void EPSignalWidget::startAddingMode(SegmentType type)
{
	setCursor(Qt::CrossCursor);
	isInAddingMode = true;
	isInTrackingMode = false;
	requestedType = type;
	update();
}

void EPSignalWidget::stopAddingMode()
{
	if (!isInAddingMode) return;

	unsetCursor();
	overlay()->setSegment(NULL);
	if (temporalSegment) {
		delete temporalSegment;
		temporalSegment = NULL;
	}
	isInAddingMode = false;

	update();
}

void EPSignalWidget::toggleTrackingMode(bool enabled)
{
	isInTrackingMode = enabled;
	unsetCursor();
	update();
}

int EPSignalWidget::verticalOffset()
{
	return _relativeVerticalOffset * height();
}
