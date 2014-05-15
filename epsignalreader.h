#ifndef EPSIGNALREADER_H
#define EPSIGNALREADER_H

#include "apworker.h"
#include "epsignal.h"

class QXmlStreamReader;

class EPSignalReader : public APWorker
{
    Q_OBJECT
public:
  explicit EPSignalReader(
    QList<QStringList> groupedFilePaths,
    QObject *parent = 0);
  ~EPSignalReader();
  static void dispatchReader(
    QList<QStringList> groupedFilePaths,
    QObject *target,
    const char *callback);
  static bool isFilePathNumbered(QString filePath);

protected:
	void run();

private:
  QString errorMessage;
  EPSignal *_epsignal;
  QXmlStreamReader *_xmlReader;
  int readFileCount;
  int readSignalCount;
  int _totalFileCountToRead;

  QList<QStringList> groupedFilePaths;
  int totalFileCountToRead();
  int totalRecordCountToRead();

  QList<EPSignal *> successfulRecords;

  bool readFileGroup(QStringList filePaths);
  bool readFile(QString filePath);
  bool readBinary(QString filePath);
  bool readDat(QString filePath);
	bool readEPG();
	bool readInfo();
	bool readSegment();
	bool readSegments();
  void emitReadingErrors(QMap<QString, QString> errors);

  QString successMessageForSignals(QList<EPSignal *> records);
};

#endif // EPSIGNALREADER_H
