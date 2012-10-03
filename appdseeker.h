#ifndef APPDSEEKER_H
#define APPDSEEKER_H

#include "apseeker.h"

class APPdSeeker : public APSeeker
{
    Q_OBJECT
public:
	explicit APPdSeeker(QObject *object, QObject *parent = 0);

private:
	SegmentType type();
	void loadDefaults();
	void seek();

	QList<QPair<int, Direction> > findSlopeChanges(QList<float> peaks);
	QList<QPair<int, int> > findSunkenSegments(QList<QPair<int, Direction> > indices);
	QList<QPair<int, int> > filterSunkenSegments(QList<QPair<int, int> > proposedSunkenSegments);

	bool areInnerPointsLowerThanEnds(int left, int right, QList<float> topPeaks);
	bool satisfyInnerPeakHeightRestrictions(int left, int right, QList<float> topPeaks);
	bool satisfyInnerDeviation(QList<float> topPeaks);
	bool satisfyInnerRestrictions(int left, int right, QList<float> topPeaks);
	QList<QPair<int, int> > chooseLongestSegments(QList<QPair<int, int> > foundSegments);

	typedef struct {
		int left, right;
		Direction direction;
		float height;
	} Slope;
	inline Slope slopeWithLeftRightHeightAndDirection(int left, int right, float height, Direction direction) const
	{
		Slope slope;
		slope.left = left;
		slope.right = right;
		slope.height = height;
		slope.direction = direction;
		return slope;
	}

	bool areSurroundingsHigher(Slope start, Slope end);
};

#endif // APPDSEEKER_H
