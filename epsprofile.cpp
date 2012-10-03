#include "epsprofile.h"

#include <QDebug>

#include "apsegmenttypescontroller.h"
#include "apsegmentdialog.h"
#include "aputils.h"
#include "mainwindow.h"
#include "epsignalscontroller.h"

#define TypeColumn 0
#define StartColumn 1
#define EndColumn 2
#define LengthColumn 3

EPSProfile::EPSProfile(QObject *parent) :
	APController(parent)
{
//	objects = QList<EPSegment *>();
	_segmentsGroupedByType = QMap<int, QList<EPSegment *> *>();
	fixing = false;

	foreach (int type, APSegmentTypesController::typeIds()) {
		_segmentsGroupedByType.insert(type, new QList<EPSegment *>());
	}

	connect(this, SIGNAL(selectionDidChanged(QObject*)),
			SLOT(updateSelectedSegment(QObject*)));
	connect(this, SIGNAL(activeObjectDidChanged(QObject*)),
			SLOT(segmentWasActivated(QObject*)));

	editHandler = new APEditHandler(this);
	segmentRecentlyAdded = false;
}

void EPSProfile::addSegment(EPSegment *segment, bool silent)
{
	beginResetModel(); {
		insertSegmentInList(segment, (QList<EPSegment *> *)&objects);
	} endResetModel();

	insertSegmentInList(segment, _segmentsGroupedByType.value(segment->type()->id()));
	connect(segment, SIGNAL(typeDidChanged(int)), SLOT(segmentDidChangeItsType(int)));
	connect(segment, SIGNAL(endsDidChanged()), SLOT(segmentDidChangeItsEnds()));
	connect(segment, SIGNAL(typeRestored(int)), SLOT(restoredSegmentType(int)));

	if (!silent) {
		segmentRecentlyAdded = true;
		fixCollisionForSegment(segment);
		emit segmentsDidChange();
	}
}

void EPSProfile::addSegment(int type, uint start, uint end, QString comments, bool silent)
{
	EPSegment *segment = (new EPSegment(type, start, end))->setComments(comments);
	addSegment(segment, silent);
}

int EPSProfile::columnCount(const QModelIndex &) const
{
	return 4;
}

int EPSProfile::countOfEmptySpaces()
{
	int emptySpaces = 0;
	QList<EPSegment *> segments = segmentsOfType(All);
	for (int i = 0; i < segments.count() - 1; i++) {
		// allow 5 points of error for manual edition
		if (segments.at(i + 1)->start() - segments.at(i)->end() > 5)
			emptySpaces++;
	}

	if (segments.first()->start() > 5)
		emptySpaces++;

	if (EPSignalsController::activeSignal()->numberOfPoints() - segments.last()->end() > 5)
		emptySpaces++;

	return emptySpaces;
}

QList<int> EPSProfile::currentTypes()
{
	QList<int> types = QList<int>();
	foreach (int type, _segmentsGroupedByType.keys()) {
		if (_segmentsGroupedByType[type]->length() > 0)
			types.append(type);
	}
	qSort(types);
	return types;
}

QVariant EPSProfile::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= count())
		return QVariant();

	EPSegment *segment = (EPSegment *)objectAt(index.row());

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case TypeColumn:
			return segment->type()->name();
		case StartColumn:
			return transformNumberOfPointsToSeconds(segment->start());
		case EndColumn:
			return transformNumberOfPointsToSeconds(segment->end());
		case LengthColumn:
			return tr("%1s").arg(transformNumberOfPointsToSeconds(segment->length()));
		}
	} else if (role == Qt::DecorationRole) {
		if (index.column() == TypeColumn)
			return segment->type()->color();
	}

	return QVariant();
}

void EPSProfile::fillGaps()
{
	for (int i = 0; i < count() - 2; i++) {
		EPSegment *left = (EPSegment *)objectAt(i);
		EPSegment *right = (EPSegment *)objectAt(i + 1);

		if (left->type() == right->type()) {
			left->setEnd(right->end(), true);
			removeSegment(right);
			i--;
		}
	}

	EPSegment *first = (EPSegment *)objectAt(0);
	if (first->start() > 0)
		first->setStart(0, true);

	EPSegment *last = (EPSegment *)objects.last();
	if (last->end() < EPSignalsController::activeSignal()->numberOfPoints() - 1)
		last->setEnd(EPSignalsController::activeSignal()->numberOfPoints() - 1, true);
}

