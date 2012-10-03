#include "epsegment.h"

#include <QDebug>

#include "apsegmenttypescontroller.h"
#include "aputils.h"
#include "epsignalscontroller.h"

EPSegment *EPSegment::_temporalSegment = NULL;

EPSegment::EPSegment(int type, unsigned int start, unsigned int end, int id, QObject *parent) :
	QObject(parent),
	_start(start),
	_end(end)
{
	_id = id == APNotFound ? qrand() % 1000000 : id;
	_type = APSegmentTypesController::typeForId(type);
	_comments = QString();
	resetTranscientValues();

	_lastState.first = start;
	_lastState.second = end;
}

QString EPSegment::comments()
{
	return _comments;
}

QString EPSegment::description(bool verbose)
{
	return verbose ? tr("%1:%2-%3").arg(type()->name())
					 .arg(transformNumberOfPointsToSeconds(start())).arg(transformNumberOfPointsToSeconds(end()))
				   : tr("%1 to %2").arg(transformNumberOfPointsToSeconds( haveTranscientValues() ? transcientStart() : start() ))
					 .arg(transformNumberOfPointsToSeconds( haveTranscientValues() ? transcientEnd() : end() ));
}

int EPSegment::duration()
{
	return length() / 100;
}

unsigned int EPSegment::end()
{
	return _end;
}

QList<EPSegment *> EPSegment::findCollisions()
{
	EPSProfile *profile = EPSignalsController::activeSignal()->profile();
	bool alreadyAdded = profile->indexOfObject(this) != -1;
	return alreadyAdded ? profile->findCollisionsForSegment(this) : profile->findCollisionsForProposedSegment(this);
}

bool EPSegment::haveTranscientValues()
{
	return (int)_transcientStart != APNotFound || (int)_transcientEnd != APNotFound;
}

int EPSegment::id()
{
	return _id;
}

bool EPSegment::isCommented()
{
	return !(comments().isEmpty());
}

unsigned int EPSegment::length()
{
	int start = haveTranscientValues() ? transcientStart() : _start;
	int end = haveTranscientValues() ? transcientEnd() : _end;

	return end - start + 1;
}

QString EPSegment::lastChangeInfo()
{
	float value = APNotFound;
	if (_lastState.first == "start") value = _start;
	else if (_lastState.first == "end") value = _end;
	else if (_lastState.first == "type") value = _type->id();

	return tr("%1:%2=%3-%4").arg(id())
			.arg(_lastState.first).arg(_lastState.second)
			.arg(value);
}

void EPSegment::pushChanges()
{
	if (transcientStart() != start()) setStart(transcientStart());
	else if (transcientEnd() != end()) setEnd(transcientEnd());
	resetTranscientValues();
}

void EPSegment::pushSerializedChange(QString change, Direction direction)
{
	// format property=prev-next
	QStringList propertyAndValues = change.split("=");
	QStringList values = propertyAndValues.last().split("-");

	QString property = propertyAndValues.first();
	QVariant prev = values.first();
	QVariant next = values.last();

	QVariant value = direction == Forward ? next : prev;
	qDebug().nospace() << "EPSegment::pushSerializedChange(" << change
					   << ", " << (direction == Forward ? "Redo" : "Undo")
					   << ")";

	if (property == "start")
		setStart(value.toUInt(), true);
	else if (property == "end")
		setEnd(value.toUInt(), true);
	else if (property == "type") {
		int oldType = type()->id();
		setType(value.toInt(), true);
		emit typeRestored(oldType);
	} else if (property == "comments")
		setComments(value.toString());
}

void EPSegment::resetTranscientValues()
{
	_transcientStart = APNotFound;
	_transcientEnd = APNotFound;
}

bool EPSegment::satisfyDurationRestriction()
{
	return duration() > APUtils::durationForSegmentType(type()->id());
}

QString EPSegment::serialize()
{
	return tr("%1:%2:%3:%4:%5").arg(id())
			.arg(type()->id()).arg(start()).arg(end())
			.arg(comments());
}

EPSegment *EPSegment::setComments(QString someComments)
{
	_comments = someComments;
	emit commentsDidChanged();
	return this;
}

void EPSegment::setEnd(unsigned int anEnd, bool silent)
{
	_lastState.first = "end"; _lastState.second = _end;
	_end = anEnd;

	if (silent) return;

	emit endsDidChanged();
	emit endsDidChanged(start(), end());
}

void EPSegment::setStart(unsigned int aStart, bool silent)
{
	_lastState.first = "start"; _lastState.second = _start;
	_start = aStart;

	if (silent) return;

	emit endsDidChanged();
	emit endsDidChanged(start(), end());
}

void EPSegment::setTranscientEnd(uint end)
{
	_transcientEnd = end;
	_transcientStart = _start;
	emit transcientEndsDidChanged();
}

void EPSegment::setTranscientRange(uint start, uint end)
{
	_transcientEnd = end;
	_transcientStart = start;
	emit transcientEndsDidChanged();
}

void EPSegment::setTranscientStart(uint start)
{
	_transcientStart = start;
	_transcientEnd = _end;
	emit transcientEndsDidChanged();
}

void EPSegment::setType(APSegmentType *type, bool silent)
{
	if (_type == type) return;

	_lastState.first = "type"; _lastState.second = _type->id();

//	if (!silent) emit typeDidChanged(_type->id()); // old type
	_type = type;
	if (!silent) {
		emit typeDidChanged(_lastState.second);
		emit typeDidChanged();
	}
}

void EPSegment::setType(int type, bool silent)
{
	setType(APSegmentTypesController::typeForId(type), silent);
}

unsigned int EPSegment::start()
{
	return _start;
}

uint EPSegment::transcientEnd()
{
	return _transcientEnd;
}

uint EPSegment::transcientStart()
{
	return _transcientStart;
}

APSegmentType *EPSegment::type()
{
	return _type;
}

EPSegment *EPSegment::unserialize(QString info)
{
	// format id:type:start:end:comments
	QStringList tokens = info.split(":");

	int id = tokens.at(0).toInt();
	int type = tokens.at(1).toInt();
	uint start = (uint)tokens.at(2).toInt();
	uint end = (uint)tokens.at(3).toInt();
	QString comments = tokens.at(4);

	EPSegment *segment = new EPSegment(type, start, end, id);
	return segment->setComments(comments);
}
