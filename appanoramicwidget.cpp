#include "appanoramicwidget.h"

#include <QPainter>
#include <QDebug>
#include <QResizeEvent>
#include "math.h"

#include "epsignalscontroller.h"
#include "apviewporthandler.h"
#include "mainwindow.h"
#include "APGlobals.h"

#define SignalVerticalPadding 4

APPanoramicKnobWidget::APPanoramicKnobWidget(QWidget *parent) :
	QWidget(parent)
{
	connect(APViewportHandler::shared(), SIGNAL(offsetDidChange()),
			this, SLOT(updateKnobGeometry()));
	connect(APViewportHandler::shared(), SIGNAL(zoomDidChange()),
			this, SLOT(updateKnobGeometry()));
	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange()),
			this, SLOT(updateKnobGeometry()));
	connect((APPanoramicWidget *)parentWidget(), SIGNAL(resized()),
			this, SLOT(updateKnobGeometry()));
}

void APPanoramicKnobWidget::paintEvent(QPaintEvent *)
{
	if (!EPSignalsController::hasActiveSignal())
		return;

	QPainter *painter = new QPainter(this);

	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(0, 255, 0, 128));
	painter->drawRect(rect().adjusted(1, 1, -1, -1));

	delete painter;
}

void APPanoramicKnobWidget::mouseReleaseEvent(QMouseEvent *event)
{
	float horizontalMultiplier = (float)parentWidget()->width() / ((APPanoramicWidget *)parentWidget())->numberOfPointsThatCanBeDrawn();
	uint pos = (geometry().left() + event->x()) / horizontalMultiplier;
	((APPanoramicWidget *)parentWidget())->setShouldUpdateOffset(false);
	APViewportHandler::goTo(((APPanoramicWidget *)parentWidget())->offset() + pos, Qt::AlignLeft, parentWidget());
}

void APPanoramicKnobWidget::updateKnobGeometry()
{
	if (!EPSignalsController::hasActiveSignal())
		return;

	APViewportHandler *viewport = APViewportHandler::shared();
	int offset = ((APPanoramicWidget *)parentWidget())->offset();
	QRect newRect = QRect();

	if ((int)viewport->offset() >= offset
			&& (int)viewport->offset() <= offset + ((APPanoramicWidget *)parentWidget())->numberOfPointsThatCanBeDrawn()) {
		float horizontalMultiplier = (float)parentWidget()->width() / ((APPanoramicWidget *)parentWidget())->numberOfPointsThatCanBeDrawn();

		int numberOfPoints = MainWindow::instance()->epsignalWidget()->numberOfPointsThatCanDraw();

		newRect.translate((viewport->offset() - offset) * horizontalMultiplier, 0);
		newRect.setWidth(numberOfPoints * horizontalMultiplier);
		newRect.setHeight(parentWidget()->height());
	}

	setGeometry(newRect);
}


APPanoramicSignalWidget::APPanoramicSignalWidget(QWidget *parent) :
	QWidget(parent)
{
	prevRect = QRect();
}

void APPanoramicSignalWidget::mouseReleaseEvent(QMouseEvent *event)
{
	float horizontalMultiplier = (float)width() / ((APPanoramicWidget *)parentWidget())->numberOfPointsThatCanBeDrawn();
	uint pos = event->x() / horizontalMultiplier;
	((APPanoramicWidget *)parentWidget())->setShouldUpdateOffset(false);
	APViewportHandler::goTo(((APPanoramicWidget *)parentWidget())->offset() + pos, Qt::AlignLeft, parentWidget());
}

void APPanoramicSignalWidget::paintEvent(QPaintEvent *event)
{
	if (!EPSignalsController::hasActiveSignal())
		return;

	QPainter *painter = new QPainter(this);

	EPSignal *signal = EPSignalsController::activeSignal();
	QRect innerRect = rect().adjusted(0, SignalVerticalPadding, 0, -SignalVerticalPadding);
	float verticalMultiplier = (float)innerRect.height() / signal->amplitude();
	int verticalOffset = signal->maximum() * verticalMultiplier + SignalVerticalPadding;

	painter->translate(0, verticalOffset);
	painter->drawLine(0, 0, width(), 0);
	painter->setPen(Qt::darkGray);

	float horizontalMultiplier = (float)width() / (float)((APPanoramicWidget *)parentWidget())->numberOfPointsThatCanBeDrawn();
	float offset = ((APPanoramicWidget *)parentWidget())->offset();

	/* Draw current and prev rect to avoid undrawn areas while scrolling */
	foreach (QRect rect, QList<QRect>() << prevRect << event->rect()) {
		float from = offset + (float)rect.left() / horizontalMultiplier;
		float to = offset + (float)rect.right() / horizontalMultiplier;
//		qDebug().nospace() << "APParonamicSignalWidget::paintEvent() => from=" << from << " to=" << to;

		QList<float> points = signal->points();
		for (float i = from + 1; i < to
			 && i < offset + numberOfPointsInOneHour()
			 && i < points.length(); i++) {
			// be aware that we multiply by -1 to flip out the signal, so positive values point to top
			QPointF leftPoint = QPointF((i - 1 - offset) * horizontalMultiplier,
										-points.at(i - 1) * verticalMultiplier);
			QPointF rightPoint = QPointF((i - offset) * horizontalMultiplier,
										 -points.at(i) * verticalMultiplier);
			painter->drawLine(leftPoint, rightPoint);
		}
	}

	delete painter;

	prevRect = event->rect().width() == rect().width() ? QRect() : event->rect();
}


