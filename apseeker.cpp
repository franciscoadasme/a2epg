#include "apseeker.h"

#include <QtCore>
#include <cmath>

#include "apnpseeker.h"
#include "appdseeker.h"
#include "apnewpdseeker.h"
#include "apcseeker.h"
#include "apgseeker.h"
#include "apfseeker.h"
#include "ape1seeker.h"
#include "APGlobals.h"
#include "aputils.h"
#include "apfileinfo.h"

APSeeker::APSeeker(QObject *object, QObject *parent) :
	APWorker(object, parent)
{
	_parameters = QMap<QString, float>();

	profile = ((EPSignal *)object)->profile();
	points = ((EPSignal *)object)->points();
	_proposedStart = APNotFound;

	_shouldStop = false;
	lastPeakIndiceRetrieved = 0;

	_logFile = NULL;
}

void APSeeker::cleanUpProposedSegments()
{
	emit workThrowsMessage("Cleaning up segments...");

	// Joining
	for (int i = 0; i < _proposedSegments.length() - 1; i++) {
		QPair<uint, uint> *pair = _proposedSegments.at(i);
		QPair<uint, uint> *next = _proposedSegments.at(i + 1);

		if (abs(pair->second - next->first) < param("allowedDistance")) {
			pair->second = next->second;
			_proposedSegments.removeAt(i + 1);
			i--;
			delete next;
		}
	}

	// Filtering by duration
	for (int i = 0; i < _proposedSegments.count(); i++) {
		QPair<uint, uint> *pair = _proposedSegments.at(i);

        if (std::abs((float)(pair->second - pair->first + 1)) < param("duration")) {
			_proposedSegments.removeAt(i);
			i--;
			delete pair;
		}
	}
}

bool APSeeker::didFindAnythingYet()
{
	return _proposedStart != APNotFound;
}

void APSeeker::dispatchProposedSegments()
{
//	qDebug() << "APSeeker::dispatchProposedSegments()";

	QMutex mutex;
	mutex.lock();
	if (profile->numberOfSegmentsOfType(type()) > 0)
		profile->removeAllSegmentsOfType(type());

	for (int i = 0; i < _proposedSegments.length(); i++) {
		QPair<uint, uint> *pair = _proposedSegments.at(i);
		profile->addSegment(type(), pair->first, pair->second, "", true);
	}
//	if (!_proposedSegments.isEmpty())
//		profile->forceEmitOfSegmentsDidChange();
	mutex.unlock();
}

void APSeeker::loadDefaults()
{
	setParam("allowedDistance", transformSecondsToNumberOfPoints(1.5));
	setParam("peakWindowSize", 16);
	setParam("needsLog", false);
  setParam("needsCleanUp", true);
}

float APSeeker::param(QString key)
{
	return _parameters.value(key, APNotFound);
}

QMap<QString, float> APSeeker::params()
{
	return _parameters;
}

QList<float> *APSeeker::peakIndices(Direction direction)
{
	QList<float> *indices = direction == Up ? &topPeakIndices : &bottomPeakIndices;
	if (indices->isEmpty()) {
		emit workThrowsMessage("Looking for peaks...");
		indices->append(APUtils::peaks(points, direction, param("peakWindowSize"), false));
		emit workThrowsMessage("Processing...");
	}
	return indices;
}

QList<float> APSeeker::peakIndicesInRange(Direction direction, int left, int right)
{
	QList<float> *peaks = peakIndices(direction);

//	bool peakRetrieved = false;

	QList<float> list;
	for (int i = 0; i < peaks->count(); i++) {
		float peakIndice = peaks->at(i);
		if (peakIndice < left) continue;
		else if (peakIndice > right) break;

//		if (!peakRetrieved) {
//			lastPeakIndiceRetrieved = i;
//			peakRetrieved = true;
//		}
		list << peakIndice;
	}

	return list;
}

int APSeeker::proposedStart()
{
	return _proposedStart;
}

bool APSeeker::proposedRangeSatisfyMinimumDuration(int proposedEnd)
{
	return proposedEnd - proposedStart() >= param("minDurationToBeConsidered");
}

void APSeeker::pushSegment(uint start, uint end)
{
	_proposedSegments.append( new QPair<uint, uint>(start, end) );

	unsetProposedStart();
}

int APSeeker::numberOfProposedSegments()
{
	return _proposedSegments.count();
}

void APSeeker::run()
{
//	QTime t;
//	t.start();
//	qDebug() << "APSeeker::run() => begin at" << QTime::currentTime();

	emit workLengthDidChange(100);
	emit workDidBegin();

	seek();

	if (param("needsLog")) {
		_logFile->close();
		_logFile = NULL;
	}

	if (!_shouldStop) {
		emit progressDidChange(100);
		if (param("needsCleanUp"))
			cleanUpProposedSegments();
//		dispatchProposedSegments();
	}

	emit workDidEnd();
	emit workDidEnd(true, object(), _proposedSegments.isEmpty() ? "No segments found"
																: QString("%1 segment(s) found").arg(_proposedSegments.count()));

//	qDebug().nospace() << "APSeeker::run() => time elapsed: " << t.elapsed() / 1000.0 << "s";
}

APSeeker *APSeeker::seekerForTypeAndSignal(SegmentType type, EPSignal *signal, QObject *parent)
{
	switch (type) {
	case Np: return new APNpSeeker(signal, parent);
	case Pd: return new APNewPdSeeker(signal, parent);
	case C: return new APCSeeker(signal, parent);
	case G: return new APGSeeker(signal, parent);
	case F: return new APFSeeker(signal, parent);
	case E1: return new APE1Seeker(signal, parent);

	default:
		return NULL;
	}
}

APSeeker *APSeeker::setParam(QString key, float value)
{
	_parameters.insert(key, value);
	return this;
}

void APSeeker::setProposedStart(int proposedStart)
{
	_proposedStart = proposedStart;
}

void APSeeker::setUp()
{
	loadDefaults();

  if (param("needsLog")) {
		QFileInfo fileInfo = ((EPSignal *)object())->fileInfo();
		QString filename = tr("%1-%2.log").arg(fileInfo.baseName())
				.arg( APUtils::stringWithSegmentType(type()).toLower() );
		QString filepath = tr("%1/%2").arg(fileInfo.path()).arg(filename);

		_logFile = new QFile(filepath);
		Q_ASSERT_X(_logFile->open(QFile::WriteOnly | QFile::Text), "APSeeker::setUp()", "Cannot create log file");
	}
}

void APSeeker::stop()
{
	_shouldStop = true;
}

SegmentType APSeeker::type()
{
	Q_ASSERT_X(false, "APSeeker::type", "need to be override by concrete objects");
	return (SegmentType)APNotFound;
}

void APSeeker::unsetProposedStart()
{
	_proposedStart = APNotFound;
}

void APSeeker::writeLog(QString str)
{
	QTextStream stream(_logFile);
	stream << str << "\n";
}
