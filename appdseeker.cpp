#include "appdseeker.h"

#include <QtAlgorithms>
#include <QDebug>

#include "mainwindow.h"
#include "aputils.h"

APPdSeeker::APPdSeeker(QObject *object, QObject *parent) :
	APSeeker(object, parent)
{
	setUp();
}

bool APPdSeeker::areInnerPointsLowerThanEnds(int left, int right, QList<float> topPeaks)
{
	float indexOfHighest = points[left] > points[right] ? left : right;
	float highest = points[indexOfHighest];
	float lowest = indexOfHighest == left ? points[right] : points[left];

	int countOfUnallowedPeaks = 0;
	for (int i = 0; i < topPeaks.count(); i++) {
		if (_shouldStop) return false;

		float peakIndice = topPeaks[i];

		float slopeToHighest = APUtils::slope(QPointF(peakIndice - 10, highest),
											  QPointF(peakIndice, points[peakIndice])) * 100;
		if (highest == right) slopeToHighest *= -1;
		float slopeToLowest = APUtils::slope(QPointF(peakIndice - 10, lowest),
											 QPointF(peakIndice, points[peakIndice])) * 100;

		//		writeLog(tr("%1\t%2\t%3\t%4\t%5").arg(left).arg(right).arg(peakIndice).arg(slopeToHighest).arg(slopeToLowest));

		if (slopeToHighest > -5 || slopeToLowest > 2.5) {
			if (++countOfUnallowedPeaks > 2)
				return false;
		}
	}
	return true;
}

bool APPdSeeker::areSurroundingsHigher(Slope start, Slope end)
{
	// we need bottom peaks
	float leftValue = points[start.right];
	float rightValue = points[end.left];

	float numberOfSurroundingPoints = transformSecondsToNumberOfPoints(1);

	float leftCount = 0, rightCount = 0;
	for (int i = 1; i < numberOfSurroundingPoints; i++) {
		if (start.left - i < 0 || end.right + i >= points.count()) break;

		float leftestValue = points[start.left - i];
		float rightestValue = points[end.right + i];

		if (leftestValue - leftValue > .2) leftCount++;
		if (rightestValue - rightValue > .2) rightCount++;
	}
//	qDebug() << "APPdSeeker::areSurroundingsHigher =>" << leftCount << rightCount;

	return leftCount > numberOfSurroundingPoints * .9 ||
			rightCount > numberOfSurroundingPoints * .9;
}

QList<QPair<int, int> > APPdSeeker::chooseLongestSegments(QList<QPair<int, int> > foundSegments)
{
	emit workThrowsMessage("Choosing best segments...");
	QList<QPair<int, int> > segments;

	QList<QPair<int, int> > segmentsWithSharedEnds;
	for (int i = 0; i < foundSegments.count(); i++) {
		if (_shouldStop) return segments;

		segmentsWithSharedEnds << foundSegments[i];

		if (i == foundSegments.count() - 1 // at the last one
				|| (foundSegments[i].first != foundSegments[i + 1].first
					&& foundSegments[i].second != foundSegments[i + 1].second)) {

			QPair<int, int> longestSegment = segmentsWithSharedEnds.first();
			for (int j = 1; j < segmentsWithSharedEnds.count(); j++) {
				int longestLength = longestSegment.second - longestSegment.first + 1;
				int segmentLength = segmentsWithSharedEnds[j].second - segmentsWithSharedEnds[j].first + 1;

				if (segmentLength > longestLength)
					longestSegment = segmentsWithSharedEnds[j];
			}
			segments << longestSegment;

			segmentsWithSharedEnds.clear();
		}
	}

	return segments;
}

QList<QPair<int, Direction> > APPdSeeker::findSlopeChanges(QList<float> peaks)
{
	emit workThrowsMessage("Finding steep slope changes...");

	QList<QPair<int, Direction> > proposedIndices;
	for (int i = 0; i < peaks.length() - 2; i++) {
		if (_shouldStop) return proposedIndices;

		int index = peaks[i];

		foreach (int nextIndex, QList<int>() << peaks[i + 1] /*<< peaks[i + 2]*/) {
			float slope = APUtils::slope(QPointF(0, points[index]), QPointF(10, points[nextIndex])) * 100;
			float height = fabs(points[nextIndex] - points[index]);

			if (fabs(slope) > param("minimumSlope")
					&& height > param("minimumHeight")) {
				proposedIndices << QPair<int, Direction>(slope > 0 ? nextIndex : index, slope > 0 ? Up : Down);
				break;
			}
		}

		if (i % (peaks.count() / 100) == 0) {
			emit progressDidChange(qRound(i * 100.0 / (float)peaks.count() * .2));
		}
	}

	return proposedIndices;
}

