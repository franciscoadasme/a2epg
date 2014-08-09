#ifndef APE1SEEKER_H
#define APE1SEEKER_H

#include "apsequentialseeker.h"

class APE1Seeker : public APSequentialSeeker
{
    Q_OBJECT
public:
    explicit APE1Seeker(QObject *object, QObject *parent = 0);

private:
    SegmentType type();
    void loadDefaults();
    bool isAllowedPoint();
};

#endif // APE1SEEKER_H
