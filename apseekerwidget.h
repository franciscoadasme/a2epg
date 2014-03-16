#ifndef APSEEKERWIDGET_H
#define APSEEKERWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>

#include "approgressbar.h"
#include "APGlobals.h"
#include "apseeker.h"

namespace Ui {
	class APSeekerWidget;
}

class APSeekerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit APSeekerWidget(QWidget *parent = 0);
	~APSeekerWidget();

	static void dispatchSeekersForTypes(QList<SegmentType> types, bool shouldFillGaps);
	static bool isRunning() { return _running; }

public slots:
	void seekerDidThrowMessage(QString msg);
	void seekerDidEnd(bool, QObject *, QString msg);
	void seekerProgressDidChange(int);

signals:
	void progressDidChange(int);

protected:
	void showEvent(QShowEvent *event);
	void closeEvent(QCloseEvent *event);

private:
	Ui::APSeekerWidget *ui;

	QMap<SegmentType, APProgressBar *> _progressBars;
	QMap<SegmentType, APSeeker *> _seekers;
	QMap<SegmentType, QToolButton *> _stopButtons;
	QMap<SegmentType, QLabel *> _labels;

	int _globalProgress;
	int _numberOfSeekersThatDidFinish;
	bool fillGaps;
	static bool _running;

	QTime t;

	void stopAll();
	void beforeClosing();
	void showInfoMessage();
    static QList<SegmentType> sortedTypes(QList<SegmentType> types);
};

#endif // APSEEKERWIDGET_H
