#ifndef APSEEKER_H
#define APSEEKER_H

#include <QtCore>

#include "apworker.h"
#include "epsprofile.h"
#include "APGlobals.h"
#include "epsignal.h"

class APSeeker : public APWorker
{
	Q_OBJECT
public:
	explicit APSeeker(QObject *object, QObject *parent = 0);

	QMap<QString, float> params();
	virtual SegmentType type(); // should be overridden

	static APSeeker *seekerForTypeAndSignal(SegmentType type, EPSignal *signal, QObject *parent = 0);
	void dispatchProposedSegments();
	int numberOfProposedSegments();

public slots:
	void stop();

protected:
	float param(QString key);
	APSeeker *setParam(QString key, float value);
	virtual void loadDefaults(); // should be overidden
	void setUp();

	QList<float> points;

	QList<float> *peakIndices(Direction direction);
	QList<float> peakIndicesInRange(Direction direction, int left, int right);

	int proposedStart();
	void setProposedStart(int proposedStart);
	void unsetProposedStart();
	bool didFindAnythingYet();
	bool proposedRangeSatisfyMinimumDuration(int proposedEnd);
	void pushSegment(uint start, uint end);

	void cleanUpProposedSegments();

	void writeLog(QString str);

	virtual void seek() { return; }
	bool _shouldStop; // flag to stop execution

	QList<QPair<uint, uint> *> _proposedSegments;

private:
	QMap<QString, float> _parameters;
	EPSProfile *profile;

	QList<float> topPeakIndices, bottomPeakIndices;
	int lastPeakIndiceRetrieved;

	int _proposedStart;

	QFile *_logFile;

	void run();
};

#endif // APSEEKER_H