QList<EPSegment *> EPSProfile::findCollisionsForSegment(EPSegment *segment)
{
	int segmentIndex = objects.indexOf(segment);
	QList<EPSegment *> collidedSegments;

	uint start = segment->haveTranscientValues() ? segment->transcientStart() : segment->start();
	uint end = segment->haveTranscientValues() ? segment->transcientEnd() : segment->end();

	/* look for left collisions */ {
		for (int i = segmentIndex - 1; i >= 0; i--) {
			EPSegment *seg = (EPSegment *)objectAt(i);
			bool overlap = seg->end() >= start && seg->start() <= start;
			bool contains = seg->start() >= start && seg->end() <= end;
			if (overlap || contains)
				collidedSegments << seg;
			else
				break;
		}
	}

	/* look for right collisions */ {
		for (int i = segmentIndex + 1; i < objects.length(); i++) {
			EPSegment *seg = (EPSegment *)objectAt(i);
			bool overlap = seg->start() <= end && seg->end() >= end;
			bool contains = seg->end() <= end && seg->start() >= start;
			if (overlap || contains)
				collidedSegments << seg;
			else
				break;
		}
	}

	return collidedSegments;
}

QList<EPSegment *> EPSProfile::findCollisionsForProposedSegment(EPSegment *segment)
{
	QList<EPSegment *> collidedSegments;

	uint start = segment->haveTranscientValues() ? segment->transcientStart() : segment->start();
	uint end = segment->haveTranscientValues() ? segment->transcientEnd() : segment->end();

	foreach (EPSegment *seg, segmentsOfType(All)) {
		bool overlap = (seg->start() <= start && seg->end() >= start)
				|| (seg->start() <= end && seg->end() >= end);
		bool contains = seg->start() >= start && seg->end() <= end;
		if (overlap || contains)
			collidedSegments << seg;
	}

	return collidedSegments;
}

void EPSProfile::fixInnerCollisions()
{
//	qDebug() << "EPSProfile::fixInnerCollisions()";

	fixing = true;

	for (int i = 0; i < count() - 1; i++) {
		EPSegment *segment = (EPSegment *)objectAt(i);

		/*
			we need to iterate through all segments again, since there are long segments that
			still could be overlapping with the current one
		*/
		for (int j = 0; j < count(); j++) {
			if (i == j) continue;

			EPSegment *otherSegment = (EPSegment *)objectAt(j);
			if (otherSegment->start() > segment->end()) break;
			else if (segment->start() > otherSegment->end()) continue;

			bool contains = otherSegment->end() < segment->end();

			int mostReliableType = APUtils::mostReliableType(segment->type()->id(), otherSegment->type()->id());
			bool isCurrentSegmentMoreReliable = mostReliableType == segment->type()->id();

			if (isCurrentSegmentMoreReliable) {

				if (contains) {
					removeSegment(otherSegment);
					i--; j--;
					if (i < 0) i = 0; // case when a colision is found at the beginning
				} else {
					otherSegment->setStart(segment->end() + 1, true);

					// add otherSegment duration validation
					if (!otherSegment->satisfyDurationRestriction()) {
						// if otherSegment is shorter than required duration, remove it
						EPSegment *next = (EPSegment *)objectAt(j + 1);
						if (next) {
							next->setStart(otherSegment->start(), true);
						}
						removeSegment(otherSegment);
					}
				}

			} else {

				if (contains) {
					EPSegment *rightestSegment = new EPSegment(segment->type()->id(), otherSegment->end() + 1, segment->end());
					segment->setEnd(otherSegment->start() - 1, true);
					addSegment(rightestSegment, true);
					// add rightestSegment and segment duration validation

					if (!segment->satisfyDurationRestriction()) {
						// if segment is shorter than required duration, remove it
						EPSegment *prev = (EPSegment *)objectAt(i - 1);
						if (prev) {
							prev->setEnd(segment->end(), true);
						}
						removeSegment(segment);
					}

					if (!rightestSegment->satisfyDurationRestriction()) {
						// if rightestSegment (new) is shorter than required duration, remove it
						EPSegment *next = (EPSegment *)objectAt(indexOfObject(rightestSegment) + 1);
						if (next) {
							next->setStart(rightestSegment->start(), true);
						}
						removeSegment(rightestSegment);
					}
				} else {
					segment->setEnd(otherSegment->start() - 1, true);
					// add segment duration validation

					if (!segment->satisfyDurationRestriction()) {
						// if segment is shorter than required duration, remove it
						EPSegment *prev = (EPSegment *)objectAt(i - 1);
						if (prev) {
							prev->setEnd(segment->end(), true);
						}
						removeSegment(segment);
					}
				}

			}
		}
	}

	qSort(objects.begin(), objects.end(), EPSProfile::segmentLessThan);
	mergeSameSegments();

	fixing = false;

//	qDebug() << "EPSProfile::fixInnerCollisions() end";
}

