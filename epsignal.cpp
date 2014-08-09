#include "epsignal.h"

#include <cmath>

EPSignal::EPSignal(QString filepath, int id, QObject *parent) :
    QObject(parent),
    _id(id)
{
    APFileInfo fileInfo(filepath);
    _fileInfos << fileInfo;
    _fileType = deriveFileType(fileInfo);
    _profile = new EPSProfile(this);
    _changed = false;

    connect(_profile, SIGNAL(segmentsDidChange()), SLOT(profileDidChanged()));

    _minimum = APInfinite;
    _maximum = -APInfinite;
}

EPSignal::EPSignal(EPSignal &signal) : QObject(0)
{
    _id = signal.id();
    _name = QString(signal.name());
    _comments = QString(signal.comments());

    foreach (APFileInfo fileInfo, signal.fileInfos()) {
        appendFilePath(fileInfo.filePath());
    }
    _fileType = deriveFileType(fileInfo());

    foreach (float point, signal.points()) {
        pushPoint(point);
    }

    _profile = new EPSProfile(this);
    foreach(EPSegment *segment, signal.profile()->segmentsOfType(All)) {
        profile()->addSegment(segment->type()->id(),
                              segment->start(),
                              segment->end(),
                              QString(segment->comments()),
                              true);
    }

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

EPSignal::FileType EPSignal::deriveFileType(APFileInfo fileInfo) {
    if (fileInfo.suffix() == "epg") return EPSignal::EPG;
    if (fileInfo.suffix() == "dat") return EPSignal::Dat;

    static QRegExp rx("((.+)?\\.)?D0\\d");
    if (rx.exactMatch(fileInfo.fileName())) return EPSignal::Acquisition;

    return EPSignal::Unknown;
}

APFileInfo EPSignal::fileInfo()
{
    return _fileInfos.first();
}

QList<APFileInfo> EPSignal::fileInfos()
{
    return _fileInfos;
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
        _name = _fileInfos.first().baseName();
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

void EPSignal::appendComment(QString aComment)
{
    if (!_comments.isEmpty())
        _comments += tr("\n");
    _comments += aComment;
    emit commentsDidchange(comments());
}

void EPSignal::setChanged(bool changed)
{
    _changed = changed;
    emit signalDidChanged();
    emit signalDidChanged(changed);
}

void EPSignal::setFileInfo(APFileInfo fileInfo)
{
    _fileInfos.clear();
    _fileInfos << fileInfo;
    emit fileInfoDidChange();
}

void EPSignal::appendFilePath(QString filePath)
{
    APFileInfo fileInfo(filePath);
    if (!fileInfos().contains(fileInfo)) {
        _fileInfos << fileInfo;
        emit fileInfoDidChange();
    }
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
