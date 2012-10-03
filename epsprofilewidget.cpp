#include "epsprofilewidget.h"

#include <QPainter>
#include <QtAlgorithms>
#include <QDebug>
#include <QMouseEvent>

#include "epsignalscontroller.h"
#include "apviewporthandler.h"
#include "aputils.h"
#include "apsegmentdialog.h"
#include "apsegmentdialog.h"
#include "mainwindow.h"
#include "apsegmenttypescontroller.h"

#define SegmentRowVerticalSpace 30
#define HorizontalHeaderWidth 32

QColor *EPSProfileWidget::_segmentLineColor = NULL;

EPSProfileWidget::EPSProfileWidget(QWidget *parent) :
	QFrame(parent)
{
	_profile = NULL;
	setMinimumHeight(SegmentRowVerticalSpace * 2);

	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange(EPSignal*)),
			this, SLOT(currentSignalDidChange(EPSignal*)));
	connect(APViewportHandler::shared(), SIGNAL(offsetDidChange()),
			this, SLOT(update()));
	connect(APViewportHandler::shared(), SIGNAL(zoomDidChange()),
			this, SLOT(update()));
	connect(this, SIGNAL(focusedSegmentDidChange()),
			this, SLOT(update()));

	_focusedSegment = NULL;
	isMouseDown = false;
	isDragging = false;
	isMouseOverHeader = false;

	innerVerticalOffset = 0;
	isOversized = false;
}

void EPSProfileWidget::currentSignalDidChange(EPSignal *signal)
{
	if (_profile)
		_profile->disconnect(this);

	_profile = !signal ? NULL : signal->profile();
	if (_profile) {
		this->connect(_profile, SIGNAL(segmentsDidChange()), SLOT(segmentsDidChange()));
		updateMaximumHeight();
	} else
		update();
}

void EPSProfileWidget::dragEvent(QMouseEvent *event)
{
	APViewportHandler *viewport = APViewportHandler::shared();
	int hotspot = viewport->transform(event->x());
//	qDebug() << "EPSProfileWidget::dragEvent(" << event->x() << ") => hotspot=" << hotspot;

	/* if hotspot is at any widget's edges, move offset until either limit has reached */ {
		if (event->x() < 5 && viewport->offset() > 0) { // if offset has reached its ends, do nothing
			viewport->setOffset(viewport->offset() - abs(event->x()));
			hotspot = viewport->transform(5);
		} else if (event->x() > width() - 5 && !viewport->hasBeenReachedOffsetLimit()) {
			viewport->setOffset(viewport->offset() + abs(event->x() - width()));
			hotspot = viewport->transform(width() - 5);
		}
	}

	/* check minimum length*/ {
		int proposedLength = draggingEnd == Left ? focusedSegment()->end() - hotspot : hotspot - focusedSegment()->start();
		if (proposedLength < transformSecondsToNumberOfPoints(1))
			return;
	}

	/* update focused segment ends */ {
		QSettings settings;
		if (draggingEnd == Left) {
			if (settings.value(LiveSegmentResizingKey, false).toBool())
				focusedSegment()->setStart(hotspot < 0 ? 0 : hotspot);
			else
				focusedSegment()->setTranscientStart(hotspot < 0 ? 0 : hotspot);
		} else {
			int numberOfPoints = EPSignalsController::activeSignal()->numberOfPoints();
			if (settings.value(LiveSegmentResizingKey, false).toBool())
				focusedSegment()->setEnd(hotspot > numberOfPoints ? numberOfPoints : hotspot);
			else
				focusedSegment()->setTranscientEnd(hotspot > numberOfPoints ? numberOfPoints : hotspot);
		}
	}

	update();
}

void EPSProfileWidget::enterEvent(QEvent *)
{

}

EPSegment *EPSProfileWidget::focusedSegment()
{
	return _focusedSegment;
}

bool EPSProfileWidget::isFocusedSegment(EPSegment *segment)
{
	return segment == _focusedSegment;
}

