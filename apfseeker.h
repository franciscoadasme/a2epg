#ifndef APFSEEKER_H
#define APFSEEKER_H

#include "apsequentialseeker.h"

class APFSeeker : public APSequentialSeeker
{
    Q_OBJECT
public:
    explicit APFSeeker(QObject *object, QObject *parent = 0);

private:
    SegmentType type();
    bool isAllowedPoint();
    void loadDefaults();
};

#endif // APFSEEKER_H
