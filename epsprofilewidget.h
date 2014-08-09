#ifndef EPSEGMENTWIDGET_H
#define EPSEGMENTWIDGET_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "epsprofile.h"
#include "epsignal.h"

class EPSProfileWidget : public QFrame
{
    Q_OBJECT
public:
    explicit EPSProfileWidget(QWidget *parent = 0);
    void reset();
    EPSegment *focusedSegment();

signals:
    void focusedSegmentDidChange();
    void focusedSegmentDidChange(bool);
    void focusedSegmentDidChange(EPSegment *);

public slots:
    void currentSignalDidChange(EPSignal *signal);
    void segmentsDidChange();
    void setFocusedSegment(EPSegment *segment);

protected:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);

    void dragEvent(QMouseEvent *);

private:
    EPSProfile *_profile;
    static QColor *_segmentLineColor;
    EPSegment *_focusedSegment;

    int innerVerticalOffset;
    void setInnerVerticalOffset(int offset);
    bool isOversized;

    bool isFocusedSegment(EPSegment *segment);

    static QColor segmentLineColor();
    void updateMaximumHeight();

    bool isMouseDown, isDragging, isMouseOverHeader;
    Direction draggingEnd;

    void paintHeaderForTypes(QPainter *painter, QList<int> types);
    void paintSegmentsOfType(QPainter *painter,
                             int type,
                             uint verticalOffset);
    void paintSegment(QPainter *painter,
                      EPSegment *segment,
                      uint verticalOffset);
    void paintHorizontalGrid(QPainter *painter);
    void paintDescriptionForSegment(QPainter *painter,
                                    EPSegment *segment,
                                    QRectF rect);
    void setPaintAttributesForSegment(QPainter *painter,
                                      EPSegment *segment,
                                      QRectF rect);

    EPSegment *segmentAtPosition(QPoint position);
    bool isMouseOverAnySegmentEnds(QPoint position);
};

#endif // EPSEGMENTWIDGET_H
