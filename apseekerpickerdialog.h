#ifndef APSEEKERPICKERDIALOG_H
#define APSEEKERPICKERDIALOG_H

#include <QDialog>
#include <QMap>
#include <QCheckBox>

#include "APGlobals.h"

namespace Ui {
class APSeekerPickerDialog;
}

class APSeekerPickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit APSeekerPickerDialog(QWidget *parent = 0);
    ~APSeekerPickerDialog();

    QList<SegmentType> selectedTypes();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::APSeekerPickerDialog *ui;

    QMap<SegmentType, QCheckBox *> checkBoxs;
    void accept();
};

#endif // APSEEKERPICKERDIALOG_H