bool EPSProfileWidget::isMouseOverAnySegmentEnds(QPoint position)
{
	APViewportHandler *viewport = APViewportHandler::shared();

	EPSegment *segmentUnderCursor = segmentAtPosition(position);
	if (!segmentUnderCursor) return false;

	uint hotspot = viewport->transform(position.x());
	bool isAtLeftEnd = abs(hotspot - segmentUnderCursor->start()) < 4 / viewport->horizontalMultiplier();
	bool isAtRightEnd = abs(hotspot - segmentUnderCursor->end()) < 4 / viewport->horizontalMultiplier();
	return isAtLeftEnd || isAtRightEnd;
}

void EPSProfileWidget::leaveEvent(QEvent *)
{

}

void EPSProfileWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	setFocusedSegment( segmentAtPosition(event->pos()) );
	APSegmentDialog::instance()->runForSegment( focusedSegment() );
}

void EPSProfileWidget::mouseMoveEvent(QMouseEvent *event)
{
	bool mouseHasMovedIntoHeader = event->x() < HorizontalHeaderWidth && event->x() > 0;
	if (mouseHasMovedIntoHeader != isMouseOverHeader) {
		isMouseOverHeader = mouseHasMovedIntoHeader;
		update();
	}

	if (isDragging || isMouseDown) {
		if (isDragging)
			dragEvent(event);
		return;
	}

	if (isMouseOverAnySegmentEnds(event->pos())) setCursor(Qt::SizeHorCursor);
	else unsetCursor();
}

void EPSProfileWidget::mousePressEvent(QMouseEvent *event)
{
	setFocusedSegment( segmentAtPosition(event->pos()) );
	if (isMouseOverAnySegmentEnds(event->pos())) {
		isDragging = true;

		int point = APViewportHandler::shared()->transform(event->x());
		int distanceToLeft = abs(focusedSegment()->start() - point);
		int distanceToRight = abs(focusedSegment()->end() - point);
		draggingEnd = distanceToLeft < distanceToRight ? Left : Right;

		QSettings settings;
		if (!settings.value(LiveSegmentResizingKey, false).toBool()) // in live resizing, we don't need transcient values
			focusedSegment()->setTranscientRange(focusedSegment()->start(), focusedSegment()->end());
	}
	isMouseDown = true;
}

void EPSProfileWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (isDragging) {
		unsetCursor();

		QSettings settings;
		if (!settings.value(LiveSegmentResizingKey, false).toBool()) { // in live resizing, we don't need to ask about collisions
			QList<EPSegment *> collidedSegments = focusedSegment()->findCollisions();
			bool shouldAcceptChanges = true;
			if (!collidedSegments.isEmpty()) {
				shouldAcceptChanges = APSegmentDialog::askAboutCollisions(focusedSegment(), collidedSegments) == QMessageBox::Save;
			}

			if (shouldAcceptChanges) focusedSegment()->pushChanges();
			else focusedSegment()->resetTranscientValues();

			update();
			MainWindow::instance()->epsignalWidget()->overlay()->forceToUpdateGeometry();
		}
	} else // if it's was dragging, don't change focused segment
		setFocusedSegment( segmentAtPosition(event->pos()) );

	isMouseDown = false;
	isDragging = false;
}

void EPSProfileWidget::paintDescriptionForSegment(QPainter *painter, EPSegment *segment, QRectF rect)
{
	/* define aligment */
	bool beyondLeftBorder = rect.left() < 0;
	bool beyondRightBorder = rect.right() > width();

	if (beyondLeftBorder) rect.setLeft(0);
	if (beyondRightBorder) rect.setRight(width());

	int aligment = Qt::AlignHCenter;
	if (beyondLeftBorder && !beyondRightBorder) {
		aligment = Qt::AlignRight;
	} else if (beyondRightBorder && !beyondLeftBorder)
		aligment = Qt::AlignLeft;
	/* end define aligment */

	/* drop shadow */
	painter->setPen(QColor(255, 255, 255, 90));
	painter->drawText(rect.adjusted(8, 5, -8, -4),
					  Qt::AlignVCenter | aligment,
					  segment->description());
	/* end drop shadow */

	painter->setPen(QColor(0, 0, 0, 128));
	painter->drawText(rect.adjusted(8, 4, -8, -4),
					  Qt::AlignVCenter | aligment,
					  segment->description());
}

