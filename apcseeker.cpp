#include "apcseeker.h"

#include <QDebug>
#include <QtCore>

#include "aputils.h"
#include "mainwindow.h"

APCSeeker::APCSeeker(QObject *object, QObject *parent) :
    APSequentialSeeker(object, parent)
{
    setUp();
}

SegmentType APCSeeker::type()
{
    return C;
}

void APCSeeker::loadDefaults()
{
    APSequentialSeeker::loadDefaults();

    setParam("windowSize", 75);
    setParam("peakWindowSize", /*35*/16);

    setParam("duration", transformSecondsToNumberOfPoints(4));
    setParam("allowedDistance", transformSecondsToNumberOfPoints(10));

//  setParam("needsCleanUp", false);
    setParam("needsLog", false);
}

bool APCSeeker::isAllowedPoint()
{
    QList<float> topPeakValues = peaksInsideWindow(Up);
    QList<float> bottomPeakValues = peaksInsideWindow(Down);
    if (topPeakValues.isEmpty() || bottomPeakValues.isEmpty()) return false;

    float amplitude = getAmplitude();
    float topDeviation = APUtils::getDeviation(
                             topPeakValues,
                             APUtils::getAverage(topPeakValues))
                         / amplitude;
    float bottomDeviation = APUtils::getDeviation(
                                bottomPeakValues,
                                APUtils::getAverage(bottomPeakValues))
                            / amplitude;

    //	float average = APUtils::getAverage(pointsInsideWindow);
    //	float deviation = APUtils::getDeviation(pointsInsideWindow, average);


    //	writeLog(tr("%1\t%2\t%3\t%4").arg(middle()).arg(amplitude)
    //                               .arg(topDeviation).arg(bottomDeviation));


    return qAbs(topDeviation - bottomDeviation) > .15
            && (topDeviation > .1 || bottomDeviation > .1)
            && amplitude >= .15 && amplitude <= 8;
}
