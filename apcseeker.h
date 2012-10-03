#ifndef APCSEEKER_H
#define APCSEEKER_H

#include "apsequentialseeker.h"

class APCSeeker : public APSequentialSeeker
{
    Q_OBJECT
public:
	explicit APCSeeker(QObject *object, QObject *parent = 0);

protected:
	SegmentType type();
	void loadDefaults();
	bool isAllowedPoint(); // needed for actual work

};

#endif // APCSEEKER_H
