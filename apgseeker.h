#ifndef APGSEEKER_H
#define APGSEEKER_H

#include "apsequentialseeker.h"

class APGSeeker : public APSequentialSeeker
{
    Q_OBJECT
public:
	explicit APGSeeker(QObject *object, QObject *parent = 0);

protected:
	SegmentType type();
	void loadDefaults();
	bool isAllowedPoint(); // needed for actual work

	float deviationOfDistanceBetweenPeaks(QList<float> indices);
};

#endif // APGSEEKER_H

