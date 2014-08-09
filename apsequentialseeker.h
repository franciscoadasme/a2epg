#ifndef APSEQUENTIALSEEKER_H
#define APSEQUENTIALSEEKER_H

#include "apseeker.h"

class APSequentialSeeker : public APSeeker
{
    Q_OBJECT
public:
    explicit APSequentialSeeker(QObject *object, QObject *parent = 0);

protected:
    void setUp();
    void loadDefaults();
    void pushSegment(uint start, uint end);

    // window-related methods
    int left();
    int right();
    int middle();
    bool walkable();
    void setUpWindow();
    void moveForth();
    void moveBack();

    QList<float> pointsInsideWindow;
    QList<float> peaksInsideWindow(Direction direction,
                                   bool returnValues = true);
    float getFrecuency(Direction direction = Up);
    float getAmplitude();

    virtual bool isAllowedPoint()
    {
        Q_ASSERT_X(false, "APSeeker", "Did not call overridden function");
        return false;
    }

private:
    int _windowLeft;
    int _windowRight;
    bool _moveable; // determinate if window can move

    int _proposedStart;

    QList<float> topPeakIndicesInsideWindow, bottomPeakIndicesInsideWindow;
    int topPeakIndicesOffset, bottomPeakIndicesOffset;

    void move(Direction direction = Forward);
    void loadPointsInsideWindow();

    void seek();
};

#endif // APSEQUENTIALSEEKER_H