QList<QPair<int, int> > APPdSeeker::findSunkenSegments(QList<QPair<int, Direction> > slopeIndices)
{
	emit workThrowsMessage("Finding sunken segments...");

	QList<QPair<int, int> > segments;

	for (int i = 0; i < slopeIndices.count() - 1; i++) {
		if (_shouldStop) return segments;

		QPair<int, Direction> slopeIndex = slopeIndices[i];

		if (slopeIndex.second == Up) continue; // must start with a downward slope
		if (points[slopeIndex.first] < points[slopeIndex.first + 1]) continue; // should start with upward peak

		for (int j = i + 1; j < slopeIndices.count(); j++) {
			QPair<int, Direction> nextSlopeIndex = slopeIndices[j];

			if (nextSlopeIndex.second == Down) continue; // must end with upward slope
			else if (nextSlopeIndex.first - slopeIndex.first > param("maximumDuration"))
				break;
			else if (nextSlopeIndex.first - slopeIndex.first < param("minimumDuration"))
				continue;

			segments << QPair<int, int>(slopeIndex.first, nextSlopeIndex.first);
		}

		if (i % (slopeIndices.count() / 100) == 0) {
			emit progressDidChange(qRound(i * 100.0 / (float)slopeIndices.count() * .2) + 20);
		}
	}

	return segments;
}

void APPdSeeker::loadDefaults()
{
	APSeeker::loadDefaults(); // should always call super

	setParam("peakWindowSize", 12);
	setParam("minimumSlope", 5);
	setParam("minimumHeight", .6);

	setParam("minimumDuration", transformSecondsToNumberOfPoints(2));
	setParam("maximumDuration", transformSecondsToNumberOfPoints(10));

  setParam("needsLog", false);
	setParam("needsCleanUp", false);
}

QList<QPair<int, int> > APPdSeeker::filterSunkenSegments(QList<QPair<int, int> > proposedSunkenSegments)
{
	emit workThrowsMessage("Filtering found segments...");

	QList<QPair<int, int> > segments;

	QPair<int, int> segment;
	int i = 0;
	foreach (segment, proposedSunkenSegments) {
		if (_shouldStop) return segments;

		QList<float> topPeaksInsideSegment = peakIndicesInRange(Up, segment.first, segment.second);
		topPeaksInsideSegment.removeFirst();
		topPeaksInsideSegment.removeLast();

		if (points[segment.first] < points[segment.second]
				&& areInnerPointsLowerThanEnds(segment.first, segment.second, topPeaksInsideSegment)
				&& satisfyInnerRestrictions(segment.first, segment.second, topPeaksInsideSegment)) {
			segments << segment;
		}

		if (++i % (proposedSunkenSegments.count() / 100) == 0) {
			int progress = qRound(i * 100.0 / (float)proposedSunkenSegments.count() * .6);
			emit progressDidChange(progress + 40);
			emit workThrowsMessage(tr("Filtering found segments... %1%").arg(progress));
		}
	}

	return segments;
}

bool APPdSeeker::satisfyInnerDeviation(QList<float> topPeaks)
{
	QList<float> slopes;
	for (int i = 0; i < topPeaks.count() - 1; i++) {
		int peakIndice = topPeaks[i];
		int nextPeakIndice = topPeaks[i + 1];

		slopes << APUtils::slope(QPointF(peakIndice, points[peakIndice]),
								 QPointF(nextPeakIndice, points[nextPeakIndice])) * 100;
	}

	float slopeAverage = APUtils::getAverage(slopes);
	float slopeDeviation = APUtils::getDeviation(slopes, slopeAverage);

	if (slopeDeviation > 2) {
//		qDebug().nospace() << "APPdSeeker::satisfyInnerDeviation() => slope deviation greater than 2";
		return false;
	}

	return true;
}

bool APPdSeeker::satisfyInnerPeakHeightRestrictions(int left, int right, QList<float> topPeaks)
{
	QList<float> bottomPeaks = peakIndicesInRange(Down, left, right);
	QList<float> peaks;
	peaks << bottomPeaks << topPeaks;
	qSort(peaks);

	float heightOfLeftCliff = 0;
	for (int i = 0; i < 3; i++) {
		float proposedHeight = qAbs(points[peaks[i]] - points[left]);
		if (proposedHeight > heightOfLeftCliff)
			heightOfLeftCliff = proposedHeight;
	}

	int countOfTinyPeaks = 0, countOfPeaksBiggerThanLeftCliff = 0;
	for (int i = 0; i < peaks.count() - 2; i++) {
		float height = qAbs(points[peaks[i]] - points[peaks[i + 1]]);
		//		writeLog(tr("%1\t%2\t%3\t%4\t%5").arg(left).arg(right).arg(peaks.count()*.25).arg(countOfTinyPeaks).arg(height));

		if (height > heightOfLeftCliff*1.15) {
//			qDebug() << "APPdSeeker::satisfyInnerPeakHeightRestrictions() =>"
//					 << "peak higher than leftCliff*1.15";
			return false;
		} else if (i < 16 // check only first 16 peaks
				   && height > heightOfLeftCliff*.575
				   && ++countOfPeaksBiggerThanLeftCliff > 2) {
//			qDebug() << "APPdSeeker::satisfyInnerPeakHeightRestrictions() =>"
//					 << "amount of peaks higher than leftCliff*.575 exceeds 2";
			return false;
		} else if (height < 0.075 && ++countOfTinyPeaks > peaks.count()*.35) {
//			qDebug() << "APPdSeeker::satisfyInnerPeakHeightRestrictions() =>"
//					 << "amount of peaks lower than 0.075 exceeds peaks.count*.35";
			return false;
		}

		//		writeLog(tr("%1\t%2\t%3\t%4\t%5\t%6").arg(left).arg(right).arg(heightOfLeftCliff)
		//				 .arg(peaks[i]).arg(peaks[i + 1]).arg(height));
	}

	return true;
}

