#include "epsignalreader.h"
#include <QFile>
#include <QTextStream>

#include <QDebug>

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

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
    QFileInfo fileInfo = _epsignal->fileInfo();
    emit workDidBegin("Reading " + fileInfo.fileName());

    if (_epsignal->fileType() == EPSignal::Unknown) {
        errorMessage = "Unknown file type";
        emitReadingError();
        return;
    }

    QStringList filePaths = retrieveFilePaths();
    emit workLengthDidChange(_epsignalLength * filePaths.count());
#if DebugReading
    qDebug() << filePaths;
#endif

    BOOL success = true;
    foreach (QString filePath, filePaths) {
        success = readFile(filePath);
        if (!success) break;
    }

    if (success) {
        _epsignal->setChanged(false);
        emit workDidEnd();
        emit workDidEnd(success, _epsignal, fileInfo.fileName() + " read.");
    } else {
        emitReadingError();
    }
}

bool EPSignalReader::readBinary(QString filePath)
{
    ifstream file(qPrintable(filePath), ios::binary);

    if (file) {
        string junk;
        for (int i = 0; i < 3; i++) getline(file, junk);

        while(not file.eof()) {
            char *bfr = new char[4];
            if (file.read(bfr, 4))
                _epsignal->pushPoint(reinterpret_cast<float&>(*bfr));
            delete[] bfr;
        }
        file.close();
        return true;
    } else {
        return false;
    }
}

bool EPSignalReader::readDat(QString filePath)
{
	QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        errorMessage = tr("Can't open file at %1.").arg(filePath);
        return false;
    }

	emit workDidBegin(tr("Reading %1").arg(QFileInfo(filePath).fileName()));

	QTextStream stream(&file);
    while (!stream.atEnd()) {
		float value = stream.readLine().toFloat();
		_epsignal->pushPoint(value);

		if (_epsignalLength != APEmpty && ++totalProgress % 500 == 0) // report each 500 lines
			emit progressDidChange(totalProgress);
	}

    return true;
}

bool EPSignalReader::readFile(QString filePath)
{
    switch (_epsignal->fileType()) {
    case EPSignal::Acquisition:
        return readBinary(filePath);
    case EPSignal::Dat:
        return readDat(filePath);
    case EPSignal::EPG:
        return readEPG();
    default:
        return false;
    }
}

bool EPSignalReader::readEPG()
{
	QFile file(_epsignal->fileInfo().filePath());
    if (!file.open(QFile::ReadOnly)) {
        errorMessage = file.errorString();
		return false;
	}

    QDataStream in(&file);
    QString header;
    in >> header;

    float p;
    while (!in.atEnd()) {
        in >> p;
        _epsignal->pushPoint(p);
    }

    _xmlReader = new QXmlStreamReader(header);
    if (_xmlReader->readNextStartElement()) {
        if (_xmlReader->name() != "epsignal"
            || _xmlReader->attributes().value("version") != "1.0") {
            errorMessage = tr("Parse error: either file does not follow epg format or it's not version 1.0.");
            return false;
        }

        while (_xmlReader->readNextStartElement()) {
            if (_xmlReader->name() == "info") {
                if (!readInfo())
                    return false;
            } else if (_xmlReader->name() == "segments") {
                if (!readSegments())
                    return false;
            } else
                _xmlReader->skipCurrentElement();
        }
    }

    if (_xmlReader->hasError()) {
        errorMessage = _xmlReader->errorString();
        return false;
    }
    _xmlReader->clear();
    delete _xmlReader;

	return true;
}

bool EPSignalReader::readInfo()
{
    if (_xmlReader->isStartElement() && _xmlReader->name() != "info") {
        errorMessage = tr("Parse error: file does not follow epg format.");
		return false;
	}

#if DebugReading
	qDebug() << "EPSignalReader::readInfo()";
#endif

    while (_xmlReader->readNextStartElement()) {
        if (_xmlReader->name() == "name")
            _epsignal->setName(_xmlReader->readElementText());
        else if (_xmlReader->name() == "comments")
            _epsignal->setComments(_xmlReader->readElementText());
        else if (_xmlReader->name() == "datname") {
            _epsignal->setDatname(_xmlReader->readElementText());
			_datFilepath = _epsignal->fileInfo().path() + "/" + _epsignal->datname();
        } else if (_xmlReader->name() == "length")
            _epsignalLength = _xmlReader->readElementText().toInt();
        else
            _xmlReader->skipCurrentElement();
    }
	return true;
}

bool EPSignalReader::readSegment()
{
    if (_xmlReader->isStartElement() && _xmlReader->name() != "segment") {
        errorMessage = tr("Parse error: file does not follow epg format.");
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

    while (_xmlReader->readNextStartElement()) {
        if (_xmlReader->name() == "type")
            type = APUtils::segmentTypeFromString(_xmlReader->readElementText());
        else if (_xmlReader->name() == "start") {
            start = _xmlReader->readElementText().toUInt(&transformationWasOk);
        } else if (_xmlReader->name() == "end")
            end = _xmlReader->readElementText().toUInt(&transformationWasOk);
        else if (_xmlReader->name() == "comments")
            comments = _xmlReader->readElementText();
		else
            _xmlReader->skipCurrentElement();

		if (!transformationWasOk) {
            errorMessage = tr("Error while reading either segment start or end.");
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
    if(_xmlReader->isStartElement() && _xmlReader->name() != "segments") {
        errorMessage = tr("Parse error: file does not follow epg format.");
		return false;
	}

#if DebugReading
	qDebug() << "EPSignalReader::readSegments()";
#endif
    while (_xmlReader->readNextStartElement())
		if (!readSegment())
			return false;
	return true;
}

QStringList EPSignalReader::retrieveFilePaths()
{
    QString filePath = _epsignal->fileInfo().filePath();
    QStringList filePaths;

    QString pattern;
    switch (_epsignal->fileType()) {
    case EPSignal::Dat:
        pattern = tr("(?<=(\\.|0))\\d(?=\\.dat)");
        break;
    case EPSignal::Acquisition:
        pattern = tr("(?<=D0)\\d");
        break;
    default:
        filePaths << filePath;
        return filePaths;
    }

    QRegularExpression rx(pattern);
    for (int i = 1; i < 10; i++) {
        QString path = filePath.replace( rx, QString::number(i) );
        if (QFileInfo(path).exists())
            filePaths << path;
        else break;
    }

	return filePaths;
}

void EPSignalReader::emitReadingError()
{
	emit workDidEnd(false, NULL,
					   tr("Can't read file %1:\n\n%2")
					   .arg(_epsignal->fileInfo().fileName())
                       .arg(errorMessage));
}
