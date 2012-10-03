#ifndef EPSPROFILE_H
#define EPSPROFILE_H

#include "apcontroller.h"
#include "epsegment.h"
#include "apedithandler.h"

class APEditHandler;

class EPSProfile : public APController
{
	Q_OBJECT
public:
	explicit EPSProfile(QObject *parent = 0);

	void addSegment(EPSegment *segment, bool silent = false);
	void addSegment(int type, uint start, uint end, QString comments = QString(), bool silent = false);
	QList<int> currentTypes();
	bool isEmpty();
	unsigned int numberOfSegmentsOfType(int type);
	void removeSegment(EPSegment *segment, bool silent = false);
	void removeSegmentWithId(int id, bool silent = false);
	void removeAllSegmentsOfType(int type);
	EPSegment *segmentAtOfType(uint hotspot, int type = All);
	QList<EPSegment *> segmentsInRangeOfType(uint start, uint end, int type = All);
	QList<EPSegment *> segmentsOfType(int type);
	EPSegment *segmentWithId(int id);

	QList<EPSegment *> findCollisionsForSegment(EPSegment *segment);
	QList<EPSegment *> findCollisionsForProposedSegment(EPSegment *segment);
	void fixCollisionForSegment(EPSegment *segment);
	void fixInnerCollisions();
	void fillGaps();

	void mergeSameSegments();

	int columnCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	void forceEmitOfSegmentsDidChange();

	APEditHandler *editHandler;

	int countOfEmptySpaces();

	int totalDurationOfSegmentType(int type);

signals:
	void segmentsDidChange();
	void selectedSegmentDidChange(EPSegment *);

public slots:
	void setSelectedSegment(EPSegment *segment);
	void segmentDidChangeItsType(int oldType);
	void restoredSegmentType(int oldType);
	void segmentDidChangeItsEnds();
	void segmentWasActivated(QObject *object);
	void remove();

private slots:
	void updateSelectedSegment(QObject *segment);

private:
	QMap<int, QList<EPSegment *> *> _segmentsGroupedByType;

	void insertSegmentInList(EPSegment *segment, QList<EPSegment *> *list);

	bool fixing;
	bool segmentRecentlyAdded;

	static bool segmentLessThan(QObject *left, QObject *right);
};

#endif // EPSPROFILE_H