bool APPdSeeker::satisfyInnerRestrictions(int left, int right, QList<float> topPeaks)
{
//	qDebug().nospace() << "APPdSeeker::satisfyInnerRestrictions(" << left << ", " << right << ")";

	//	MainWindow::instance()->epsignalWidget()->pushDebugPoints(topPeaks, Qt::red);
	//	MainWindow::instance()->epsignalWidget()->pushDebugPoints(bottomPeaks, Qt::blue);

	return satisfyInnerDeviation(topPeaks)
			&& satisfyInnerPeakHeightRestrictions(left, right, topPeaks);
}

void APPdSeeker::seek()
{
	QList<float> indices;
	indices.append(*(peakIndices(Up)));
	indices.append(*(peakIndices(Down)));
	emit workThrowsMessage("Sorting found peaks...");
	qSort(indices);

	// find slopes
	QList<Slope> slopes;
	for (int i = 0; i < indices.length(); i++) {
		if (_shouldStop) return;

		int index = indices[i];
		for (int offset = 1; offset < 4 && i + offset < indices.count(); offset++) {
			int nextIndex = indices[i + offset];

			float height = points[nextIndex] - points[index];
			Direction direction = height > 0 ? Up : Down;
			/* Pd beginning (downward slope) should be calculated only between i and i + 1,
				since Pd always start with a steep slope */
			if (direction == Down && offset > 1) break;

			/* Pd start slope should measured between top and bottom peaks (not between top and top) */
			float isTopPeak = points[index] > points[index + 1];
			if (direction == Down && !isTopPeak) break;

			if (qAbs(height) > param("minimumHeight")) {

				/* Look up for near slopes to check which is most likely (tallest) to be a Pd end */
				bool shouldBeConsidered = true;
				for (int j = slopes.count() - 1; j >= 0; j--) {
					Slope slope = slopes[j];
					if (index - slope.left > transformSecondsToNumberOfPoints(1)) break; /* not beyond 100 points */
					if (direction != slope.direction) continue; /* do not compare if they are not in the same direction */

					/* If there is already a taller slope, skip current... remove smaller ones otherwise. */
					if (qAbs(height) < slope.height) { shouldBeConsidered = false; break; }
					else { slopes.removeAt(j); }
				}

				if (shouldBeConsidered) /* Add it only when no taller slope has been found */
					slopes << slopeWithLeftRightHeightAndDirection(index, nextIndex, qAbs(height), direction);
			}
		}

		if (i % (indices.count() / 100) == 0) {
			emit progressDidChange(qRound(i * 100.0 / (float)indices.count() * .2));
		}
	}

//	QList<float> debug;
//	foreach (Slope slope, slopes) {
//		qDebug().nospace() << "APPdSeeker::seek() => left=" << slope.left
//						   << ", right=" << slope.right
//						   << ", direction=" << (slope.direction == Up ? "Up" : "Down");
//		debug << slope.left << slope.right;
//	}
//	MainWindow::instance()->epsignalWidget()->pushDebugPoints(debug);

	// find sunken regions
	QList<QPair<Slope, Slope> > segments;

	for (int i = 0; i < slopes.count() - 1; i++) {
		if (_shouldStop) return;

		Slope slope = slopes[i];
		if (slope.direction == Up) continue; // must start with a downward slope

		for (int j = i + 1; j < slopes.count(); j++) {
			Slope nextSlope = slopes[j];

			if (nextSlope.direction == Down) continue; // must end with upward slope
			else if (nextSlope.right - slope.left > param("maximumDuration")) break;
			else if (nextSlope.right - slope.left < param("minimumDuration")) continue;

			qDebug().nospace() << "APPdSeeker::seek() => left=" << slope.left
								  << ", right=" << nextSlope.right
								  << ", areSurroundingsHigher=" << areSurroundingsHigher(slope, nextSlope);

			pushSegment(slope.left, nextSlope.right);
			segments << QPair<Slope, Slope>(slope, nextSlope);
		}

//		if (i % (slopes.count() / 100) == 0) {
//			emit progressDidChange(qRound(i * 100.0 / (float)slopes.count() * .2) + 20);
//		}
	}
}

SegmentType APPdSeeker::type()
{
	return Pd;
}