void EPSProfileWidget::paintEvent(QPaintEvent *event)
{
	if (!_profile) return; // no profile, do not draw nothing

	QPainter *painter = new QPainter(this);

	/* font styling */
	QFont font = QFont();
	font.setBold(true);
	painter->setFont(font);
	painter->setRenderHint(QPainter::TextAntialiasing);
	/* end font styling */

	if (isOversized)
		painter->translate(0, -innerVerticalOffset);

	APUtils::paintHorizontalGridInRect(painter, rect());
	paintHorizontalGrid(painter);

	QList<int> types = _profile->currentTypes();
	uint verticalOffset = SegmentRowVerticalSpace;
	foreach (int type, types) {
		paintSegmentsOfType(painter, type, verticalOffset);
		verticalOffset += SegmentRowVerticalSpace;
	}

	paintHeaderForTypes(painter, types);

	delete painter;
	QFrame::paintEvent(event); // call super
}

void EPSProfileWidget::paintHeaderForTypes(QPainter *painter, QList<int> types)
{
	painter->save();

	bool shouldPaintTransparent = isMouseOverHeader || isDragging;

	// draw background
	painter->setBrush(QColor(245, 245, 245, shouldPaintTransparent ? 128 : 232)); // alpha 85%
	painter->setPen(Qt::NoPen);
	painter->drawRect(1, 1, HorizontalHeaderWidth, maximumHeight() - 2);

	// right border
	painter->setPen(QColor(216, 216, 216, shouldPaintTransparent ? 128 : 232)); // alpha 85%
	painter->drawLine(HorizontalHeaderWidth + 1, 0, HorizontalHeaderWidth + 1, maximumHeight());

	// draw names
	painter->setPen(QColor(0, 0, 0, shouldPaintTransparent ? 64 : 128));
	uint verticalOffset = SegmentRowVerticalSpace;
	foreach (int type, types) {

		painter->drawText(QRect(0, verticalOffset - 10, HorizontalHeaderWidth, 20),
						  Qt::AlignCenter | Qt::TextSingleLine,
						  APSegmentTypesController::typeForId(type)->name());
		verticalOffset += SegmentRowVerticalSpace;
	}

	painter->restore();
}

void EPSProfileWidget::paintHorizontalGrid(QPainter *painter)
{
	painter->save();

	painter->setPen(EPSProfileWidget::segmentLineColor());
	for (int verticalOffset = SegmentRowVerticalSpace; verticalOffset < height(); verticalOffset += SegmentRowVerticalSpace) {
		painter->drawLine(QPoint(0, verticalOffset), QPoint(width(), verticalOffset));
	}

	painter->restore();
}

void EPSProfileWidget::paintSegment(QPainter *painter, EPSegment *segment, uint verticalOffset)
{
	painter->save();

	APViewportHandler *viewport = APViewportHandler::shared();

	int start = segment->haveTranscientValues() ? segment->transcientStart() : segment->start();
	int end = segment->haveTranscientValues() ? segment->transcientEnd() : segment->end();

	float segmentLeft = (start - (int)viewport->offset()) * viewport->horizontalMultiplier();
	float segmentRight = (end - (int)viewport->offset()) * viewport->horizontalMultiplier();
	QRectF segmentRect = QRectF(QPointF(segmentLeft, verticalOffset - 10),
								QPointF(segmentRight, verticalOffset + 10));
	setPaintAttributesForSegment(painter, segment, segmentRect);
	painter->drawRoundedRect(segmentRect, 2, 2);

	if (isFocusedSegment(segment)) { // draw info
		paintDescriptionForSegment(painter, segment, segmentRect);
	}

	painter->restore();
}

