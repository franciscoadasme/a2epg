#ifndef APNPSEEKER_H
#define APNPSEEKER_H

#include "apsequentialseeker.h"

class APNpSeeker : public APSeeker
{
    Q_OBJECT
public:
	explicit APNpSeeker(QObject *object, QObject *parent = 0);

private:
	SegmentType type();
	void loadDefaults();
	void seek();

	void findLowSlopeDeviations(QList<float> indices);
};

#endif // APNPSEEKER_H
