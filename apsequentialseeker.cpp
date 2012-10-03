#include "apsequentialseeker.h"

#include "aputils.h"
#include <QDebug>

APSequentialSeeker::APSequentialSeeker(QObject *object, QObject *parent) :
	APSeeker(object, parent)
{
	topPeakIndicesOffset = 0;
	bottomPeakIndicesOffset = 0;

	_windowLeft = 0;
	_windowRight = 0;
	_moveable = true;
}

float APSequentialSeeker::getAmplitude()
{
	QList<float> peakUpValues = peaksInsideWindow(Up);
	QList<float> peakDownValues = peaksInsideWindow(Down);

	return APUtils::getAverage(peakUpValues) - APUtils::getAverage(peakDownValues);
}

float APSequentialSeeker::getFrecuency(Direction direction)
{
	QList<float> topPeakValues = peaksInsideWindow(direction);
	return topPeakValues.length() * (transformSecondsToNumberOfPoints(1) / pointsInsideWindow.count());
}

int APSequentialSeeker::left()
{
	return _windowLeft < 0 ? 0 : _windowLeft;
}

void APSequentialSeeker::loadDefaults()
{
	APSeeker::loadDefaults();

	setParam("windowSize", 100);
	setParam("step", 1);
}

void APSequentialSeeker::loadPointsInsideWindow()
{
	if (pointsInsideWindow.isEmpty()) {
		// in load window, we add a new element, so we initialize with windowSize -1
		for (int i = left(); i < right(); i++)
			pointsInsideWindow << points[i];
	} else {
		if (_windowRight < points.count())
			pointsInsideWindow << points[right() - 1];
		if (_windowLeft > 0 && !pointsInsideWindow.isEmpty())
			pointsInsideWindow.removeFirst();
	}
}

int APSequentialSeeker::middle()
{
	return (_windowLeft + _windowRight) / 2;
}

void APSequentialSeeker::move(Direction direction)
{
	int step = param("step");
	if (direction == Backward) {
		step = -step;
	}

	_windowLeft += step;
	_windowRight += step;

	_moveable = middle() <= points.count() - 1;
}

void APSequentialSeeker::moveBack()
{
	move(Backward);
}

void APSequentialSeeker::moveForth()
{
	move(Forward);
}

QList<float> APSequentialSeeker::peaksInsideWindow(Direction direction, bool returnValues)
{
	QList<float> *peakIndices = APSeeker::peakIndices(direction);

	QList<float> *peakIndicesInsideWindow = direction == Up ? &topPeakIndicesInsideWindow : &bottomPeakIndicesInsideWindow;
	int *peakIndicesOffset = direction == Up ? &topPeakIndicesOffset : &bottomPeakIndicesOffset;

	if (peakIndicesInsideWindow->isEmpty()) {
		for (int i = *peakIndicesOffset; i < peakIndices->length() && peakIndices->at(i) <= right(); i++)
			peakIndicesInsideWindow->append(peakIndices->at(i));
	} else {
		while (!peakIndicesInsideWindow->isEmpty() && peakIndicesInsideWindow->first() < left()) {
			peakIndicesInsideWindow->removeFirst();
			(*peakIndicesOffset)++;
		}

		int nextRightPeakIndex = *peakIndicesOffset + peakIndicesInsideWindow->count();
		while (nextRightPeakIndex < peakIndices->length() && peakIndices->at(nextRightPeakIndex) <= right()) {
			peakIndicesInsideWindow->append(peakIndices->at(nextRightPeakIndex));
			nextRightPeakIndex++;
		}
	}

	if (!returnValues) return *peakIndicesInsideWindow;

	QList<float> peakValuesInsideWindow;
	foreach (float peakIndex, *peakIndicesInsideWindow) {
		peakValuesInsideWindow << points[peakIndex];
	}

	return peakValuesInsideWindow;
}

void APSequentialSeeker::pushSegment(uint start, uint end)
{
	APSeeker::pushSegment(start, end);

//	for (int i = 0; i < (right() - end) / param("step") + 1 && i < points.length(); i++)
//		moveForth();
//	while(left() < end && middle() < points.length() - 1)
//		moveForth();
}

int APSequentialSeeker::right()
{
	int righest = points.count() - 1;
	return _windowRight > righest ? righest : _windowRight;
}

void APSequentialSeeker::seek()
{
	int pointsInOnePercent = points.length() / 100;
	while (walkable()) {
		loadPointsInsideWindow();

		// if this region is already added, skip it
		if (_proposedSegments.isEmpty() || left() > (int)_proposedSegments.last()->second) {

			if (isAllowedPoint()) {
				if (!didFindAnythingYet())
					setProposedStart(left());
			} else {
				if (didFindAnythingYet() && proposedRangeSatisfyMinimumDuration(right() - 1))
					pushSegment(proposedStart(), right() - 1);
				unsetProposedStart();
			}
		}

		if (middle() % pointsInOnePercent == 0) {
			emit workThrowsMessage(tr("Proccessing %1%...").arg(qRound(middle() * 100.0 / points.count())));
			emit progressDidChange(qRound(middle() * 100.0 / points.count()));
		}

		moveForth();
	}

	if (didFindAnythingYet()) {
		if (proposedRangeSatisfyMinimumDuration(right() - 1)) {
			pushSegment(proposedStart(), right() - 1);
		}
	}
}

void APSequentialSeeker::setUp()
{
	APSeeker::setUp();

	setUpWindow();
}

void APSequentialSeeker::setUpWindow()
{
	uint halfWindowSize = (param("windowSize") / 2);
	_windowLeft = -halfWindowSize;
	_windowRight = halfWindowSize;
}

bool APSequentialSeeker::walkable()
{
	return _moveable && !_shouldStop;
}
