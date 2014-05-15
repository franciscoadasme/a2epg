#include "epsignalreader.h"
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QMap>

#include <QDebug>

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

#include "APGlobals.h"
#include "aputils.h"
#include "apstatuscontroller.h"
#include "apfileinfo.h"

EPSignalReader::EPSignalReader(
  QList<QStringList> groupedFilePaths,
  QObject *parent) :
  APWorker(NULL, parent)
{
  readFileCount = 0;
  readSignalCount = 0;
  _totalFileCountToRead = APNotFound;
  this->groupedFilePaths = groupedFilePaths;
}

EPSignalReader::~EPSignalReader()
{
  qDeleteAll(successfulRecords);
  successfulRecords.clear();
}

void EPSignalReader::dispatchReader(
  QList<QStringList> groupedFilePaths,
  QObject *target,
  const char *callback)
{
  EPSignalReader *reader = new EPSignalReader(groupedFilePaths, target);
  connect(reader, SIGNAL(workDidEnd(bool, QObject*, QString)),
          target, callback);
  connect(reader, &EPSignalReader::finished,
          reader, &QObject::deleteLater);
	APStatusController::bindToWorker(reader);
	reader->start();
}

void EPSignalReader::run()
{
  QMap<QString, QString> failedRecords;

  emit workLengthDidChange(totalFileCountToRead());
  emit progressDidChange(0); // reset progress indicator

  foreach (QStringList filePaths, groupedFilePaths) {
    _epsignal = new EPSignal(filePaths.first());
    int success = readFileGroup(filePaths);
    if (success) {
      if (filePaths.size() > 1) {
        _epsignal->setName(_epsignal->fileInfo().baseName());
      }
      _epsignal->setChanged(false);
      successfulRecords << _epsignal;
    } else {
      failedRecords[filePaths.first()] = errorMessage;
    }
    readSignalCount++;
  }

  if (failedRecords.isEmpty()) {
    emit workDidEnd();
    foreach (EPSignal *signal, successfulRecords) {
      emit workDidEnd(true,
                      signal,
                      successMessageForSignals(successfulRecords));
    }
  } else {
    emitReadingErrors(failedRecords);
  }
}

bool EPSignalReader::readBinary(QString filePath)
{
    ifstream file(qPrintable(filePath), ios::binary);

    if (file) {
        QString comments;
        string comment;
        for (int i = 0; i < 3; i++) {
          getline(file, comment);
          comments += comment.c_str();
        }
        _epsignal->setComments(comments);

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

	QTextStream stream(&file);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    float value = line.toFloat();
    _epsignal->pushPoint(value);
	}

    return true;
}

bool EPSignalReader::readFile(QString filePath)
{
  QString beginMessage = tr("Reading %1").arg(QFileInfo(filePath).fileName());
  if (groupedFilePaths.count() > 1) {
    beginMessage += tr(" (%1/%2)").arg(readSignalCount + 1)
                                  .arg(totalRecordCountToRead());
  }
  emit workDidBegin(beginMessage);

  switch (_epsignal->fileType()) {
    case EPSignal::Acquisition:
      return readBinary(filePath);
    case EPSignal::Dat:
      return readDat(filePath);
    case EPSignal::EPG:
      return readEPG();
    default:
      errorMessage = "Unknown file type";
      return false;
  }
}

bool EPSignalReader::readFileGroup(QStringList filePaths)
{
  bool success = true;
  foreach (QString filePath, filePaths) {
    success = readFile(filePath);
    emit progressDidChange(++readFileCount);
    if (!success) break;
    _epsignal->appendFilePath(filePath);
  }
  return success;
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

void EPSignalReader::emitReadingErrors(QMap<QString, QString> errors)
{
  QStringList errorMessages;
  QMapIterator<QString, QString> i(errors);
  while (i.hasNext()) {
    i.next();
    errorMessages << tr("%1: %2").arg(APFileInfo(i.key()).fileName())
                                 .arg(i.value());
  }
  emit workDidEnd(false, NULL, errorMessages.join("\n"));
}

QString EPSignalReader::successMessageForSignals(QList<EPSignal *> records)
{
  QString message;
  if (records.count() > 1) {
    message += tr("%1 records were").arg(records.count());
  } else {
    message += "One record was";
  }
  return message + " loaded.";
}

int EPSignalReader::totalFileCountToRead()
{
  if (_totalFileCountToRead == APNotFound) {
    _totalFileCountToRead = 0;
    foreach (QStringList filePaths, groupedFilePaths) {
      _totalFileCountToRead += filePaths.count();
    }
  }

  if (_totalFileCountToRead == 1) {
    // Force to display an indetermined (busy) progress indicator
    _totalFileCountToRead = 0;
  }

  return _totalFileCountToRead;
}

int EPSignalReader::totalRecordCountToRead()
{
  return groupedFilePaths.count();
}
