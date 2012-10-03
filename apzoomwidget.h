#ifndef APZOOMWIDGET_H
#define APZOOMWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

namespace Ui {
    class APZoomWidget;
}

class APZoomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit APZoomWidget(QWidget *parent = 0);
    ~APZoomWidget();

	void slideIn();
	void slideOut();
	bool isPointAtHotspot(QPoint point);

public slots:
	void updateLocation();
	void updateUi();
	void enableButtonForOrientation(Qt::Orientation);
	void disableButtonForOrientation(Qt::Orientation);
	void animationDidEnd();

protected:
	void paintEvent(QPaintEvent *);

private:
    Ui::APZoomWidget *ui;
	QPropertyAnimation *_animator;
	QRect _hotspot;

	void setUpConnections();
};

#endif // APZOOMWIDGET_H
