#ifndef APNEWPDSEEKER_H
#define APNEWPDSEEKER_H

#include "apseeker.h"

class APNewPdSeeker : public APSeeker
{
    Q_OBJECT
public:
	explicit APNewPdSeeker(QObject *object, QObject *parent = 0);

protected:
	SegmentType type();
	void loadDefaults();
	void seek();

	QList<float> sortedPeakIndices();
	QList<Slope> findSteepestSlopes(QList<float> indices);
	void findSunkenSegments(QList<Slope> slopes);
	bool satisfyStrongVariantRestrictions(Slope start, Slope end, QList<float> topPeaks, QList<float> bottomPeaks);
    bool satisfyWeakVariantRestrictions(Slope start, Slope end, QList<float> topPeaks, QList<float> bottomPeaks);

    bool areMostInnerPeaksBeneathRoof(Slope start, Slope end, QList<float> peaks,
                                      float minimumRelativeHeight, float minimumRelativeCount);
	void chooseShortestSegments();
};

#endif // APNEWPDSEEKER_H
