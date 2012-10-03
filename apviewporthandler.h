#ifndef APVIEWPORTHANDLER_H
#define APVIEWPORTHANDLER_H

#include <QObject>
#include <QMap>

class APViewportHandler : public QObject
{
    Q_OBJECT
	Q_PROPERTY (int offset READ offset WRITE setOffset NOTIFY offsetDidChange)

public:
	static APViewportHandler *shared();

	float horizontalZoom();
	float verticalZoom();
	uint offset();
	bool hasBeenReachedOffsetLimit();
	void changeZoom(Qt::Orientation orientation, float step);
	QString printableZoomForOrientation(Qt::Orientation orientation);
	float horizontalMultiplier();
	float verticalMultiplier();
	static uint numberOfPointsThatCanBeDrawnInWidth(uint width);
	static void goTo(uint position, Qt::AlignmentFlag aligment = Qt::AlignLeft, QWidget *target = NULL);
	float transform(float num, Qt::Orientation orientation = Qt::Horizontal);
	void move(int step);

	inline const static float horizontalScale() { return 1; }
	inline const static float verticalScale() { return 64; }

signals:
	void zoomDidChange(); // do not send zoom values, since this signal can be connected to update() slot in widgets
	void offsetDidChange(); // same here
	void offsetDidChange(int); // same here
	void zoomLimitWasReached(Qt::Orientation orientation);
	void zoomLimitWasAvoid(Qt::Orientation orientation);

public slots:
	void increaseHorizontalZoom();
	void increaseVerticalZoom();
	void decreaseHorizontalZoom();
	void decreaseVerticalZoom();
	void resetZoom();
	void setOffset(int offset);

private:
	QMap<Qt::Orientation, float> _zoomValues;
	uint _offset;

	static APViewportHandler *_instance;

	explicit APViewportHandler(QObject *parent = 0);
};

#endif // APVIEWPORTHANDLER_H
