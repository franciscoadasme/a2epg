#ifndef EPSIGNALREADER_H
#define EPSIGNALREADER_H

#include "apworker.h"
#include "epsignal.h"
#include <QXmlStreamReader>

class EPSignalReader : public APWorker
{
    Q_OBJECT
public:
    explicit EPSignalReader(EPSignal *eps, QObject *parent = 0);

	static EPSignalReader *reader(QString filepath);
	static void dispatchReader(QString filePath, QObject *target = 0, const char *callback = 0);

protected:
	void run();

private:
    QString _datFilepath;
    QString errorMessage;
    EPSignal *_epsignal;
	int _epsignalLength;
	QXmlStreamReader _xmlReader;
	int totalProgress;

    bool readFile(QString filePath);
    bool readBinary(QString filePath);
    bool readDat(QString filePath);
	bool readEPG();
	bool readInfo();
	bool readSegment();
	bool readSegments();
    void emitReadingError();
    QStringList retrieveFilePaths();
};

#endif // EPSIGNALREADER_H
