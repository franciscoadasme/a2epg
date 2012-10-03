#ifndef APSEGMENTTYPE_H
#define APSEGMENTTYPE_H

#include <QObject>
#include <QColor>
#include "APGlobals.h"

class APSegmentType : public QObject
{
    Q_OBJECT
public:
	explicit APSegmentType(int id, QString name, QColor color, QObject *parent = 0);
	static APSegmentType *unserialize(QString serialized);

	bool operator ==(SegmentType type) const;
	bool operator ==(const APSegmentType & type) const;

	int id() const;
	QString name() const;
	QColor color() const;
	QString serialize() const;

signals:
	void propertyDidChanged();

public slots:
	void setName(QString name);
	void setColor(QColor color);

private:
	int _id;
	QString _name;
	QColor _color;
};

#endif // APSEGMENTTYPE_H