void EPSProfileWidget::paintSegmentsOfType(QPainter *painter, int type, uint verticalOffset)
{
	painter->save();

	APViewportHandler *viewport = APViewportHandler::shared();

	uint left = viewport->offset(), right = viewport->offset() + viewport->numberOfPointsThatCanBeDrawnInWidth(width());
	QList<EPSegment *> segmentsToDraw = _profile->segmentsInRangeOfType(left, right, type);
	foreach (EPSegment *segment, segmentsToDraw) {
		paintSegment(painter, segment, verticalOffset);
	}

	painter->restore();
}

void EPSProfileWidget::reset()
{
	setFocusedSegment(NULL);
}

void EPSProfileWidget::resizeEvent(QResizeEvent *event)
{
	isOversized = maximumHeight() > height();

	if (innerVerticalOffset > 0) {
		int delta = height() - event->oldSize().height();
		if (delta > 0) {
			setInnerVerticalOffset(innerVerticalOffset - delta);
		}
	}
}

EPSegment *EPSProfileWidget::segmentAtPosition(QPoint position)
{
	QList<int> types = _profile->currentTypes();

	int segmentLineVerticalCenter = SegmentRowVerticalSpace - innerVerticalOffset;
	for (int i = 0; i < types.length(); ++i) { // find in which segment type line it is
		if (position.y() > segmentLineVerticalCenter - 10
				&& position.y() < segmentLineVerticalCenter + 12) { // add 2 bottom padding to tracking purpose
			uint hotspot = APViewportHandler::shared()->transform(position.x());
			return _profile->segmentAtOfType(hotspot, types.at(i));
		}
		segmentLineVerticalCenter += SegmentRowVerticalSpace;
	}

	return NULL;
}

QColor EPSProfileWidget::segmentLineColor()
{
	if (_segmentLineColor == NULL) {
		_segmentLineColor = new QColor(0, 0, 0, 20);
	}
	return *_segmentLineColor;
}

void EPSProfileWidget::segmentsDidChange()
{
	setHidden(_profile->isEmpty());
	updateMaximumHeight();
	update();
}

void EPSProfileWidget::setFocusedSegment(EPSegment *segment)
{
	if (_focusedSegment == segment) return;

	_focusedSegment = segment;
	emit focusedSegmentDidChange();
	emit focusedSegmentDidChange(segment);
	emit focusedSegmentDidChange(segment != NULL);

	if (segment && sender() != 0) // if call is from outside, go to that segment
		APViewportHandler::goTo(segment->start());
}

void EPSProfileWidget::setInnerVerticalOffset(int offset)
{
	if (innerVerticalOffset == offset) return;
	innerVerticalOffset = offset;
	update();
}

void EPSProfileWidget::setPaintAttributesForSegment(QPainter *painter, EPSegment *segment, QRectF rect)
{
	// set visual properties
	QColor segmentColor = segment->type()->color();
	bool shouldHighlight = isFocusedSegment(segment);
	QLinearGradient gradient = QLinearGradient(rect.x(), rect.y(), rect.x(), rect.y() + rect.height());
	gradient.setColorAt(0, segmentColor.lighter(shouldHighlight ? 135 : 165));
	gradient.setColorAt(1, segmentColor.lighter(shouldHighlight ? 115 : 145));
	painter->setBrush(gradient);
	painter->setPen(shouldHighlight ? segmentColor.darker(125) : segmentColor.lighter(125));
}

void EPSProfileWidget::updateMaximumHeight()
{
//	qDebug("EPSProfileWidget::updateMaximumHeight()");
	setMaximumHeight(SegmentRowVerticalSpace * (_profile->currentTypes().length() + 1));

	isOversized = maximumHeight() > height();
}

void EPSProfileWidget::wheelEvent(QWheelEvent *event)
{
	if (isOversized) {
		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15;

		int offset = innerVerticalOffset - numSteps*10;
		if (offset < 0) offset = 0;
		else if (offset > maximumHeight() - height()) offset = maximumHeight() - height();

		setInnerVerticalOffset(offset);
	}

	event->accept();
}
