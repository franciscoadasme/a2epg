#include "ape1seeker.h"

#include <QDebug>
#include <QtCore>

#include "mainwindow.h"
#include "aputils.h"

APE1Seeker::APE1Seeker(QObject *object, QObject *parent) :
    APSequentialSeeker(object, parent)
{
    setUp();
}

SegmentType APE1Seeker::type()
{
    return E1;
}

void APE1Seeker::loadDefaults()
{
    APSequentialSeeker::loadDefaults();

    setParam("windowSize", 100);//50
    setParam("minDurationToBeConsidered", 200);
    setParam("duration", transformSecondsToNumberOfPoints(15));
    setParam("allowedDistance", transformSecondsToNumberOfPoints(3));
    setParam("peakWindowSize", 40);//25

    setParam("needsLog", false);
}

bool APE1Seeker::isAllowedPoint()
{
    QList<float> topPeakIndices = peaksInsideWindow(Up,false);
    QList<float> bottomPeakIndices = peaksInsideWindow(Down,false);

    float amplitude = getAmplitude();

    float PEAK_THRESHOLD = .05;
    for (int i=0 ; i < topPeakIndices.length() - 1; i++){
        int left = topPeakIndices.at(i);
        int right = topPeakIndices.at(i + 1);
        for (int j = left + 1; j < right - 1; j++) {

            float value = points[j];
            bool isPeak = value - points[j - 1] > PEAK_THRESHOLD
                          && value - points[j + 1] > PEAK_THRESHOLD;

            if (isPeak)
                return false;
        }
    }

    QList<float> indices;
    indices.append(topPeakIndices);
    indices.append(bottomPeakIndices);

    qSort(indices);

    for(int i=0; i < indices.length()-1 ; i++) {
        Direction peakDir = topPeakIndices.contains(indices[i]) ? Up : Down;
        Direction peakDirNext = topPeakIndices.contains(indices[i+1]) ? Up : Down;

        if(peakDir == Down && peakDirNext == Up){
            float diff = indices.at(i+1) - indices.at(i);
            if(diff > 10) return false;
        }

    }

    bool satisfyAmplitudeRestriction = amplitude > 0.1 ;

    return  satisfyAmplitudeRestriction;
}
