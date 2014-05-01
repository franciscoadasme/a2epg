#include "apviewporthandler.h"

#include <QDebug>

#include "mainwindow.h"
#include "epsignalscontroller.h"

#define Step 0.1
#define BottomLimit 0.2

APViewportHandler *APViewportHandler::_instance = NULL;

APViewportHandler::APViewportHandler(QObject *parent) :
    QObject(parent)
{
  QSettings settings;
  _zoomValues[Qt::Horizontal] = settings.value(ZoomHorizontalKey, 1).toFloat();
  _zoomValues[Qt::Vertical] = settings.value(ZoomVerticalKey, 1).toFloat();
  _offset = 0;
}

APViewportHandler *APViewportHandler::shared()
{
	if (!_instance) {
		_instance = new APViewportHandler();
	}
	return _instance;
}

void APViewportHandler::changeZoom(Qt::Orientation orientation, float step)
{
  _zoomValues[orientation] += step;
	emit zoomDidChange();

  if (this->isAtZoomLimit(orientation)) {
		emit zoomLimitWasReached(orientation);
	} else
		emit zoomLimitWasAvoid(orientation);
}

void APViewportHandler::decreaseHorizontalZoom()
{
	changeZoom(Qt::Horizontal, -Step);
}

void APViewportHandler::decreaseVerticalZoom()
{
	changeZoom(Qt::Vertical, -Step);
}

void APViewportHandler::goTo(uint position, Qt::AlignmentFlag aligment, QWidget *target)
{
	APViewportHandler *viewport = APViewportHandler::shared();
	int width = MainWindow::instance()->epsignalWidget()->width();
	int numberOfPointsThatCanBeDrawn = numberOfPointsThatCanBeDrawnInWidth(width);

	int left;
	if (aligment == Qt::AlignCenter) {
		left = position - numberOfPointsThatCanBeDrawn / 2; // position should at center
	} else {
		left = position - transformSecondsToNumberOfPoints(1);
	}

	if (left < 0)
		left = 0;
	int maximumNumberOfPoints = EPSignalsController::activeSignal()->numberOfPoints();
	if (left + numberOfPointsThatCanBeDrawn > maximumNumberOfPoints)
		left = maximumNumberOfPoints - numberOfPointsThatCanBeDrawn;

	//	viewport->setOffset(left);

	QPropertyAnimation *animator = new QPropertyAnimation(viewport, "offset");
	animator->setDuration(200);
	animator->setStartValue(QVariant(viewport->offset()));
	animator->setEndValue(QVariant(left));
	animator->setEasingCurve(QEasingCurve::OutQuad);

	if (target)
		connect(animator, SIGNAL(finished()),
				target, SLOT(goToAnimationDidEnd()));

	animator->start();
}

bool APViewportHandler::hasBeenReachedOffsetLimit()
{
	uint width = MainWindow::instance()->epsignalWidget()->width();
	uint maxOffset = EPSignalsController::activeSignal()->numberOfPoints()
			- (int)APViewportHandler::numberOfPointsThatCanBeDrawnInWidth(width);

	return offset() == maxOffset;
}

float APViewportHandler::horizontalZoom()
{
	return _zoomValues[Qt::Horizontal];
}

float APViewportHandler::horizontalMultiplier()
{
	return horizontalZoom() * APViewportHandler::horizontalScale();
}

void APViewportHandler::increaseHorizontalZoom()
{
	changeZoom(Qt::Horizontal, Step);
}

void APViewportHandler::increaseVerticalZoom()
{
	changeZoom(Qt::Vertical, Step);
}

bool APViewportHandler::isAtZoomLimit(Qt::Orientation orientation)
{
  return _zoomValues[orientation] < BottomLimit;
}

uint APViewportHandler::numberOfPointsThatCanBeDrawnInWidth(uint width)
{
	return (float)width / APViewportHandler::shared()->horizontalMultiplier();
}

void APViewportHandler::move(int step)
{
	setOffset(_offset + step);
}

uint APViewportHandler::offset()
{
	return _offset;
}

QString APViewportHandler::printableZoomForOrientation(Qt::Orientation orientation)
{
	float zoomValue = _zoomValues[orientation] * 100.0;
	return tr("%1%").arg(zoomValue);
}

void APViewportHandler::resetZoom()
{
	_zoomValues[Qt::Horizontal] = 1;
	_zoomValues[Qt::Vertical] = 1;
	emit zoomDidChange();

	emit zoomLimitWasAvoid(Qt::Horizontal);
	emit zoomLimitWasAvoid(Qt::Vertical);
}

void APViewportHandler::setOffset(int offset) {
	if (EPSignalsController::hasActiveSignal()) {
		int width = MainWindow::instance()->epsignalWidget()->width();
		int maxOffset = EPSignalsController::activeSignal()->numberOfPoints()
				- (int)APViewportHandler::numberOfPointsThatCanBeDrawnInWidth(width);

		if (offset < 0 || offset > maxOffset)
			return;
	}

	_offset = offset;
	emit offsetDidChange();
	emit offsetDidChange(_offset);
}

float APViewportHandler::transform(float num, Qt::Orientation orientation)
{
	float multiplier = orientation == Qt::Horizontal ? horizontalMultiplier() : verticalMultiplier();
	return num / multiplier + offset();
}

float APViewportHandler::verticalZoom()
{
	return _zoomValues[Qt::Vertical];
}

float APViewportHandler::verticalMultiplier()
{
	return verticalZoom() * APViewportHandler::verticalScale();
}