void EPSProfile::fixCollisionForSegment(EPSegment *segment)
{
	QList<EPSegment *> collidedSegments = findCollisionsForSegment(segment);
	if (collidedSegments.isEmpty()) {
		editHandler->saveAction(segmentRecentlyAdded ? APEditHandler::Create : APEditHandler::Update,
								segmentRecentlyAdded ? segment->serialize() : segment->lastChangeInfo());

		// FIX to force overlay to update
		MainWindow::instance()->epsignalWidget()->overlay()->forceToUpdateGeometry();
		return;
	}

	beginResetModel();
	editHandler->beginGroup();

	foreach (EPSegment *collidedSegment, collidedSegments) {
		bool contains = collidedSegment->start() >= segment->start()
				&& collidedSegment->end() <= segment->end();
		bool contained = segment->start() >= collidedSegment->start()
				&& segment->end() <= collidedSegment->end();

		if (contains) {
			removeSegment(collidedSegment, true);
			editHandler->saveAction(APEditHandler::Remove, collidedSegment->serialize());
		} else if (contained) {

			if (collidedSegment->type() == segment->type()) { // if they are equals, just remove new segment...
				segment->setStart(collidedSegment->start(), true);
				segment->setEnd(collidedSegment->end(), true);

				removeSegment(collidedSegment, true);
				editHandler->saveAction(APEditHandler::Remove, collidedSegment->serialize());
			} else {
				EPSegment *rightPart = new EPSegment(collidedSegment->type()->id(),
													 segment->end() + 1, collidedSegment->end());
				addSegment(rightPart, true);
				editHandler->saveAction(APEditHandler::Create, rightPart->serialize());

				collidedSegment->setEnd(segment->start() - 1, true);
				editHandler->saveAction(APEditHandler::Update, collidedSegment->lastChangeInfo());
			}

		} else if (collidedSegment->start() <= segment->end()
				   && collidedSegment->end() >= segment->end()) { // right overlap

			if (collidedSegment->type() == segment->type()) {
				segment->setEnd(collidedSegment->end(), true);
				if (!segmentRecentlyAdded) // if it was just added, wait for collisions fixes
					editHandler->saveAction(APEditHandler::Update, segment->lastChangeInfo());

				removeSegment(collidedSegment, true);
				editHandler->saveAction(APEditHandler::Remove, collidedSegment->serialize());
			} else {
				collidedSegment->setStart(segment->end() + 1, true);
				editHandler->saveAction(APEditHandler::Update, collidedSegment->lastChangeInfo());
			}
		} else if (collidedSegment->end() >= segment->start()
				   && collidedSegment->start() <= segment->start()) { // left overlap

			if (collidedSegment->type() == segment->type()) {
				segment->setStart(collidedSegment->start(), true);
				if (!segmentRecentlyAdded) // if it was just added, wait for collisions fixes
					editHandler->saveAction(APEditHandler::Update, segment->lastChangeInfo());

				removeSegment(collidedSegment, true);
				editHandler->saveAction(APEditHandler::Remove, collidedSegment->serialize());
			} else {
				collidedSegment->setEnd(segment->start() - 1, true);
				editHandler->saveAction(APEditHandler::Update, collidedSegment->lastChangeInfo());
			}
		}
	}

	editHandler->saveAction(segmentRecentlyAdded ? APEditHandler::Create : APEditHandler::Update,
							segmentRecentlyAdded ? segment->serialize() : segment->lastChangeInfo());
	segmentRecentlyAdded = false; // reset flag

	// FIX to force overlay to update
	MainWindow::instance()->epsignalWidget()->overlay()->forceToUpdateGeometry();

	editHandler->endGroup();
	endResetModel();

	emit segmentsDidChange();
}

void EPSProfile::forceEmitOfSegmentsDidChange()
{
	emit segmentsDidChange();
}

