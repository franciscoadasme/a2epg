#include "epsignalreader.h"
#include <QFile>
#include <QTextStream>

#include <QDebug>

#include "APGlobals.h"
#include "aputils.h"
#include "apstatuscontroller.h"

EPSignalReader::EPSignalReader(EPSignal *eps, QObject *parent) :
	APWorker(eps, parent)
{
	_epsignalLength = APEmpty; // length not known
	_epsignal = (EPSignal *)object();
	totalProgress = 0;
	moveToThread(this);
}

EPSignalReader *EPSignalReader::reader(QString filepath)
{
	EPSignalReader *reader = new EPSignalReader(new EPSignal(filepath));
	return reader;
}

void EPSignalReader::dispatchReader(QString filePath, QObject *target, const char *callback)
{
	EPSignalReader *reader = EPSignalReader::reader(filePath);
	connect(reader, SIGNAL(workDidEnd(bool, QObject*, QString)),
			target, callback);
	APStatusController::bindToWorker(reader);
	reader->start();
}

void EPSignalReader::run()
{
	emit workDidBegin("Reading " + _epsignal->fileInfo().filePath());

	if (_epsignal->fileInfo().suffix() == "epg") {
		if (!readEPG()) return;
	} else {
		// since there isn't a epg file, just use filename as signal's name
		_epsignal->setName(_epsignal->fileInfo().baseName());
		_epsignal->setDatname(_epsignal->fileInfo().fileName());
	}

	QStringList filePaths = retrieveFilePaths();
	emit workLengthDidChange(_epsignalLength * filePaths.count());
	foreach (QString filePath, filePaths)
		readDat(filePath);

	_epsignal->setChanged(false);
	emit workDidEnd();
	emit workDidEnd(true, _epsignal, _epsignal->fileInfo().filePath() + " read.");
}

void EPSignalReader::readDat(QString filePath)
{
	QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
		emit workDidEnd(false, NULL,
						tr("Can't open file at %1.").arg(filePath));
		return;
    }

	emit workDidBegin(tr("Reading %1").arg(QFileInfo(filePath).fileName()));

	QTextStream stream(&file);
    while (!stream.atEnd()) {
		float value = stream.readLine().toFloat();
		_epsignal->pushPoint(value);

		if (value > _epsignal->maximum())
			_epsignal->setMaximum(value);
		else if (value < _epsignal->minimum())
			_epsignal->setMinimum(value);

		if (_epsignalLength != APEmpty && ++totalProgress % 500 == 0) // report each 500 lines
			emit progressDidChange(totalProgress);
	}
}

bool EPSignalReader::readEPG()
{
	QFile file(_epsignal->fileInfo().filePath());
    if (!file.open(QFile::ReadOnly)) {
		emitReadingErrorWithInfo(file.errorString());
		return false;
	}

	_xmlReader.setDevice(&file);
	if (_xmlReader.readNextStartElement()) {
		if (_xmlReader.name() != "epsignal"
			|| _xmlReader.attributes().value("version") != "1.0") {
			emitReadingErrorWithInfo(tr("Parse error: either file does not follow epg format or it's not version 1.0."));
			return false;
		}

		while (_xmlReader.readNextStartElement()) {
			if (_xmlReader.name() == "info") {
				if (!readInfo())
					return false;
			} else if (_xmlReader.name() == "segments") {
				if (!readSegments())
					return false;
			} else
				_xmlReader.skipCurrentElement();
        }
	}

	if (_xmlReader.hasError()) {
		emitReadingErrorWithInfo(_xmlReader.errorString());
		return false;
	}

	return true;
}

bool EPSignalReader::readInfo()
{
	if (_xmlReader.isStartElement() && _xmlReader.name() != "info") {
		emitReadingErrorWithInfo(tr("Parse error: file does not follow epg format."));
		return false;
	}

#if DebugReading
	qDebug() << "EPSignalReader::readInfo()";
#endif

	while (_xmlReader.readNextStartElement()) {
		if (_xmlReader.name() == "name")
			_epsignal->setName(_xmlReader.readElementText());
		else if (_xmlReader.name() == "comments")
			_epsignal->setComments(_xmlReader.readElementText());
		else if (_xmlReader.name() == "datname") {
			_epsignal->setDatname(_xmlReader.readElementText());
			_datFilepath = _epsignal->fileInfo().path() + "/" + _epsignal->datname();
		} else if (_xmlReader.name() == "length")
			_epsignalLength = _xmlReader.readElementText().toInt();
        else
			_xmlReader.skipCurrentElement();
    }
	return true;
}

bool EPSignalReader::readSegment()
{
	if (_xmlReader.isStartElement() && _xmlReader.name() != "segment") {
		emitReadingErrorWithInfo(tr("Parse error: file does not follow epg format."));
		return false;
	}

#if DebugReading
	qDebug() << "EPSignalReader::readSegment()";
#endif

	// property holders
	SegmentType type = All;
	unsigned int start = APNotFound, end = APNotFound;
	QString comments;

	bool transformationWasOk = true;

	while (_xmlReader.readNextStartElement()) {
		if (_xmlReader.name() == "type")
			type = APUtils::segmentTypeFromString(_xmlReader.readElementText());
		else if (_xmlReader.name() == "start") {
			start = _xmlReader.readElementText().toUInt(&transformationWasOk);
		} else if (_xmlReader.name() == "end")
			end = _xmlReader.readElementText().toUInt(&transformationWasOk);
		else if (_xmlReader.name() == "comments")
			comments = _xmlReader.readElementText();
		else
			_xmlReader.skipCurrentElement();

		if (!transformationWasOk) {
			emitReadingErrorWithInfo(tr("Error while reading either segment start or end."));
			return false;
		}
	}

#if DebugReading
	qDebug().nospace() << "EPSignalReader::readSegment() => type=" << type
			 << ", start=" << start << ", end=" << end << ", comments=" << comments;
#endif

	// add segment with read values
	_epsignal->profile()->addSegment(type, start, end, comments, true);

	return true;
}

bool EPSignalReader::readSegments()
{
	if(_xmlReader.isStartElement() && _xmlReader.name() != "segments") {
		emitReadingErrorWithInfo(tr("Parse error: file does not follow epg format."));
		return false;
	}

#if DebugReading
	qDebug() << "EPSignalReader::readSegments()";
#endif
	while (_xmlReader.readNextStartElement())
		if (!readSegment())
			return false;
	return true;
}

QStringList EPSignalReader::retrieveFilePaths()
{
	QStringList filePaths;
	// if .epg wasn't found, so assume current file is .dat
	filePaths << (_datFilepath.isEmpty() ? _epsignal->fileInfo().filePath() : _datFilepath);

	if (filePaths.first().endsWith(".1.dat")
			|| filePaths.first().endsWith("_01.dat")) {
		for (int i = 2; ; i++) {
			QString filePath = filePaths.first();
			filePath.replace(".1.dat", QString(".%1.dat").arg(i));
			filePath.replace("_01.dat", QString("_%1%2.dat").arg(i < 10 ? "0" : "").arg(i));

			if (QFileInfo(filePath).exists())
				filePaths << filePath;
			else break;
		}
	}

	return filePaths;
}

void EPSignalReader::emitReadingErrorWithInfo(QString info)
{
	emit workDidEnd(false, NULL,
					   tr("Can't read file %1:\n\n%2")
					   .arg(_epsignal->fileInfo().fileName())
					   .arg(info));
}
