#include "apzoomwidget.h"
#include "ui_apzoomwidget.h"

#include <QDebug>
#include <QPainter>

#include "epsignalwidget.h"
#include "apviewporthandler.h"

APZoomWidget::APZoomWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::APZoomWidget)
{
	ui->setupUi(this);

	_animator = new QPropertyAnimation(this, "pos", this);
	_animator->setDuration(200);
	_animator->setEasingCurve(QEasingCurve::OutQuad);

	_hotspot = rect();
	_hotspot.adjust(0, 0, 8, 8); // add a padding to cover right bottom margin

	setUpConnections();
  updateUi();

  foreach (Qt::Orientation orientation, QList<Qt::Orientation>() << Qt::Horizontal << Qt::Vertical) {
    if (APViewportHandler::shared()->isAtZoomLimit(orientation)) {
      disableButtonForOrientation(orientation);
    }
  }

	hide();
}

APZoomWidget::~APZoomWidget()
{
    delete ui;
}

void APZoomWidget::animationDidEnd()
{
	if (_animator->direction() == QPropertyAnimation::Backward)
		hide();
}

void APZoomWidget::disableButtonForOrientation(Qt::Orientation orientation)
{
	QToolButton *zoomButton = orientation == Qt::Horizontal ? ui->hZoomOutButton : ui->vZoomOutButton;
	zoomButton->setEnabled(false);
}

void APZoomWidget::enableButtonForOrientation(Qt::Orientation orientation)
{
	QToolButton *zoomButton = orientation == Qt::Horizontal ? ui->hZoomOutButton : ui->vZoomOutButton;
	zoomButton->setEnabled(true);
}

bool APZoomWidget::isPointAtHotspot(QPoint point)
{
	return _hotspot.contains(point);
}

void APZoomWidget::paintEvent(QPaintEvent *)
{
	QPainter *painter = new QPainter(this);

	QRect innerRect = rect().adjusted(1, 1, -2, -2);

	painter->setPen(QColor(192, 192, 192, 128));
	QLinearGradient gradient = QLinearGradient(innerRect.topLeft(), innerRect.bottomLeft());
	gradient.setColorAt(0, QColor(248, 248, 248, 192));
	gradient.setColorAt(1, QColor(224, 224, 224, 192));
	painter->setBrush(gradient);

	painter->drawRoundedRect(innerRect, 6, 6); // innerborder

	delete painter;
}

void APZoomWidget::setUpConnections()
{
	APViewportHandler *viewport = APViewportHandler::shared();

	connect((EPSignalWidget *)parentWidget(), SIGNAL(resized()),
			this, SLOT(updateLocation()));

	// button connections
	connect(ui->hZoomInButton, SIGNAL(clicked()),
			viewport, SLOT(increaseHorizontalZoom()));
	connect(ui->hZoomOutButton, SIGNAL(clicked()),
			viewport, SLOT(decreaseHorizontalZoom()));
	connect(ui->vZoomInButton, SIGNAL(clicked()),
			viewport, SLOT(increaseVerticalZoom()));
	connect(ui->vZoomOutButton, SIGNAL(clicked()),
			viewport, SLOT(decreaseVerticalZoom()));
	connect(ui->resetButton, SIGNAL(clicked()),
			viewport, SLOT(resetZoom()));

	connect(viewport, SIGNAL(zoomDidChange()),
			this, SLOT(updateUi()));
	connect(viewport, SIGNAL(zoomLimitWasReached(Qt::Orientation)),
			this, SLOT(disableButtonForOrientation(Qt::Orientation)));
	connect(viewport, SIGNAL(zoomLimitWasAvoid(Qt::Orientation)),
			this, SLOT(enableButtonForOrientation(Qt::Orientation)));

	connect(_animator, SIGNAL(finished()),
			this, SLOT(animationDidEnd()));
}

void APZoomWidget::slideIn()
{
	if (isVisible() || _animator->state() == QPropertyAnimation::Running)
		return;

	raise();
	show();
	_animator->setDirection(QPropertyAnimation::Forward);
	_animator->start();
}

void APZoomWidget::slideOut()
{
	if (isHidden())
		return;

	_animator->setDirection(QPropertyAnimation::Backward);
	_animator->start();
}

void APZoomWidget::updateLocation()
{
	_hotspot.moveBottomRight(parentWidget()->rect().bottomRight());
	setGeometry(_hotspot.adjusted(0, 0, -8, -8));

	_animator->setStartValue(QPoint(_hotspot.x(), parentWidget()->rect().bottom()));
	_animator->setEndValue(QPoint(_hotspot.x(), parentWidget()->rect().bottom() - height() - 8));
}

void APZoomWidget::updateUi()
{
	APViewportHandler *viewport = APViewportHandler::shared();

	ui->hZoomLabel->setText(viewport->printableZoomForOrientation(Qt::Horizontal));
	ui->vZoomLabel->setText(viewport->printableZoomForOrientation(Qt::Vertical));
}
