#ifndef APPREFERENCESWIDGET_H
#define APPREFERENCESWIDGET_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
    class APPreferencesWidget;
}

class APPreferencesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit APPreferencesWidget(QWidget *parent = 0);
    ~APPreferencesWidget();

	static void open();

private slots:
	void on_openPanoramicCheckBox_clicked();
	void on_askWhenDeletingSegmentCheckBox_clicked();
	void on_liveSegmentResizingCheckBox_clicked();

	void on_openInfoCheckBox_clicked();

	void on_segmentTypesTableView_activated(const QModelIndex &index);

	void on_removeSegmentTypeButton_clicked();

	void on_createSegmentTypeButton_clicked();
  void on_pdMaximumDurationSpinBox_valueChanged(double value);

private:
    Ui::APPreferencesWidget *ui;

	static APPreferencesWidget *_instance;
	static APPreferencesWidget *instance();
};

#endif // APPREFERENCESWIDGET_H
