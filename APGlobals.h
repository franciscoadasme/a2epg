#ifndef APGLOBALS_H
#define APGLOBALS_H

#define APNotFound -1
#define APEmpty 0

#define APSampleRate 0.01

#define APInfinite 999999999

enum SegmentType {
    Np,
    C,
    Pd,
    Pd1,
    Pd2,
    Pd3,
    E1,
    E2,
    F,
    G,
    All
};

enum Direction {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};


typedef struct {
    int left, right;
    Direction direction;
    float height;
} Slope;

inline Slope slopeWithLeftRightHeightAndDirection(int left,
                                                  int right,
                                                  float height,
                                                  Direction direction)
{
    Slope slope;
    slope.left = left;
    slope.right = right;
    slope.height = height;
    slope.direction = direction;
    return slope;
}

// debug
#define DebugReading 0

inline static int numberOfPointsInOneHour()
{
    return 3600 / APSampleRate;
}
inline static float transformSecondsToNumberOfPoints(float seconds)
{
    return seconds / APSampleRate;
}
inline static float transformNumberOfPointsToSeconds(float numberOfPoints)
{
    return numberOfPoints * APSampleRate;
}

// settings keys
#define OpenPanoramicOnSignalChangeKey "OpenPanoramicOnSignalChange"
#define LastLocationVisitedKey "LastLocationVisited"
#define AskWhenDeletingSegmentsKey "AskWhenDeletingSegments"
#define LiveSegmentResizingKey "LiveSegmentResizing"
#define OpenInfoOnSignalChangeKey "OpenInfoOnSignalChange"
#define ZoomHorizontalKey "ZoomHorizontal"
#define ZoomVerticalKey "ZoomVertical"
#define PdMaximumDurationKey "PdMaximumDuration"
#define FillGapsDuringScanKey "FillGapsDuringScan"

#endif // APGLOBALS_H
