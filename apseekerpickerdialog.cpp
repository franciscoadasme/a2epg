#include "apseekerpickerdialog.h"
#include "ui_apseekerpickerdialog.h"

#include "QSettings"

#include "aputils.h"

#define SelectedSearchEngineseKey "SelectedSearchEngines"

APSeekerPickerDialog::APSeekerPickerDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::APSeekerPickerDialog)
{
	ui->setupUi(this);

	checkBoxs.insert(Pd, ui->pdCheckBox);
	checkBoxs.insert(Np, ui->npCheckBox);
	checkBoxs.insert(G, ui->gCheckBox);
	checkBoxs.insert(F, ui->fCheckBox);
	checkBoxs.insert(E1, ui->e1CheckBox);
//	checkBoxs.insert(E2, ui->e2CheckBox);
	checkBoxs.insert(C, ui->cCheckBox);

	QSettings settings;

	settings.beginGroup(SelectedSearchEngineseKey);
	QStringList keys = settings.childKeys();
	foreach (QString key, keys) {
		SegmentType type = APUtils::segmentTypeFromString(key);
		bool wasSeekerSelected = settings.value(key).toBool();
		if (checkBoxs.contains(type))
			checkBoxs.value(type)->setChecked(wasSeekerSelected);
	}
	settings.endGroup();
}

APSeekerPickerDialog::~APSeekerPickerDialog()
{
	delete ui;
}

void APSeekerPickerDialog::accept()
{
	QSettings settings;

	settings.beginGroup(SelectedSearchEngineseKey);
	QMap<SegmentType, QCheckBox *>::const_iterator i = checkBoxs.constBegin();
	while (i != checkBoxs.constEnd()) {
		settings.setValue(APUtils::stringWithSegmentType(i.key()), i.value()->isChecked());
		++i;
	}
	settings.endGroup();

	QDialog::accept();
}

void APSeekerPickerDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if (button->text() == "Reset") {
		QMap<SegmentType, QCheckBox *>::const_iterator i = checkBoxs.constBegin();
		while (i != checkBoxs.constEnd()) {
			i.value()->setChecked(false);
			++i;
		}
	}
}

QList<SegmentType> APSeekerPickerDialog::selectedTypes()
{
	QList<SegmentType> types;

	QMap<SegmentType, QCheckBox *>::const_iterator i = checkBoxs.constBegin();
	while (i != checkBoxs.constEnd()) {

		if (i.value()->isChecked()) {
			types << i.key();
		}

		++i;
	}

	return types;
}
