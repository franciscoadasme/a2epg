#include "preferenceswidget.h"
#include "ui_preferenceswidget.h"

#include "waveforms/waveformviewcontroller.h"

PreferencesWidget *PreferencesWidget::_instance = nullptr;

PreferencesWidget::PreferencesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreferencesWidget)
{
    ui->setupUi(this);
    m_waveformViewController = new WaveformViewController(ui->waveformTreeView);
}

PreferencesWidget::~PreferencesWidget()
{
    delete ui;
}

PreferencesWidget *PreferencesWidget::sharedInstance()
{
    if (_instance == nullptr)
        _instance = new PreferencesWidget();

    return _instance;
}
