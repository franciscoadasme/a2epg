#include "aputils.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "math.h"

#include "apviewporthandler.h"
#include "epsignalwriter.h"
#include "mainwindow.h"
#include "epsignalscontroller.h"

QList<SegmentType> APUtils::_segmentTypes = QList<SegmentType>() << Np << C << Pd << Pd1 << Pd2 << Pd3 << E1 << E2 << F << G;
QList<QString> APUtils::_segmentTypeNames = QList<QString>()
		<< QObject::tr("Np") << QObject::tr("C")
		<< QString("Pd") << QString("Pd1") << QString("Pd2") << QString("Pd3")
		<< QString("E1") << QString("E2") << QString("F") << QString("G");
QList<SegmentType> APUtils::segmentTypesOrderedByConfidence = QList<SegmentType>()
		<< Np << Pd << G << F << E1 << C;
QList<int> APUtils::durationOfSegmentTypes = QList<int>() << 5 << 4 << 2 << 2 << 2 << 2 << 15 << 15 << 15 << 20;

APUtils::APUtils()
{
	Q_ASSERT_X(true, "APUtils", "instantiating a static class");
}

QList<SegmentType> APUtils::segmentTypes()
{
	return APUtils::_segmentTypes;
}

QList<QString> APUtils::segmentTypeNames()
{
	return APUtils::_segmentTypeNames;
}

QColor APUtils::colorForSegmentType(SegmentType type)
{
	switch(type) {
	case Np: return Qt::gray;
	case Pd: return Qt::red;
	case Pd1: return QColor(Qt::red).lighter(150);
	case Pd2: return QColor(Qt::red).lighter(125);
	case Pd3: return QColor(Qt::red).darker(125);
	case C: return Qt::darkYellow;
	case G: return Qt::green;
	case F: return Qt::darkGreen;
	case E1: return Qt::cyan;
	case E2: return Qt::darkCyan;
	default: return Qt::black;
	}
}

int APUtils::durationForSegmentType(int type)
{
	return durationOfSegmentTypes.at(type);
}

SegmentType APUtils::mostReliableType(int type1, int type2)
{
	int confidence1 = (int)segmentTypesOrderedByConfidence.indexOf((SegmentType)type1);
	int confidence2 = (int)segmentTypesOrderedByConfidence.indexOf((SegmentType)type2);
	return (SegmentType)(confidence1 <= confidence2 ? type1 : type2);
}

SegmentType APUtils::segmentTypeFromString(QString string)
{
	unsigned int index = 0;
	foreach (QString segmentTypeName, APUtils::segmentTypeNames()) {
		if (string.compare(segmentTypeName, Qt::CaseInsensitive) == 0) {
			return APUtils::segmentTypes().at(index);
		}
		index++;
	}
	return (SegmentType)APNotFound;
}

QString APUtils::stringWithSegmentType(SegmentType type)
{
	unsigned int index = 0;
	foreach (SegmentType segmentType, APUtils::segmentTypes()) {
		if (type == segmentType) {
			return APUtils::segmentTypeNames().at(index);
		}
		index++;
	}
	return QObject::tr("");
}

QString APUtils::formattedTime(unsigned int timestamp, QString format)
{
	int hours, mins;
	if (format.contains("%h")) {
		hours = timestamp / 3600;
		mins  = (int)timestamp % 3600 / 60;
	} else {
		hours = 0;
		mins = timestamp / 60;
	}

	int secs  = timestamp - hours*3600 - mins*60;
	return format.replace("%h", QString::number(hours), Qt::CaseInsensitive)
			.replace("%m", QString::number(mins), Qt::CaseInsensitive)
			.replace("%s", QString::number(secs), Qt::CaseInsensitive);
}

void APUtils::paintSignalUsingPainterWithRect(EPSignal *signal, QPainter *painter, QRect bounds)
{
	painter->save();
	painter->setPen(Qt::black);

	APViewportHandler *viewport = APViewportHandler::shared();
//	uint paintUntil = bounds.width()/viewport->horizontalZoom() > signal->numberOfPoints() - viewport->offset() ?
//				signal->numberOfPoints() - viewport->offset() : bounds.width()/viewport->horizontalZoom();
	uint numberOfPointsToDraw = bounds.width() / viewport->horizontalMultiplier();
	if (numberOfPointsToDraw > signal->numberOfPoints()) {
		numberOfPointsToDraw = signal->numberOfPoints() - viewport->offset();

		// is offset + width is greater than points, decrease offset!
	}

	QList<float> points = signal->points();
	for (uint i = 1; i < numberOfPointsToDraw; i++) {
		int x = i + viewport->offset();
		if (x > points.length() - 1)
			break;

		// be aware that we multiply by -1 to flip out the signal, so positive values point to top
		QPointF leftPoint = QPointF((i - 1) * viewport->horizontalMultiplier(),
									-points.at(x - 1) * viewport->verticalMultiplier());
		QPointF rightPoint = QPointF(i * viewport->horizontalMultiplier(),
									 -points.at(x) * viewport->verticalMultiplier());

		painter->drawLine(leftPoint, rightPoint);
	}

	painter->restore();
}

