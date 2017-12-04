#ifndef PREFERENCESWIDGET_H
#define PREFERENCESWIDGET_H

#include <QMenu>
#include <QWidget>

class WaveformViewController;

namespace Ui {
class PreferencesWidget;
}

class PreferencesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreferencesWidget(QWidget *parent = 0);
    ~PreferencesWidget();

    static PreferencesWidget *sharedInstance();

private slots:

private:
    static PreferencesWidget *_instance;

    Ui::PreferencesWidget *ui;
    WaveformViewController *m_waveformViewController;
};

#endif // PREFERENCESWIDGET_H
