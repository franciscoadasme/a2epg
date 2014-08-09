#include "apgseeker.h"

#include <QDebug>
#include <QtCore>

#include "aputils.h"
#include "mainwindow.h"

APGSeeker::APGSeeker(QObject *object, QObject *parent) :
    APSequentialSeeker(object, parent)
{
    setUp();
}

SegmentType APGSeeker::type()
{
    return G;
}

void APGSeeker::loadDefaults()
{
    APSequentialSeeker::loadDefaults();

    setParam("windowSize", 200);
    setParam("minDurationToBeConsidered", param("windowSize"));
    setParam("duration", transformSecondsToNumberOfPoints(/*8*/20));
    setParam("allowedDistance", transformSecondsToNumberOfPoints(2));
    setParam("peakWindowSize", 16);

    //	setParam("needsCleanUp", false);
    //	setParam("needsLog", true);
}

float APGSeeker::deviationOfDistanceBetweenPeaks(QList<float> indices)
{
    if (indices.count() < 3) return 0;

    QList<float> peakDistances;
    for (int i = 0; i < indices.count() - 1; i++) {
        int index = indices[i];
        int nextIndex = indices[i + 1];

        peakDistances << nextIndex - index;
    }
    return APUtils::getDeviation(peakDistances, APUtils::getAverage(peakDistances));
}

bool APGSeeker::isAllowedPoint()
{
    QList<float> topPeakValues = peaksInsideWindow(Up);
    QList<float> bottomPeakValues = peaksInsideWindow(Down);

//    if (middle() == 0) {
//        MainWindow::instance()->epsignalWidget()->pushDebugPoints(*(peakIndices(Up)));
//        MainWindow::instance()->epsignalWidget()->pushDebugPoints(*(peakIndices(Down)), Qt::blue);
//        writeLog(tr("left\tright\ttopFrecuency\tbottomFrecuency\t"
//                    "top peaks deviation\tbottom peaks deviation\t"
//                    "top peak distance deviation\tbottom peak distance deviation"));
//    }

    float topFrecuency = getFrecuency(Up);
    float bottomFrecuency = getFrecuency(Down);
    float amplitude = APUtils::getAverage(topPeakValues)
                      - APUtils::getAverage(bottomPeakValues);
    float normalizedTopPeaksDeviation = APUtils::getDeviation(
                                            topPeakValues,
                                            APUtils::getAverage(topPeakValues))
                                        / amplitude;
    float normalizedBottomPeaksDeviation = APUtils::getDeviation(
                                               bottomPeakValues,
                                               APUtils::getAverage(bottomPeakValues))
                                           / amplitude;
    float deviationOfDistanceBetweenTopPeaks =
            deviationOfDistanceBetweenPeaks(peaksInsideWindow(Up, false));
    float deviationOfDistanceBetweenBottomPeaks =
            deviationOfDistanceBetweenPeaks(peaksInsideWindow(Down, false));

//    writeLog(tr("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8").arg(left()).arg(right()).arg(topFrecuency)
//             .arg(bottomFrecuency).arg(normalizedTopPeaksDeviation).arg(normalizedBottomPeaksDeviation)
//             .arg(deviationOfDistanceBetweenTopPeaks).arg(deviationOfDistanceBetweenBottomPeaks));

    bool satisfyFrecuencyRestriction = topFrecuency > 3.5 && topFrecuency < 8
                                       && bottomFrecuency > 3 && bottomFrecuency < 8;
    bool satisfyAmplitudeRestriction = amplitude > .35;
    bool satisfyPeakDeviationRestriction = normalizedTopPeaksDeviation < .19
                                           && normalizedBottomPeaksDeviation < .19;
    bool satisfyPeakDistanceRestriction = deviationOfDistanceBetweenTopPeaks < 8.5
                                          && deviationOfDistanceBetweenBottomPeaks < 8.5;

    return satisfyFrecuencyRestriction
            && satisfyAmplitudeRestriction
            && satisfyPeakDeviationRestriction
            && satisfyPeakDistanceRestriction < 8.5;
}
