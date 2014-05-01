#ifndef APUTILS_H
#define APUTILS_H

#include <QtCore>
#include <QtGui>

#include "APGlobals.h"
#include "epsignal.h"

class APUtils
{
public:
	APUtils();

	static QList<SegmentType> segmentTypes();
	static QList<QString> segmentTypeNames();

	static SegmentType segmentTypeFromString(QString string);
	static QString stringWithSegmentType(SegmentType type);
	static QColor colorForSegmentType(SegmentType type);
	static SegmentType mostReliableType(int type1, int type2);
	static int durationForSegmentType(int type);

	static QString formattedTime(unsigned int timestamp, QString format);

	static void paintSignalUsingPainterWithRect(EPSignal *signal, QPainter *painter, QRect bounds);
	static void paintVerticalGridInRect(QPainter *painter, QRect rect);

	static float sum(QList<float> list);
	static bool isPeak(float num, QList<float> valuesInsideWindow, Direction dir);
	static float getAverage(QList<float> list);
	static float getDeviation(QList<float> list, float average);
	static QList<float> peaks(QList<float> list, Direction dir, int windowSize, bool returnValues = true, bool checkSignificance = false);
	static float slope(QPointF a, QPointF b);

	static QString runSaveDialog(int extensionFlags, QString defaultExtension = "");

	static QColor randomColor();

private:
	static QList<SegmentType> _segmentTypes;
	static QList<QString> _segmentTypeNames;
	static QList<SegmentType> segmentTypesOrderedByConfidence;
	static QList<int> durationOfSegmentTypes;
};

#endif // APUTILS_H
