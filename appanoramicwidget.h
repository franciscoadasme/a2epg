#ifndef APPANORAMICWIDGET_H
#define APPANORAMICWIDGET_H

#include <QFrame>

#include "epsignal.h"

class APPanoramicKnobWidget : public QWidget
{
    Q_OBJECT
public:
    explicit APPanoramicKnobWidget(QWidget *parent = 0);
    public slots:
    void updateKnobGeometry();
    void mouseReleaseEvent(QMouseEvent *);
    protected:
    void paintEvent(QPaintEvent *);
};

class APPanoramicSignalWidget : public QWidget
{
    Q_OBJECT
public:
    explicit APPanoramicSignalWidget(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
private:
    QRect prevRect;
};

class APPanoramicWidget : public QFrame
{
    Q_OBJECT
public:
    explicit APPanoramicWidget(QWidget *parent = 0);

    APPanoramicSignalWidget *signalWidget();
    APPanoramicKnobWidget *knobWidget();
    int offset();
    int numberOfPointsThatCanBeDrawn();
    void setShouldUpdateOffset(bool flag);

signals:
    void resized();
    void updateLeftArrowState(bool);
    void updateRightArrowState(bool);
    void updateRangeText(QString);

public slots:
    void updateContent(EPSignal *signal);
    void updateOffset(int viewportOffset);
    void goToNextSection();
    void goToPrevSection();
    void goToAnimationDidEnd();

protected:
    void resizeEvent(QResizeEvent *);

private:
    APPanoramicSignalWidget *_signalWidget;
    APPanoramicKnobWidget *_knobWidget;
    int _offset;
    bool _shouldUpdateOffset; // allow to avoid updating offset when we call to goTo

    void emitRangeText();
};

#endif // APPANORAMICWIDGET_H
