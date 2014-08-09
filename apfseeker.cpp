#include "apfseeker.h"

#include "mainwindow.h"
#include "aputils.h"

APFSeeker::APFSeeker(QObject *object, QObject *parent) :
    APSequentialSeeker(object, parent)
{
    setUp();
}

SegmentType APFSeeker::type()
{
    return F;
}

void APFSeeker::loadDefaults()
{
    APSequentialSeeker::loadDefaults();

    setParam("windowSize", 100);
    setParam("peakWindowSize", 12);

    setParam("minimumTopFrecuency", 5.5);
    setParam("minimumBottomFrecuency", 8);

    setParam("minimumAmplitude", .01);
    setParam("maximumAmplitude", .3);

    setParam("minimumPeakDeviation", .05);
    setParam("maximumPeakDeviation", .4);

    setParam("minDurationToBeConsidered", 100);
    setParam("duration", transformSecondsToNumberOfPoints(/*15*/15));
    setParam("allowedDistance", transformSecondsToNumberOfPoints(/*2*/10));

    //	setParam("needsLog", true);
    //	setParam("needsCleanUp", false);
}

bool APFSeeker::isAllowedPoint()
{
    QList<float> topPeakValues = peaksInsideWindow(Up);
    QList<float> bottomPeakValues = peaksInsideWindow(Down);

//    if (middle() == 0) {
//        EPSignalWidget *epsw = MainWindow::instance()->epsignalWidget();
//        epsw->pushDebugPoints(*(peakIndices(Up)));
//        epsw->pushDebugPoints(*(peakIndices(Down)), Qt::blue);
//        writeLog(tr("left\tright\ttopFrecuency\tbottomFrecuency\tamplitude\t"
//                    "top dev\tbottom dev"));
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

//    writeLog(tr("%1\t%2\t%3\t%4\t%5\t%6\t%7").arg(left()).arg(right())
//             .arg(topFrecuency).arg(bottomFrecuency).arg(amplitude)
//             .arg(normalizedTopPeaksDeviation).arg(normalizedBottomPeaksDeviation));

    bool satisfyFrecuencyRestriction =
            topFrecuency > param("minimumTopFrecuency")
            && bottomFrecuency > param("minimumBottomFrecuency");
    bool satisfyAmplitudeRestriction = amplitude > param("minimumAmplitude")
                                       && amplitude < param("maximumAmplitude");
    bool satisfyPeakDeviationRestriction =
            normalizedTopPeaksDeviation > param("minimumPeakDeviation")
            && normalizedTopPeaksDeviation < param("maximumPeakDeviation")
            && normalizedBottomPeaksDeviation > param("minimumPeakDeviation")
            && normalizedBottomPeaksDeviation < param("maximumPeakDeviation");

    return satisfyFrecuencyRestriction
            && satisfyAmplitudeRestriction
            && satisfyPeakDeviationRestriction;
}
