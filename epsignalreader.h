#ifndef EPSIGNALREADER_H
#define EPSIGNALREADER_H

#include "apworker.h"
#include "epsignal.h"
#include <QXmlStreamReader>

class EPSignalReader : public APWorker
{
    Q_OBJECT
public:
  explicit EPSignalReader(EPSignal *eps, bool loadRelatedFiles = true, QObject *parent = 0);

  static EPSignalReader *reader(QString filepath, bool loadRelatedFiles = true);
  static void dispatchReader(QString filePath, QObject *target, const char *callback, bool loadRelatedFiles = true);
  static bool isFilePathNumbered(QString filePath);

protected:
	void run();

private:
  QString errorMessage;
  EPSignal *_epsignal;
	int _epsignalLength;
  QXmlStreamReader *_xmlReader;
	int totalProgress;
  bool _loadRelatedFiles;

  bool readFile(QString filePath);
  bool readBinary(QString filePath);
  bool readDat(QString filePath);
	bool readEPG();
	bool readInfo();
	bool readSegment();
	bool readSegments();
  void emitReadingError();

  static QRegularExpression regexForFilePath(QString filePath);
  QStringList retrieveFilePaths();
  static QString suggestedCollectionNameBasedOnFilePath(QString filePath);
  QString successMessageForFilePaths(QStringList filePaths);
  QString successCommentForFilePaths(QStringList filePaths);
};

#endif // EPSIGNALREADER_H
