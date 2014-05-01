#include "appreferenceswidget.h"
#include "ui_appreferenceswidget.h"

#include <QtCore>
#include <QColorDialog>
#include <QMessageBox>
#include <QInputDialog>

#include "APGlobals.h"
#include "apsegmenttypescontroller.h"
#include "aputils.h"

APPreferencesWidget *APPreferencesWidget::_instance = NULL;

APPreferencesWidget::APPreferencesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::APPreferencesWidget)
{
    ui->setupUi(this);

	QSettings settings;
	ui->openPanoramicCheckBox->setChecked(settings.value(OpenPanoramicOnSignalChangeKey, false).toBool());
	ui->askWhenDeletingSegmentCheckBox->setChecked(settings.value(AskWhenDeletingSegmentsKey, true).toBool());
	ui->liveSegmentResizingCheckBox->setChecked(settings.value(LiveSegmentResizingKey, false).toBool());
	ui->openInfoCheckBox->setChecked(settings.value(OpenInfoOnSignalChangeKey, false).toBool());
  ui->pdMaximumDurationSpinBox->setValue(settings.value(PdMaximumDurationKey, 10).toFloat());

	ui->segmentTypesTableView->setModel(APSegmentTypesController::shared()->arrangedObjects());
	APSegmentTypesController::shared()->setSelectionModel(ui->segmentTypesTableView->selectionModel());
}

APPreferencesWidget::~APPreferencesWidget()
{
    delete ui;
}

APPreferencesWidget *APPreferencesWidget::instance()
{
	if (_instance == NULL)
		_instance = new APPreferencesWidget();
	return _instance;
}

void APPreferencesWidget::open()
{
	instance()->show();
}

void APPreferencesWidget::on_openPanoramicCheckBox_clicked()
{
	QSettings settings;
	settings.setValue(OpenPanoramicOnSignalChangeKey, ui->openPanoramicCheckBox->isChecked());
}

void APPreferencesWidget::on_askWhenDeletingSegmentCheckBox_clicked()
{
	QSettings settings;
	settings.setValue(AskWhenDeletingSegmentsKey, ui->askWhenDeletingSegmentCheckBox->isChecked());
}

void APPreferencesWidget::on_liveSegmentResizingCheckBox_clicked()
{
	QSettings settings;
	settings.setValue(LiveSegmentResizingKey, ui->liveSegmentResizingCheckBox->isChecked());
}

void APPreferencesWidget::on_openInfoCheckBox_clicked()
{
	QSettings settings;
	settings.setValue(OpenInfoOnSignalChangeKey, ui->openInfoCheckBox->isChecked());
}

void APPreferencesWidget::on_segmentTypesTableView_activated(const QModelIndex &index)
{
	if (index.column() != 0) return; // only edit color, which is at fist column... anything else, avoid it

	QColor currentColor = APSegmentTypesController::shared()->data(index, Qt::DecorationRole).value<QColor>();
	QColor color = QColorDialog::getColor(currentColor, this);
	if (color.isValid())
		APSegmentTypesController::shared()->setData(index, color, Qt::EditRole);
}

void APPreferencesWidget::on_removeSegmentTypeButton_clicked()
{
	if (APSegmentTypesController::shared()->removable()) {
		APSegmentTypesController::shared()->remove();
	} else
		QMessageBox::warning(this, "AutoEPG",
							 "<strong>An error occurs while removing a segment type.</strong><br /><br />"
							 "Either there is no selection or the segment type is not removable.");
	ui->segmentTypesTableView->setFocus();
}

void APPreferencesWidget::on_createSegmentTypeButton_clicked()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("AutoEPG ~ New segment type"),
										 tr("Segment type name (3 letters):"), QLineEdit::Normal,
										 "", &ok);
	if (!ok || text.isEmpty()) return;

	if (!QRegExp("[a-zA-Z0-9]*").exactMatch(text)) {
		int ret = QMessageBox::warning(this, "AutoEPG",
									   "<strong>Invalid non-alphanumeric characters were found.</strong><br />"
									   "They will be removed automatically.<br /><br />"
									   "Are you sure you want to continue?",
									   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
		if (ret == QMessageBox::Ok)
			text.remove(QRegExp(QString::fromUtf8("[-\\s`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\]]")));
		else return;
	}

	if (text.length() > 3) {
		QMessageBox::warning(this, "AutoEPG",
							 "<strong>Invalid name found.</strong><br /><br />"
							 "Entered name exceeds the allowed length.");
		return;
	}

	APSegmentTypesController::shared()->addSegmentType(text, APUtils::randomColor());
}

void APPreferencesWidget::on_pdMaximumDurationSpinBox_valueChanged(double value)
{
  QSettings settings;
  settings.setValue(PdMaximumDurationKey, value);
}
