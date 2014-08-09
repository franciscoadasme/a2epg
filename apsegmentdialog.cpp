#include "apsegmentdialog.h"
#include "ui_apsegmentdialog.h"

#include "mainwindow.h"
#include "APGlobals.h"
#include "epsignalscontroller.h"
#include "aputils.h"
#include "apsegmenttypescontroller.h"

APSegmentDialog *APSegmentDialog::_instance = NULL;

APSegmentDialog::APSegmentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::APSegmentDialog)
{
    ui->setupUi(this);

    ui->startSpinBox->setMinimum(0);
    ui->endSpinBox->setMinimum(0);
    ui->endSpinBox->setMaximum(APInfinite);
}

APSegmentDialog::~APSegmentDialog()
{
    delete ui;
}

APSegmentDialog *APSegmentDialog::instance()
{
    if (_instance == NULL) {
        _instance = new APSegmentDialog(MainWindow::instance());
    }
    return _instance;
}

void APSegmentDialog::accept()
{
    if (ui->typeComboBox->currentIndex() == -1) {
        QMessageBox::warning(this, "AutoEPG",
                             "No segment type was selected.\n\n"
                             "Please, select one in order to continue.");
        return;
    }

    _segment->setTranscientRange(
                transformSecondsToNumberOfPoints(ui->startSpinBox->value()),
                transformSecondsToNumberOfPoints(ui->endSpinBox->value()));

    _segment->setType(APSegmentTypesController::typeAt(
                          ui->typeComboBox->currentIndex()));
    _segment->setComments( ui->commentsTextEdit->document()->toPlainText() );

    QList<EPSegment *> collidedSegments = _segment->findCollisions();
    if (!collidedSegments.isEmpty()) {
        int ret = APSegmentDialog::askAboutCollisions(_segment, collidedSegments);
        if (ret == QMessageBox::Save) {
            _segment->pushChanges();
        } else if (ret == QMessageBox::Discard) {
            QDialog::reject();
            return;
        } else {
            ui->startSpinBox->setValue(
                        transformNumberOfPointsToSeconds(_segment->start()));
            ui->endSpinBox->setValue(
                        transformNumberOfPointsToSeconds(_segment->end()));
            _segment->resetTranscientValues();
            // if user does not accept, wait for him to fix collisions manually
            return;
        }
    } else {
        _segment->pushChanges();
    }

    QDialog::accept();
}

int APSegmentDialog::askAboutCollisions(EPSegment *segment,
                                        QList<EPSegment *> collidedSegments)
{
    QMessageBox msgBox(MainWindow::instance());
    msgBox.setWindowTitle("AutoEPG ~ Resolving collision");
    msgBox.setText(tr("<strong>%1 segment collision(s) was found.</strong>")
                   .arg(collidedSegments.length()));
    msgBox.setInformativeText("<strong>Note</strong>: segment collisions will "
                              "be automatically fixed, provoking changes in "
                              "surrounding segments and sometimes, its removal."
                              "<br /><br />"
                              "Are you sure you want to keep your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
    msgBox.setDefaultButton(QMessageBox::Discard);

    QString details = tr("Focused segment range is [ %1, %2 ] and it will be "
                         "changed to [ %3, %4 ].\n\n"
                         "Affected segments:")
                      .arg(transformNumberOfPointsToSeconds(segment->start()))
                      .arg(transformNumberOfPointsToSeconds(segment->end()))
                      .arg(transformNumberOfPointsToSeconds(segment->transcientStart()))
                      .arg(transformNumberOfPointsToSeconds(segment->transcientEnd()));
    foreach (EPSegment *seg, collidedSegments) {
        details.append(tr("\n      %1 at range [ %2, %3 ].")
                       .arg( seg->type()->name() )
                       .arg( transformNumberOfPointsToSeconds(seg->start()) )
                       .arg( transformNumberOfPointsToSeconds(seg->end()) ));
    }
    msgBox.setDetailedText(details);

    return msgBox.exec();
}

void APSegmentDialog::closeEvent(QCloseEvent *)
{
    _segment = NULL;
    ui->startSpinBox->setMaximum(APInfinite);
    ui->endSpinBox->setMaximum(APInfinite);
    ui->endSpinBox->setMinimum(0);
}

int APSegmentDialog::runForSegment(EPSegment *segment)
{
    if (segment == NULL)
        return QDialog::Rejected;

    instance()->_segment = segment;

    float start = transformNumberOfPointsToSeconds(segment->start());
    float end = transformNumberOfPointsToSeconds(segment->end());

    instance()->ui->endSpinBox->setMinimum(start);
    uint total = EPSignalsController::activeSignal()->numberOfPoints();
    instance()->ui->endSpinBox->setMaximum(transformNumberOfPointsToSeconds(total));
    instance()->ui->startSpinBox->setMaximum(end);

    instance()->ui->startSpinBox->setValue(start);
    instance()->ui->endSpinBox->setValue(end);

    instance()->ui->commentsTextEdit->setPlainText(segment->comments());

    instance()->ui->typeComboBox->clear();
    foreach (APSegmentType *type, APSegmentTypesController::types())
        instance()->ui->typeComboBox->addItem(type->name());
    instance()->ui->typeComboBox->setCurrentIndex(
                APSegmentTypesController::indexOfType(segment->type()));

    return instance()->exec();
}

void APSegmentDialog::showEvent(QShowEvent *)
{
    this->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    this->size(),
                    qApp->desktop()->availableGeometry()
                    ));
}

void APSegmentDialog::valueDidChanged(QString)
{
    ui->durationLabel->setText(tr("Duration: %1 second(s)")
                               .arg(ui->endSpinBox->value()
                                    - ui->startSpinBox->value() + 0.01));
}
