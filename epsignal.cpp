#include "epsignal.h"

#include <cmath>

EPSignal::EPSignal(QString filepath, int id, QObject *parent) :
	QObject(parent),
	_id(id)
{
	_fileInfo = QFileInfo(filepath);
    _fileType = deriveFileType(_fileInfo);
	_profile = new EPSProfile(this);
	_changed = false;

	connect(_profile, SIGNAL(segmentsDidChange()), SLOT(profileDidChanged()));

	_minimum = APInfinite;
	_maximum = -APInfinite;
}

float EPSignal::amplitude()
{
	return fabs(maximum() - minimum());
}

QString EPSignal::comments()
{
	return _comments;
}

QString EPSignal::datname()
{
	return _datname;
}

QFileInfo EPSignal::fileInfo()
{
	return _fileInfo;
}

EPSignal::FileType EPSignal::fileType()
{
    return _fileType;
}

bool EPSignal::hasChanged()
{
	return _changed;
}

bool EPSignal::hasMorePointsThan(uint number)
{
	return numberOfPoints() > number;
}

int EPSignal::id()
{
	return _id;
}

bool EPSignal::isCommented()
{
	return !comments().isEmpty();
}

uint EPSignal::length()
{
	return points().length() * APSampleRate; // in seconds
}

float EPSignal::maximum()
{
	return _maximum;
}

float EPSignal::minimum()
{
	return _minimum;
}

QString EPSignal::name()
{
    if (_name == NULL) {
        _name = _fileInfo.baseName();
    }
	return _name;
}

uint EPSignal::numberOfPoints()
{
	return points().isEmpty() ? APEmpty : points().length();
}

QList<float> EPSignal::points()
{
	return _points;
}

EPSProfile *EPSignal::profile()
{
	return _profile;
}

void EPSignal::profileDidChanged()
{
	setChanged(true);
}

void EPSignal::pushPoint(float point)
{
	_points.append(point);

    if (point > _maximum)
        _maximum = point;
    else if (point < _minimum)
        _minimum = point;
}

void EPSignal::setComments(QString aComment)
{
	_comments = aComment;
	emit commentsDidchange(comments());
}

void EPSignal::setChanged(bool changed)
{
	_changed = changed;
	emit signalDidChanged();
	emit signalDidChanged(changed);
}

void EPSignal::setDatname(QString datname)
{
	_datname = datname;
}

void EPSignal::setFileInfo(QFileInfo fileInfo)
{
	_fileInfo = fileInfo;
	emit fileInfoDidChange();
}

void EPSignal::setMaximum(float maximum)
{
	_maximum = maximum;
}

void EPSignal::setMinimum(float minimum)
{
	_minimum = minimum;
}

void EPSignal::setName(QString aName)
{
	_name = aName;
	emit nameDidChanged();
	emit nameDidChanged(name());
}