QVariant EPSProfile::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation == Qt::Vertical)
		return QVariant();

	switch (section) {
	case TypeColumn:
		return "Type";
	case StartColumn:
		return "Start";
	case EndColumn:
		return "End";
	case LengthColumn:
		return "Length";
	}

	return QVariant();
}

void EPSProfile::insertSegmentInList(EPSegment *segment, QList<EPSegment *> *list)
{
	unsigned int index = 0;
	foreach (EPSegment *seg, *list) {
		if (segment->start() < seg->start()) {
			list->insert(index, segment);
			return;
		}
		index++;
	}
	list->append(segment); // it should be added to the last position
}

bool EPSProfile::isEmpty()
{
	return count() == 0;
}

void EPSProfile::mergeSameSegments()
{
	for (int i = 0; i < count() - 1; i++) {
		EPSegment *segment = (EPSegment *)objectAt(i);
		EPSegment *other = (EPSegment *)objectAt(i + 1);

//		qDebug() << "EPSProfile::mergeSameSegments() =>" << segment->description(true) << other->description(true);

		if (other->start() - segment->end() < 200 && segment->type() == other->type()) {
			segment->setEnd(other->end(), true);
			removeSegment(other);
			i--;
		}
	}
}

unsigned int EPSProfile::numberOfSegmentsOfType(int type)
{
	return type == All ? count() : _segmentsGroupedByType.value(type)->length();
}

void EPSProfile::remove()
{
	EPSegment *segment = (EPSegment *)take();
	segment->disconnect(this);
	_segmentsGroupedByType.value(segment->type()->id())->removeOne(segment);

	editHandler->saveAction(APEditHandler::Remove, segment->serialize());

	emit segmentsDidChange();
}

void EPSProfile::removeSegment(EPSegment *segment, bool silent)
{
	segment->disconnect(this);
	beginRemoveRows(QModelIndex(), indexOfObject(segment), indexOfObject(segment)); {
		objects.removeOne(segment);
	} endRemoveRows();
	_segmentsGroupedByType.value(segment->type()->id())->removeOne(segment);

	if (!silent)
		emit segmentsDidChange();
}

void EPSProfile::removeSegmentWithId(int id, bool silent)
{
	removeSegment(segmentWithId(id), silent);
}

void EPSProfile::removeAllSegmentsOfType(int type)
{
	beginResetModel();

	Q_ASSERT_X(APSegmentTypesController::typeIds().contains(type), "EPSProfile::removeAllSegmentsOfType", "type not valid");

	foreach (EPSegment *segment, segmentsOfType(type)) {
		segment->disconnect(this);
		objects.removeOne(segment);
	}
	_segmentsGroupedByType.value(type)->clear();
	emit segmentsDidChange();

	endResetModel();
}

void EPSProfile::restoredSegmentType(int oldType)
{
	EPSegment *segment = (EPSegment *)sender();
	insertSegmentInList(segment, _segmentsGroupedByType.value(segment->type()->id()));
	_segmentsGroupedByType[(int)oldType]->removeOne(segment);

	emit segmentsDidChange();

	dataChanged(index(indexOfObject(segment), 0), index(indexOfObject(segment), 0));
}

EPSegment *EPSProfile::segmentAtOfType(uint hotspot, int type)
{
	foreach (EPSegment *segment, segmentsOfType(type)) {
		if (segment->start() <= hotspot && segment->end() >= hotspot) {
			return segment;
		}
	}
	return NULL;
}

void EPSProfile::segmentDidChangeItsEnds()
{
	if (fixing || !sender()) return; // avoid NULLs
	qDebug() << "EPSProfile::segmentDidChangeItsEnds() =>" << sender();
	fixCollisionForSegment((EPSegment *)sender());

	emit segmentsDidChange();
}

