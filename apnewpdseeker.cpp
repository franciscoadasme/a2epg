#include "apnewpdseeker.h"

#include "aputils.h"
#include "mainwindow.h"

APNewPdSeeker::APNewPdSeeker(QObject *object, QObject *parent) :
    APSeeker(object, parent)
{
    setUp();
}

SegmentType APNewPdSeeker::type()
{
    return Pd;
}

void APNewPdSeeker::loadDefaults()
{
    APSeeker::loadDefaults();

    QSettings settings;

    setParam("peakWindowSize", 10);
    setParam("minimumHeight", .6);

    setParam("minimumDuration", transformSecondsToNumberOfPoints(2));
    setParam("maximumDuration",
             transformSecondsToNumberOfPoints(
                 settings.value(PdMaximumDurationKey, 10).toFloat()));

    setParam("needsLog", false);
    setParam("needsCleanUp", false);
}

void APNewPdSeeker::seek()
{
    QList<float> indices = sortedPeakIndices();
    QList<Slope> slopes = findSteepestSlopes(indices);
    findSunkenSegments(slopes);
    chooseShortestSegments();
}

void APNewPdSeeker::chooseShortestSegments()
{
    emit workThrowsMessage("Choosing best segments...");

    QList<QPair<uint, uint> *> segmentsWithSharedEnds;
    for (int i = 0; i < _proposedSegments.count(); i++) {

        QPair<uint, uint> *pair = _proposedSegments[i];
        segmentsWithSharedEnds << pair;

        if (i == _proposedSegments.count() - 1 // at the last one
            || pair->second < _proposedSegments[i + 1]->first) {

            if (segmentsWithSharedEnds.count() > 1) {

                QPair<uint, uint> *shortestSegment = segmentsWithSharedEnds.first();
                for (int j = 1; j < segmentsWithSharedEnds.count(); j++) {
                    int shortestLength = shortestSegment->second
                                         - shortestSegment->first + 1;
                    int segmentLength = segmentsWithSharedEnds[j]->second
                                        - segmentsWithSharedEnds[j]->first + 1;

                    if (segmentLength < shortestLength)
                        shortestSegment = segmentsWithSharedEnds[j];
                }

                segmentsWithSharedEnds.removeOne(shortestSegment); // remove one to keep

                QPair<uint, uint> *pairToDelete;
                foreach (pairToDelete, segmentsWithSharedEnds) {
                    _proposedSegments.removeOne(pairToDelete);
                    i--;
                }
            }

            segmentsWithSharedEnds.clear();
        }
    }
}

QList<Slope> APNewPdSeeker::findSteepestSlopes(QList<float> indices)
{
    emit workThrowsMessage("Finding steepest slopes...");

    QList<Slope> slopes;

    for (int i = 0; i < indices.count(); i++) {
        if (_shouldStop) return slopes;

        int index = indices[i];
        for (int offset = 1; offset < 6 && i + offset < indices.count(); offset++) {
            int nextIndex = indices[i + offset];

            float height = points[nextIndex] - points[index];
            Direction direction = height > 0 ? Up : Down;
            /* Pd beginning (downward slope) should be calculated only between i
             * and i + 3, since Pd always start with a steep slope */
            if (direction == Down && offset > 3) break;

            /* They cannot be to far away */
            if (nextIndex - index
                > transformSecondsToNumberOfPoints(direction == Down ? .35 : .5))
                break;

            /* Pd start slope should measured between top and bottom peaks (not
             * between top and top) */
            float isTopPeak = points[index] > points[index + 1]
                              || points[index] > points[index - 1];
            if (direction == Down && !isTopPeak) break;

            if (qAbs(height) > param("minimumHeight")) {

                /* Look up for near slopes to check which is most likely
                 * (tallest) to be a Pd end */
                bool shouldBeConsidered = true;
                for (int j = slopes.count() - 1; j >= 0; j--) {
                    Slope slope = slopes[j];
                    if (index - slope.left > transformSecondsToNumberOfPoints(.5))
                        break; /* not beyond 100 points */
                    if (direction != slope.direction)
                        /* do not compare if they are not in the same direction */
                        continue;

                    /* If there is already a taller slope, skip current...
                     * remove smaller ones otherwise. */
                    if (qAbs(height) < slope.height) {
                        shouldBeConsidered = false; break;
                    }
                    else { slopes.removeAt(j); }
                }

                /* Add it only when no taller slope has been found */
                if (shouldBeConsidered)
                    slopes << slopeWithLeftRightHeightAndDirection(
                                  index, nextIndex, qAbs(height), direction);
            }
        }

        if (i * 100 % indices.count() == 0) {
            emit progressDidChange(qRound(i * 100.0 / (float)indices.count() * .2));
        }
    }

    return slopes;
}

