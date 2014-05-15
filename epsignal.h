#ifndef EPSIGNAL_H
#define EPSIGNAL_H

#include <QObject>
#include <QFileInfo>
#include "epsprofile.h"
#include "APGlobals.h"
#include "apfileinfo.h"

class EPSegment;

class EPSignal : public QObject
{
  Q_OBJECT
	Q_PROPERTY (QString name READ name WRITE setName NOTIFY nameDidChanged)

public:
	explicit EPSignal(QString filepath, int id = APNotFound, QObject *parent = 0);

  enum FileType {
    EPG,
    Dat,
    Acquisition,
    Unknown
  };

	// accessors
	int id();
	QString name();
	void setName(QString name);
	QString comments();
	void setComments(QString aComment);
  void appendComment(QString aComment);
	bool isCommented();
  APFileInfo fileInfo();
  QList<APFileInfo> fileInfos();
  void setFileInfo(APFileInfo fileInfo);
  void appendFilePath(QString filePath);
	QList<float> points();
	uint length();
	uint numberOfPoints();
	EPSProfile *profile();
	bool hasMorePointsThan(uint number);
  FileType fileType();

	bool hasChanged();
	void setChanged(bool changed);

	float minimum();
	void setMinimum(float minimum);
	float maximum();
	void setMaximum(float maximum);
	float amplitude();

	// actions
	void pushPoint(float point);

public slots:
	void profileDidChanged();

signals:
	void nameDidChanged();
	void nameDidChanged(QString);
	void commentsDidchange(QString);
	void fileInfoDidChange();
	void signalDidChanged();
	void signalDidChanged(bool);

private:
	int _id;
  QString _name;
	QString _comments;
  QList<APFileInfo> _fileInfos;
	bool _changed;
  FileType _fileType;

	QList<float> _points;
	EPSProfile *_profile;
	float _minimum, _maximum;

  EPSignal::FileType deriveFileType(APFileInfo fileInfo);
};

#endif // EPSIGNAL_H