void EPSProfile::segmentDidChangeItsType(int oldType)
{
	EPSegment *segment = (EPSegment *)sender();
	insertSegmentInList(segment, _segmentsGroupedByType.value(segment->type()->id()));
	_segmentsGroupedByType[(int)oldType]->removeOne(segment);

	editHandler->beginGroup(); {

		int indexOfSegment = indexOfObject(segment);
		EPSegment *next = indexOfSegment < count() - 1 ? (EPSegment *)objectAt(indexOfSegment + 1) : NULL;
		EPSegment *prev = indexOfSegment > 0 ? (EPSegment *)objectAt(indexOfSegment - 1) : NULL;

		if (next && prev
				&& next->type() == segment->type()
				&& prev->type() == segment->type()) {
			qDebug() << "EPSProfile::segmentDidChangeItsType() => prev, segment and next have same type";

			prev->setEnd(next->end(), true);
			editHandler->saveAction(APEditHandler::Update, prev->lastChangeInfo());
			dataChanged(index(indexOfObject(prev), 0), index(indexOfObject(prev), 0));

			segment->setType(oldType, true);
			removeSegment(segment, true);
			editHandler->saveAction(APEditHandler::Remove, segment->serialize());

			removeSegment(next, true);
			editHandler->saveAction(APEditHandler::Remove, next->serialize());

			MainWindow::instance()->epsprofileWidget()->setFocusedSegment(prev);
		} else if (next && next->type() == segment->type()) {
			qDebug() << "EPSProfile::segmentDidChangeItsType() => next and segment have same type";

			next->setStart(segment->start(), true);
			editHandler->saveAction(APEditHandler::Update, next->lastChangeInfo());
			dataChanged(index(indexOfObject(next), 0), index(indexOfObject(next), 0));

			segment->setType(oldType, true);
			removeSegment(segment, true);
			editHandler->saveAction(APEditHandler::Remove, segment->serialize());

			MainWindow::instance()->epsprofileWidget()->setFocusedSegment(next);
		} else if (prev && prev->type() == segment->type()) {
			qDebug() << "EPSProfile::segmentDidChangeItsType() => prev and segment have same type";

			prev->setEnd(segment->end(), true);
			editHandler->saveAction(APEditHandler::Update, prev->lastChangeInfo());
			dataChanged(index(indexOfObject(prev), 0), index(indexOfObject(prev), 0));

			segment->setType(oldType, true);
			removeSegment(segment, true);
			editHandler->saveAction(APEditHandler::Remove, segment->serialize());

			MainWindow::instance()->epsprofileWidget()->setFocusedSegment(prev);
		} else {
			editHandler->saveAction(APEditHandler::Update, segment->lastChangeInfo());
			dataChanged(index(indexOfObject(segment), 0), index(indexOfObject(segment), 0));
		}

	} editHandler->endGroup();

	emit segmentsDidChange();
}

QList<EPSegment *> EPSProfile::segmentsInRangeOfType(uint start, uint end, int typeId)
{
	QList<EPSegment *> segments = QList<EPSegment *>();
	foreach (EPSegment *segment, segmentsOfType(typeId)) {
		uint segmentStart = segment->haveTranscientValues() ? segment->transcientStart() : segment->start();
		uint segmentEnd = segment->haveTranscientValues() ? segment->transcientEnd() : segment->end();

		bool isStartInRange = segmentStart >= start && segmentStart < end;
		bool isEndInRange = segmentEnd <= end && segmentEnd > start;
		bool containsRange = segmentStart <= start && segmentEnd >= end;

		if (isStartInRange || isEndInRange || containsRange) {
			if (typeId == All || segment->type()->id() == typeId)
				segments.append(segment);
		}
	}
	return segments;
}

bool EPSProfile::segmentLessThan(QObject *left, QObject *right)
{
	return ((EPSegment *)left)->start() < ((EPSegment *)right)->start();
}

QList<EPSegment *> EPSProfile::segmentsOfType(int type)
{
	QList<EPSegment *> *segments = (QList<EPSegment *> *)&objects;
	return type == All ? *segments : *(_segmentsGroupedByType.value(type));
}

void EPSProfile::segmentWasActivated(QObject *object)
{
	qDebug() << "EPSProfile::segmentWasActivated()";
	APSegmentDialog::runForSegment((EPSegment *)object);
}

EPSegment *EPSProfile::segmentWithId(int id)
{
	foreach (EPSegment *segment, segmentsOfType(All)) {
		if (segment->id() == id)
			return segment;
	}
	return NULL;
}

void EPSProfile::setSelectedSegment(EPSegment *segment)
{
	setSelectedObject(segment);
}

int EPSProfile::totalDurationOfSegmentType(int type)
{
	int duration = 0;
	foreach (EPSegment *segment, *(_segmentsGroupedByType.value(type))) {
		duration += segment->length();
	}
	return transformNumberOfPointsToSeconds(duration);
}

void EPSProfile::updateSelectedSegment(QObject *segment)
{
	emit selectedSegmentDidChange((EPSegment *)segment);
}
