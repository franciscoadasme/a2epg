#include "epsegmentoverlay.h"

#include <QtCore>
#include <QtGui>

#include "apviewporthandler.h"

EPSegmentOverlay::EPSegmentOverlay(QWidget *parent) :
    QWidget(parent)
{
    _segment = NULL;
    hide();

    connect(APViewportHandler::shared(), SIGNAL(offsetDidChange()),
            this, SLOT(forceToUpdateGeometry()));
    connect(APViewportHandler::shared(), SIGNAL(zoomDidChange()),
            this, SLOT(forceToUpdateGeometry()));
    setMouseTracking(true);
}

void EPSegmentOverlay::forceToUpdateGeometry()
{
    if (_segment == NULL) return;

    int start = _segment->haveTranscientValues()
                ? _segment->transcientStart()
                : _segment->start();

    APViewportHandler *viewport = APViewportHandler::shared();
    int left = (start - (int)viewport->offset()) * viewport->horizontalMultiplier();
    int width = _segment->length() * viewport->horizontalMultiplier();
    int height = parentWidget()->height();

    setGeometry(left, 0, width, height);
}

void EPSegmentOverlay::paintEvent(QPaintEvent *)
{
    QPainter *painter = new QPainter(this);

    QColor bgColor = QColor(_segment->type()->color());
    QColor strokeColor = bgColor;
    bgColor.setAlphaF(.15);
    painter->setPen(strokeColor);

    painter->fillRect(rect(), bgColor);
    painter->drawLine(rect().left(), 0, rect().left(), height());
    painter->drawLine(rect().right(), 0, rect().right(), height());

    if (_segment->isCommented()) {
        QRect textRect = QRect(rect().adjusted(20, 20, -20, -20));
        textRect.moveCenter(rect().center());

        painter->drawText(textRect,
                          Qt::AlignLeading | Qt::AlignTop | Qt::TextWordWrap,
                          _segment->comments());
    }

    delete painter;
}

void EPSegmentOverlay::setSegment(EPSegment *segment)
{
    if (_segment == segment) return;

    if (_segment != NULL) _segment->disconnect(this);
    _segment = segment;

    if (_segment == NULL) {
        hide();
        return;
    }

    connect(_segment, SIGNAL(endsDidChanged()),
            SLOT(forceToUpdateGeometry()));
    connect(_segment, SIGNAL(transcientEndsDidChanged()),
            SLOT(forceToUpdateGeometry()));
    connect(_segment, SIGNAL(typeDidChanged()),
            SLOT(update()));
    connect(_segment, SIGNAL(commentsDidChanged()),
            SLOT(update()));
    forceToUpdateGeometry();
    show();
}