void APNewPdSeeker::findSunkenSegments(QList<Slope> slopes)
{
    emit workThrowsMessage("Finding sunken segments...");

    for (int i = 0; i < slopes.count() - 1; i++) {
        if (_shouldStop) return;

        Slope slope = slopes[i];
        if (slope.direction == Up) continue; // must start with a downward slope

        for (int j = i + 1; j < slopes.count(); j++) {
            Slope other = slopes[j];

//            EPSignalWidget *epsw = MainWindow::instance()->epsignalWidget();
//            epsw->pushDebugLine(slope.left, points[slope.left],
//                                slope.right, points[slope.right]);
//            epsw->pushDebugLine(other.left, points[other.left],
//                                other.right, points[other.right]);

            if (other.direction == Down) continue; // must end with upward slope

            int length = other.right - slope.left;
            if (length > param("maximumDuration")) break;
            else if (length < param("minimumDuration")) continue;

            // both slopes should have a similar value
            float averageHeight = (slope.height + other.height) / 2;
            if (qAbs(slope.height - other.height) > averageHeight * /*.65*/ .75)
                continue;

            // right end should be equal or higher than left one
            float slopeValue = APUtils::slope(QPointF(slope.left, points[slope.left]),
                    QPointF(other.right, points[other.right]));
            if (slopeValue * 100 < -0.1)
                continue;

            // check that left siblings are higher than inner section
            bool shouldBeConsidered = true;
            float value = points[slope.right];
            for (int i = 0; i < transformSecondsToNumberOfPoints(1); i++) {
                int at = slope.left - transformSecondsToNumberOfPoints(.25) - i;
                float sibling = points[at];
                if (sibling - value < slope.height * .5) {
                    shouldBeConsidered = false;
                    break;
                }
            }
            if (!shouldBeConsidered) continue;

            // calculate inside peaks
            QList<float> innerPoints;
            for (int pos = slope.right + 1; pos < other.left - 1; pos++) {
                innerPoints << points[pos];
            }
            QList<float> topPeaks = APUtils::peaks(innerPoints, Up, 12, false);
            QList<float> bottomPeaks = APUtils::peaks(innerPoints, Down, 12, false);

            if (satisfyStrongVariantRestrictions(slope, other, topPeaks, bottomPeaks)
                || satisfyWeakVariantRestrictions(slope, other, topPeaks, bottomPeaks))
                pushSegment(slope.left, other.right);
        }

        if (i * 100 % slopes.count() == 0) {
            int progress = qRound(i * 100.0 / (float)slopes.count() * .8);
            emit progressDidChange(progress + 20);
            emit workThrowsMessage(tr("Finding sunken segments... %1%").arg(progress));
        }
    }
}

bool APNewPdSeeker::areMostInnerPeaksBeneathRoof(Slope start,
                                                 Slope end,
                                                 QList<float> peaks,
                                                 float minimumRelativeHeight,
                                                 float minimumRelativeCount)
{
    float averageHeight = (start.height + end.height) / 2;

    /* straight line equation */
    float m = APUtils::slope(QPointF(start.left, points[start.left]),
            QPointF(end.right, points[end.right]));
    float x = start.left, y = points[start.left];

    int count = 0;
    foreach (float index, peaks) {

        index = start.right + 1 + index;
        float diff = (m * (index - x) + y) - points[index];

//        EPSignalWidget *epsw = MainWindow::instance()->epsignalWidget();
//        epsw->pushDebugLine(index,
//                            points[index],
//                            index,
//                            m * (index - x) + y,
//                            QColor(0, 0, 255, 63));

        if (diff > averageHeight * minimumRelativeHeight) count++;
        else if (diff / averageHeight < -0.35) return false;
    }

    return count > peaks.count() * minimumRelativeCount;
}

bool APNewPdSeeker::satisfyStrongVariantRestrictions(
        Slope start,
        Slope end,
        QList<float> topPeaks,
        QList<float> bottomPeaks)
{
    int averageHeight = (start.height + end.height) / 2;
    if (averageHeight < 1) return false;

    float m = APUtils::slope(QPointF(start.right, points[start.right]),
            QPointF(end.left, points[end.left]));
    float x = start.right, y = points[start.right];
    foreach (float index, bottomPeaks) {
        index = start.right + 1 + index;
        float diff = (m * (index - x) + y) - points[index];

        if (diff / averageHeight > .75) return false;
    }

    return areMostInnerPeaksBeneathRoof(start, end, topPeaks, .65, .65);
}

bool APNewPdSeeker::satisfyWeakVariantRestrictions(
        Slope start,
        Slope end,
        QList<float> topPeaks,
        QList<float> bottomPeaks)
{
    if (!areMostInnerPeaksBeneathRoof(start, end, topPeaks, .1, .95))
        return false;

    float minimumValue = points[start.left] - start.height * .6,
            lowerThreshold = points[start.right];

//    EPSignalWidget *epsw = MainWindow::instance()->epsignalWidget();
//    epsw->pushDebugLine(start.right + 1,
//                        minimumValue,
//                        end.left - 1,
//                        minimumValue,
//                        Qt::darkCyan);
//    epsw->pushDebugLine(start.right + 1,
//                        lowerThreshold,
//                        end.left - 1,
//                        lowerThreshold,
//                        Qt::darkRed);

    int count = 0;
    for (int i = 0; i < bottomPeaks.count(); i++) {
        int index = start.right + 1 + bottomPeaks[i];

        float value = points[index];
        if (value > minimumValue) count++;
        else if (value < lowerThreshold) return false;

//        epsw->pushDebugLine(index, value, index, minimumValue, Qt::cyan);
    }

    return count > bottomPeaks.count() * .75;
}

QList<float> APNewPdSeeker::sortedPeakIndices()
{
    QList<float> indices;
    indices.append(*(peakIndices(Up)));
    indices.append(*(peakIndices(Down)));

    emit workThrowsMessage("Sorting found peaks...");
    qSort(indices);

    return indices;
}
