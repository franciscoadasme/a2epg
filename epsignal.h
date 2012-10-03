#ifndef EPSIGNAL_H
#define EPSIGNAL_H

#include <QObject>
#include <QFileInfo>
#include "epsprofile.h"
#include "APGlobals.h"

class EPSegment;

class EPSignal : public QObject
{
    Q_OBJECT
	Q_PROPERTY (QString name READ name WRITE setName NOTIFY nameDidChanged)

public:
	explicit EPSignal(QString filepath, int id = APNotFound, QObject *parent = 0);

	// accessors
	int id();
	QString name();
	void setName(QString name);
	QString datname();
	void setDatname(QString datname);
	QString comments();
	void setComments(QString aComment);
	bool isCommented();
	QFileInfo fileInfo();
	void setFileInfo(QFileInfo fileInfo);
	QList<float> points();
	uint length();
	uint numberOfPoints();
	EPSProfile *profile();
	bool hasMorePointsThan(uint number);

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
	QString _datname;
	QString _comments;
	QFileInfo _fileInfo;
	bool _changed;

	QList<float> _points;
	EPSProfile *_profile;
	float _minimum, _maximum;
};

#endif // EPSIGNAL_H
