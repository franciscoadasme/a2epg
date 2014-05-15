#ifndef EPSIGNALWRITER_H
#define EPSIGNALWRITER_H

#include "apworker.h"
#include "epsignal.h"
#include "apfileinfo.h"

class QXmlStreamWriter;

class EPSignalWriter : public APWorker
{	
    Q_OBJECT
public:
	explicit EPSignalWriter(QString filepath, EPSignal *eps, int options = BasedOnExtension, QObject *parent = 0);

	enum WritingFlags {
		Epg = 1,
		Dat = 2,
		Csv = 4,
		BasedOnExtension = 8
	};

	static EPSignalWriter *writer(QString filepath, EPSignal *eps, int options = BasedOnExtension);
	static void dispatchWriter(QString filePath, EPSignal *eps,
							   QObject *target = 0, const char *callback = 0,
							   int options = BasedOnExtension);

protected:
	void run();

private:
	EPSignal *_epsignal;
  APFileInfo _fileInfo;
	int _options;
  QXmlStreamWriter *xmlWriter;

	QFile *createFile(QString filePath);

	void writeDat();
	void writeEpg();
	void writeCsv();

	void writeInfo();
	void writeSegments();
	void writeSegment(EPSegment *segment);
};

#endif // EPSIGNALWRITER_H