void APUtils::paintHorizontalGridInRect(QPainter *painter, QRect rect)
{
	painter->save();

	uint pointPerSecond = 1.0 / APSampleRate;
	APViewportHandler *viewport = APViewportHandler::shared();
	uint offset = viewport->offset();

	painter->setPen(QColor(0, 0, 0, 16));
	for (uint i = offset; i < offset + rect.width()/viewport->horizontalMultiplier(); ++i) {
		if (i % pointPerSecond == 0) {

			QLineF line = QLineF((i - offset) * viewport->horizontalMultiplier(), 1,
								 (i - offset) * viewport->horizontalMultiplier(), rect.height());
			painter->drawLine(line);
			if (i > 0 && i % (pointPerSecond*5) == 0) {
				painter->drawLine(line);
				painter->drawLine(line);
			}
		}
	}

	painter->restore();
}

float APUtils::sum(QList<float> list)
{
	float result = 0;
	foreach (float num, list) {
		result += num;
	}
	return result;
}

bool APUtils::isPeak(float num, QList<float> valuesInsideWindow, Direction dir)
{
	foreach (float value, valuesInsideWindow) {
		if (num == value) continue; /* check if this does not happen */

		bool isValueHigherPeakThanNum = dir == Up && value > num;
		bool isValueLowerPeakThanNum = dir == Down && value < num;

		if (isValueHigherPeakThanNum || isValueLowerPeakThanNum)
			return false;
	}
	return true;
}

float APUtils::getAverage(QList<float> list)
{
	return list.isEmpty() ? -APInfinite : APUtils::sum(list) / (float)list.count();
}

float APUtils::getDeviation(QList<float> list, float average)
{
	if (list.isEmpty()) return -APInfinite;

	float result = 0;
	foreach (float num, list) {
		result += pow(num - average, 2);
	}
	return sqrt(result / list.length());
}

QList<float> APUtils::peaks(QList<float> list, Direction dir, int windowSize, bool returnValues, bool checkSignificance)
{
//	QTime t;
//	t.start();
//	qDebug().nospace() << "APUtils::peaks(" << (dir == Up ? "Up" : "Down")
//					   << ") => begin at " << QTime::currentTime();

	QList<float> peaks;
	/* points at limits cannot be included since we dont know points beyond window */
	for (int i = 1; i < list.length() - 1; i++) {
		bool isProposedPeak = true;

		for (int offset = 1; offset <= windowSize / 2; offset++) {
			int leftSiblingIndex = i - offset;
			int rightSiblingIndex = i + offset;

			/* compare which each sibling and check if it a peak... if not, break the loops */ {
				foreach (int siblingIndex, QList<int>() << leftSiblingIndex << rightSiblingIndex) {
					if (siblingIndex < 0 || siblingIndex >= list.length())
						continue;

					bool notTopPeak = dir == Up && list[i] < list[siblingIndex];
					bool notBottomPeak = dir == Down &&  list[i] > list[siblingIndex];
					// it there are two equals, choose at leading pos
					bool notLeadingCopy = list[i] == list[siblingIndex] && i > siblingIndex;

					bool notSignificantPeak = qAbs(list[i] - list[siblingIndex]) < .05;

					if (notTopPeak || notBottomPeak || notLeadingCopy || (checkSignificance && notSignificantPeak)) {
						isProposedPeak = false;
						break;
					}
				}
			}

			if (!isProposedPeak) break;
		}

		if (isProposedPeak)
			peaks << (returnValues ? list.at(i) : i);
	}

//	qDebug().nospace() << "APUtils::peaks(" << (dir == Up ? "Up" : "Down")
//					   << ") => time elapsed: " << t.elapsed() / 1000.0 << "s";
	return peaks;
}

float APUtils::slope(QPointF a, QPointF b)
{
	return (b.y() - a.y()) / (b.x() - a.x());
}

QString APUtils::runSaveDialog(int extensionFlags, QString defaultExtension)
{
	QStringList filters, extensions;
	if (extensionFlags & EPSignalWriter::Epg) {
		filters << "Electrical penetration signal (*.epg)";
		extensions << "epg";
	}
	if (extensionFlags & EPSignalWriter::Dat) {
		filters << ".dat (*.dat)";
		extensions << "dat";
	}
	if (extensionFlags & EPSignalWriter::Csv) {
		filters << "Comma-separated values (*.csv)";
		extensions << "csv";
	}

	QSettings settings;
	QString selectedFilter = filters.first();
	QString selectedDir = settings.value(LastLocationVisitedKey, QDir::homePath()).toString();
	QString proposedName = EPSignalsController::activeSignal()->fileInfo().baseName().replace(" ", "_");
	if (!defaultExtension.isEmpty())
		proposedName.append("." + defaultExtension);

	QString fileName = QFileDialog::getSaveFileName(MainWindow::instance(), QObject::tr("Save"),
													selectedDir + "/" + proposedName,
													filters.join(";;"),
													&selectedFilter);
	if (fileName.isEmpty()) return "";
	else {
		QFileInfo fileInfo(fileName);
		settings.setValue(LastLocationVisitedKey, fileInfo.path());

		if (fileInfo.suffix().isEmpty() || !extensions.contains(fileInfo.suffix())) { // append extension based on selected filter
			foreach (QString filter, filters) {
				if (filter == selectedFilter) {
					fileName.append(extensions[filters.indexOf(filter)]);
					break;
				}
			}
		}
	}

	return fileName/*.replace(" ", "_")*/;
}

QColor APUtils::randomColor()
{
	int red = qrand() % 224;
	int green = qrand() % 224;
	int blue = qrand() % 224;
	return QColor(32 + red, 32 + green, 32 + blue);
}
