#include "epsignalwriter.h"

#include <QDebug>
#include "apstatuscontroller.h"

EPSignalWriter::EPSignalWriter(QString filepath, EPSignal *eps, int options, QObject *parent) :
	APWorker(parent)
{
	moveToThread(this);

	_epsignal = eps;
	_fileInfo = QFileInfo(filepath);

	if (options & BasedOnExtension) {
		if (_fileInfo.suffix() == "epg") {
			_options = Epg;
		} else if (_fileInfo.suffix() == "csv") {
			_options = Csv;
		} else if (_fileInfo.suffix() == "dat") {
			_options = Dat;
		}
	} else {
		_options = options;
	}
}

EPSignalWriter *EPSignalWriter::writer(QString filepath, EPSignal *eps, int options)
{
	return new EPSignalWriter(filepath, eps, options);
}

QFile *EPSignalWriter::createFile(QString filePath)
{
	QFile *file = new QFile(filePath);
    if (!file->open(QFile::WriteOnly/* | QFile::Text*/)) {
		emit workDidEnd(false, NULL, tr("Can't create file at %1:\n\n%2")
						.arg(filePath)
						.arg(file->errorString()));
		delete file;
		return NULL;
	}

	return file;
}

void EPSignalWriter::dispatchWriter(QString filePath, EPSignal *eps, QObject *target, const char *callback, int options)
{
	if (filePath.isEmpty()) return;

	EPSignalWriter *writer = EPSignalWriter::writer(filePath, eps, options);
	APStatusController::bindToWorker(writer);
	connect(writer, SIGNAL(workDidEnd(bool, QObject*, QString)),
			target, callback);
	writer->start();
}

void EPSignalWriter::run()
{
	emit workDidBegin();
	emit workDidBegin("Writing " + _fileInfo.fileName());

	if (_options & Epg) writeEpg();
	if (_options & Csv) writeCsv();
	if (_options & Dat) writeDat();

	emit workDidEnd();
	emit workDidEnd(true, _epsignal, _fileInfo.fileName() + " saved.");
}

void EPSignalWriter::writeCsv()
{
  QFile *file = createFile(_fileInfo.filePath());
  if (!file) return;

  QTextStream stream(file);
  stream.setRealNumberNotation(QTextStream::FixedNotation);
  stream.setRealNumberPrecision(2);

  stream << "Type,Start,End,Elapsed" << endl;
  foreach (EPSegment *segment, _epsignal->profile()->segmentsOfType(All)) {
    float start = transformNumberOfPointsToSeconds(segment->start());
    float end = transformNumberOfPointsToSeconds(segment->end());

    stream << segment->type()->name()
           << "," << start
           << "," << end
           << "," << end - start
           << endl;
  }

  file->close();
}

void EPSignalWriter::writeDat()
{
	QFile *file = createFile(_fileInfo.filePath().replace(".epg", ".dat"));
	if (!file) return;

	QTextStream stream(file);
	QList<float> points = _epsignal->points();

	emit workLengthDidChange(points.length());

	int pointsInOnePercent = points.length() / 100.0;
	for (int i = 0; i < points.length(); i++) {
		stream << points[i];

		if (i % pointsInOnePercent == 0)
			emit progressDidChange(i);
	}

	file->close();
}

void EPSignalWriter::writeEpg()
{
	_epsignal->setFileInfo(_fileInfo);

    QString header;
    xmlWriter = new QXmlStreamWriter(&header);
    xmlWriter->writeStartDocument();
    xmlWriter->writeDTD("<!DOCTYPE epsignal>");

    xmlWriter->writeStartElement("epsignal");
    xmlWriter->writeAttribute("version", "1.0");

	writeInfo();
	writeSegments();

    xmlWriter->writeEndElement();
    xmlWriter->writeEndDocument();
    delete xmlWriter;

    QFile *file = createFile(_fileInfo.filePath());
    if (!file) return;
    QDataStream out(file);
    out << header;
    foreach (float p, _epsignal->points()) {
        out << p;
    }

	file->close();
}

void EPSignalWriter::writeInfo()
{
    xmlWriter->writeStartElement("info");

    xmlWriter->writeTextElement("name", _epsignal->name());
    xmlWriter->writeTextElement("comments", _epsignal->comments());
    xmlWriter->writeTextElement("length", QString::number(_epsignal->numberOfPoints()));

    xmlWriter->writeEndElement();
}

void EPSignalWriter::writeSegment(EPSegment *segment)
{
    xmlWriter->writeStartElement("segment");

    xmlWriter->writeTextElement("type", segment->type()->name());
    xmlWriter->writeTextElement("start", QString::number(segment->start()));
    xmlWriter->writeTextElement("end", QString::number(segment->end()));
    xmlWriter->writeTextElement("comments", segment->comments());

    xmlWriter->writeEndElement();
}

void EPSignalWriter::writeSegments()
{
	if (_epsignal->profile()->isEmpty())
		return;

    xmlWriter->writeStartElement("segments");
    xmlWriter->writeAttribute("count", QString::number(_epsignal->profile()->numberOfSegmentsOfType(All)));

	foreach (EPSegment *segment, _epsignal->profile()->segmentsOfType(All))
		writeSegment(segment);

    xmlWriter->writeEndElement();
}