APPanoramicWidget::APPanoramicWidget(QWidget *parent) :
	QFrame(parent)
{
	_signalWidget = new APPanoramicSignalWidget(this);
	_knobWidget = new APPanoramicKnobWidget(this);
	_offset = 0;
	_shouldUpdateOffset = true;

	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange(EPSignal*)),
			this, SLOT(updateContent(EPSignal*)));
	connect(APViewportHandler::shared(), SIGNAL(offsetDidChange(int)),
			this, SLOT(updateOffset(int)));
}

void APPanoramicWidget::emitRangeText()
{
	int numberOfPointsInOneHour = 3600 / APSampleRate;
	int numberOfPoints = EPSignalsController::activeSignal()->numberOfPoints();
	int proposedNumberOfPoints = _offset + numberOfPointsInOneHour;
	int to = proposedNumberOfPoints > numberOfPoints ? numberOfPoints : proposedNumberOfPoints;

	int numberOfSections = numberOfPoints / numberOfPointsInOneHour;
	if (numberOfSections < 1) numberOfSections = 1;
	int currentSection = _offset / numberOfPointsInOneHour + 1;

    emit updateRangeText(tr("Showing from %1 to %2 seconds (%3 of %4 hours)")
						 .arg(_offset * APSampleRate)
						 .arg(to * APSampleRate)
						 .arg(currentSection)
						 .arg(numberOfSections));
}

void APPanoramicWidget::goToAnimationDidEnd()
{
	setShouldUpdateOffset(true);
}

void APPanoramicWidget::goToNextSection()
{
	int numberOfPointsInOneHour = 3600 / APSampleRate;
	_offset += numberOfPointsInOneHour;

	if (_offset + numberOfPointsInOneHour >= (int)EPSignalsController::activeSignal()->numberOfPoints())
		emit updateRightArrowState(false);
	else
		emit updateRightArrowState(true);
	emit updateLeftArrowState(true);
	emitRangeText();

	_signalWidget->update();
	_knobWidget->updateKnobGeometry();
}

void APPanoramicWidget::goToPrevSection()
{
	_offset -= numberOfPointsInOneHour();

	if (_offset == 0)
		emit updateLeftArrowState(false);
	else
		emit updateLeftArrowState(true);
	emit updateRightArrowState(true);
	emitRangeText();

	_signalWidget->update();
	_knobWidget->updateKnobGeometry();
}

int APPanoramicWidget::offset()
{
	return _offset;
}

APPanoramicKnobWidget *APPanoramicWidget::knobWidget()
{
	return _knobWidget;
}

int APPanoramicWidget::numberOfPointsThatCanBeDrawn()
{
	EPSignal *signal = EPSignalsController::activeSignal();
	return (int)signal->numberOfPoints() < numberOfPointsInOneHour()
			? signal->numberOfPoints() : numberOfPointsInOneHour();
}

void APPanoramicWidget::resizeEvent(QResizeEvent *)
{
	_signalWidget->setGeometry(rect());
	emit resized();
}

void APPanoramicWidget::setShouldUpdateOffset(bool flag)
{
	_shouldUpdateOffset = flag;
}

APPanoramicSignalWidget *APPanoramicWidget::signalWidget()
{
	return _signalWidget;
}

void APPanoramicWidget::updateContent(EPSignal *signal)
{
	if (!EPSignalsController::hasActiveSignal()) return;

	float numberOfSections = signal->numberOfPoints() / numberOfPointsInOneHour();

	_offset = 0;
	emit updateLeftArrowState(false);
	emit updateRightArrowState(numberOfSections > 1);
	emitRangeText();

	_signalWidget->update();
	_knobWidget->updateKnobGeometry();
}

void APPanoramicWidget::updateOffset(int viewportOffset)
{
	if (!_shouldUpdateOffset || !EPSignalsController::hasActiveSignal())
		return;

	if (viewportOffset > offset() + numberOfPointsThatCanBeDrawn())
		goToNextSection();
	else if (viewportOffset < offset())
		goToPrevSection();
}
