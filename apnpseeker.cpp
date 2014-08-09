#include "apnpseeker.h"

#include <QDebug>

#include "aputils.h"
#include "mainwindow.h"

APNpSeeker::APNpSeeker(QObject *object, QObject *parent) :
    APSeeker(object, parent)
{
    setUp();
}

SegmentType APNpSeeker::type()
{
    return Np;
}

void APNpSeeker::findLowSlopeDeviations(QList<float> indices)
{
    emit workThrowsMessage("Finding low slope deviations...");

    for (int i = 0; i < indices.count() - 1; i++) {
        if (_shouldStop) return;

        int index = indices[i];
        int nextIndex = indices[i + 1];

        float slope = APUtils::slope(QPointF(0, points[index]),
                                     QPointF(10, points[nextIndex])) * 100;
//        writeLog(tr("%1\t%2").arg(transformNumberOfPointsToSeconds(index)).arg(slope));

        if (qAbs(slope) < param("maximumNormalizedSlope")) {
            if (!didFindAnythingYet())
                setProposedStart(index);
        } else {
            if (didFindAnythingYet() && proposedRangeSatisfyMinimumDuration(index))
                pushSegment(proposedStart(), index);
            unsetProposedStart();
        }

        if (i % (indices.count() / 100) == 0) {
            emit progressDidChange(qRound(i * 100.0 / (float)indices.count()));
        }
    }

    if (didFindAnythingYet()) {
        if (proposedRangeSatisfyMinimumDuration(points.count() - 1)) {
            pushSegment(proposedStart(), points.count() - 1);
        }
    }
}

void APNpSeeker::loadDefaults()
{
    APSeeker::loadDefaults(); // should always call super

    setParam("minDurationToBeConsidered", transformSecondsToNumberOfPoints(.65));
    setParam("duration", transformSecondsToNumberOfPoints(5));
    setParam("allowedDistance", transformSecondsToNumberOfPoints(3));
    setParam("peakWindowSize", 16);
    setParam("maximumNormalizedSlope", .75);

    //	setParam("needsLog", true);
}

void APNpSeeker::seek()
{
    QList<float> topPeaks = *(peakIndices(Up));
    QList<float> bottomPeaks = *(peakIndices(Down));

//    MainWindow::instance()->epsignalWidget()->pushDebugPoints(topPeaks, Qt::red);
//    MainWindow::instance()->epsignalWidget()->pushDebugPoints(bottomPeaks, Qt::blue);

    QList<float> indices;
    indices.append(topPeaks);
    indices.append(bottomPeaks);
    emit workThrowsMessage("Sorting found peaks...");
    qSort(indices);

    findLowSlopeDeviations(indices);
}
