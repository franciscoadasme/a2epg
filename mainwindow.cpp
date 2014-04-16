#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "epsignalscontroller.h"
#include "apviewporthandler.h"
#include "appanoramicwidget.h"
#include "apseekerwidget.h"
#include "aputils.h"
#include "appreferenceswidget.h"
#include "epsignalreader.h"
#include "epsignalwriter.h"
#include "apstatuscontroller.h"
#include "apsegmenttypescontroller.h"
#include "apseekerpickerdialog.h"
#include "apversion.h"

MainWindow *MainWindow::_instance = NULL;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->horizontalScrollBar->hide();
	ui->splitter->setStretchFactor(0, 1);
	ui->epsprofileWidget->hide();

	ui->navWidget->hide();
//	ui->segmentsTableWidget->setTitleLabel(ui->segmentsLabel);

	ui->signalsTableView->setModel(EPSignalsController::shared()->arrangedObjects());
	EPSignalsController::shared()->setSelectionModel(ui->signalsTableView->selectionModel());
	EPSignalsController::shared()->connect(ui->signalsTableView, SIGNAL(activated(QModelIndex)),
										   SLOT(activeIndexDidChange(QModelIndex)));

	setUpPanoramic();
	setUpConnections();
	setUpActionsAndMenus();

    move(QApplication::desktop()->screen()->rect().center() - rect().center());

	QCoreApplication::setOrganizationName("nosze");
	QCoreApplication::setApplicationName("autoepg");

	APSegmentTypesController::shared();
}

MainWindow::~MainWindow()
{
	delete ui;
}

MainWindow *MainWindow::instance()
{
	if (!_instance) {
		_instance = new MainWindow();
	}
	return _instance;
}

void MainWindow::about()
{
  QString message = tr("<p><strong style=\"font-size: 18px; line-height: 1em;\">AutoEPG %1</strong><br>"
                       "Automatic Processing of Electrical Penetration Graphs<br>"
                       "<small style=\"color: #666;\">Build %2 ~ Mar 2014</small></p>"

                       "<p>This application was developed by <a href=\"http://about.me/franciscoadasme\">Francisco Adasme</a>"
                       " and <a href=\"http://about.me/camila.munoz\">Camila Mu&ntilde;oz</a>"
                       " from Centre for Bioinformatics and Molecular Simulations"
                       " (<a href=\"http://cbsm.utalca.cl\"><abrr>CBSM</abbr></a>) in association with"
                       " <strong>Claudio C. Ramirez</strong> from Instituto de Biolog&iacute;a Vegetal y Biotecnolog&iacute;a"
                       " (<a href=\"http://biologia.utalca.cl\"><abrr>IBVB</abbr></a>), Universidad de Talca, Chile.</p>"

                       "<small>This copy can be distributed freely for research and/or academic purposes."
                       " Any kind of commercial use is not intented by the developers.</small>").arg(AP_VERSION).arg(AP_BUILD);
  QMessageBox::about(this, "AutoEPG", message);
}

void MainWindow::aboutQt()
{
	QMessageBox::aboutQt(this, windowTitle());
}

void MainWindow::addSegment()
{
	ui->epsprofileWidget->setFocusedSegment(NULL);
	ui->epsignalWidget->startAddingMode((SegmentType)((QAction *)sender())->data().toInt());
}

void MainWindow::closeCurrentSignal()
{
	EPSignalsController::shared()->closeSignal();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QList<EPSignal *> unsavedSignals = EPSignalsController::shared()->unsavedSignals();
	if (unsavedSignals.count() > 0) {
		QMessageBox msgBox(this);
		msgBox.setIcon(QMessageBox::Question);
		msgBox.setWindowTitle("AutoPEPS ~ Closing");
		msgBox.setText(QString::number(unsavedSignals.count()) + " signal(s) have been modified.");
		msgBox.setInformativeText("Do you want to save your changes?");
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Save);

		QStringList names;
		foreach (EPSignal *signal, unsavedSignals)
			names << signal->name();
		msgBox.setDetailedText("The following signals have been modified:\n\n  - " + names.join("\n  - "));

		int ret = msgBox.exec();
		if (ret == QMessageBox::Cancel) { event->ignore(); return; }
		else if (ret == QMessageBox::Save) {

			foreach (EPSignal *signal, unsavedSignals) {
				QString filePath = signal->fileInfo().filePath();
				if (signal->fileInfo().suffix() != "epg") {
					filePath = APUtils::runSaveDialog(EPSignalWriter::Epg);
					if (filePath.isEmpty()) { event->ignore(); return; }
				}

				EPSignalWriter *writer = EPSignalWriter::writer(filePath, signal, EPSignalWriter::Epg);
				writer->start();
				writer->wait(2000);
			}
		}
	}

  QSettings settings;
  settings.setValue(ZoomHorizontalKey, APViewportHandler::shared()->horizontalZoom());
  settings.setValue(ZoomVerticalKey, APViewportHandler::shared()->verticalZoom());

	event->accept();
}

void MainWindow::disconnectFromSignal(EPSignal *oldSignal)
{
	if (!oldSignal) return;
	oldSignal->disconnect(ui->actionSave);
	oldSignal->disconnect(ui->segmentsTab);

	oldSignal->profile()->setSelectionModel(NULL);
	oldSignal->profile()->disconnect(this);
	ui->segmentsTableView->disconnect(oldSignal->profile());
	ui->epsprofileWidget->disconnect(oldSignal->profile());
	oldSignal->disconnect(ui->epsprofileWidget);

	ui->actionUndo->disconnect(oldSignal->profile()->editHandler);
	ui->actionRedo->disconnect(oldSignal->profile()->editHandler);
}

EPSignalWidget *MainWindow::epsignalWidget()
{
	return ui->epsignalWidget;
}

EPSProfileWidget *MainWindow::epsprofileWidget()
{
	return ui->epsprofileWidget;
}

void MainWindow::goTo()
{
	int current = transformNumberOfPointsToSeconds(APViewportHandler::shared()->offset());
	int maximum = transformNumberOfPointsToSeconds(EPSignalsController::activeSignal()->numberOfPoints());

	bool ok;
	int position = QInputDialog::getInt(this, "AutoEPG ~ Go to", tr("Enter position (seconds) you want to go:\n\n"
																	 "Must be between 0 and %1").arg(maximum),
										current, 0, maximum, 5, &ok);
	if (ok) APViewportHandler::goTo(transformSecondsToNumberOfPoints(position));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (!EPSignalsController::hasActiveSignal()) return;

	int singleStep = ui->horizontalScrollBar->singleStep();
	int pageStep = ui->horizontalScrollBar->pageStep();

	switch (event->key()) {
	case Qt::Key_Left:
		APViewportHandler::shared()->move(-singleStep);
		break;
	case Qt::Key_Right:
		APViewportHandler::shared()->move(singleStep);
		break;
	case Qt::Key_PageUp:
		APViewportHandler::shared()->move(-pageStep);
		break;
	case Qt::Key_PageDown:
		APViewportHandler::shared()->move(pageStep);
		break;
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		qDebug("MainWindow::keyReleaseEvent() => Escape key pressed");
		ui->epsignalWidget->stopAddingMode();
	}
}

void MainWindow::openFile()
{
	QSettings settings;

  QStringList filters;
  filters << tr("All Supported files (*.epg *.D0* *.dat)")
          << tr("Electrical Penetration Signal (*.epg)")
          << tr("PROBE/Stylet+a Acquisition Data (*.D0*)")
          << tr("ASCII (*.dat)");

	QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    settings.value(LastLocationVisitedKey, QDir::homePath()).toString(),
                                                    filters.join(";;"));
	if (filePath.isEmpty()) return;
	settings.setValue(LastLocationVisitedKey, QFileInfo(filePath).path());

  bool loadRelatedFiles = true;
  if (EPSignalReader::isFilePathNumbered(filePath)) {
    int result = QMessageBox::warning(this, tr("AutoEPG ~ Opening a file"),
                                      tr("You are opening a file that appears to be part of a larger recording.\n\n"
                                         "Do you want to load existing files that belongs to the same recording as well?"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::Yes);
    loadRelatedFiles = result == QMessageBox::Yes;
  }

  EPSignalReader::dispatchReader(filePath, this, SLOT(readingDidEnd(bool,QObject*,QString)), loadRelatedFiles);
}

void MainWindow::navWidgetVisibilityChanged(bool isVisible)
{
	if (!isVisible)
		this->setFocus();
}

void MainWindow::readingDidEnd(bool success, QObject *signal, QString msg)
{
	if (success) {
		EPSignalsController::pushSignal((EPSignal *)signal);
	} else
        QMessageBox::warning(this, "Reading error", msg);

}

void MainWindow::removeFocusedSegmentAction()
{
	QSettings settings;
	if (settings.value("AskWhenDeletingSegments", true).toBool()) {
		int ret = QMessageBox::warning(this, tr("AutoEPG ~ Deleting a segment"),
									   tr("Are you sure you want to continue?.\n\n"
										  "Note: this change cannot be undone."),
									   QMessageBox::Ok | QMessageBox::Cancel,
									   QMessageBox::Ok);
		if (ret == QMessageBox::Cancel)
			return;
	}

	EPSignalsController::activeSignal()->profile()->remove();
}

void MainWindow::runAll()
{
	if (APSeekerWidget::isRunning()) {
		QMessageBox::warning(this, "AutoEPG", "There are seekers running already.");
		return;
	}

    APSeekerWidget::dispatchSeekersForTypes(QList<SegmentType>() << Np << C << Pd << E1 << G,
											ui->actionFill_Gaps->isChecked());
}

void MainWindow::runChecked()
{
	if (APSeekerWidget::isRunning()) {
		QMessageBox::warning(this, "AutoEPG", "There are seekers running already.");
		return;
	}

	APSeekerPickerDialog *picker = new APSeekerPickerDialog(this);
	if (picker->exec() == QDialog::Accepted) {

		QList<SegmentType> typesToRun = picker->selectedTypes();

		if (typesToRun.contains(C) && (!typesToRun.contains(Pd) || !typesToRun.contains(G))) {
			int ret = QMessageBox::warning(this, "AutoEPG",
										   "<strong>You are about to run C seeker without Pd and/or G contrapart.</strong><br /><br />"
										   "C search engine's known drawback is that sometimes it considers sections of Pd and G,"
										   " so it's extremely encouraged running these seekers together.<br /><br />"
										   "Would you like to run Pd and G as well?",
										   QMessageBox::Yes | QMessageBox::No);
			if (ret == QMessageBox::Yes) {
				if (!typesToRun.contains(Pd)) typesToRun << Pd;
				if (!typesToRun.contains(G)) typesToRun << G;
			}
		}

		APSeekerWidget::dispatchSeekersForTypes(typesToRun, false);
	}
}



void MainWindow::save()
{
	EPSignal *signal = EPSignalsController::activeSignal();
	QString ext = signal->fileInfo().suffix();
	QString filePath = ext == "epg" ? signal->fileInfo().filePath()
                                    : APUtils::runSaveDialog(EPSignalWriter::Epg, "epg");
	EPSignalWriter::dispatchWriter(filePath, signal,
								   this, SLOT(writingDidEnd(bool,QObject*,QString)));
}

void MainWindow::saveAs()
{
	QString filePath = APUtils::runSaveDialog(EPSignalWriter::Epg | EPSignalWriter::Dat | EPSignalWriter::Csv, "epg");

	if (QFileInfo(filePath).suffix() == "csv") {
		if (EPSignalsController::activeSignal()->profile()->isEmpty()) {
			QMessageBox::warning(this, "AutoEPG",
								 "You haven't added any waveform segment to this signal,"
								 " so there is nothing to be saved.");
			return;
    }
  }

	EPSignalWriter::dispatchWriter(filePath, EPSignalsController::activeSignal(),
								   this, SLOT(writingDidEnd(bool,QObject*,QString)));
}

void MainWindow::saveAll()
{
	QString filePath = APUtils::runSaveDialog(EPSignalWriter::Epg);
	EPSignalWriter::dispatchWriter(filePath, EPSignalsController::activeSignal(),
								   this, SLOT(writingDidEnd(bool,QObject*,QString)),
                                   EPSignalWriter::Epg);
}

void MainWindow::setUpConnections()
{
	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange(EPSignal*)),
			this, SLOT(updateUIComponentsForSignal(EPSignal*)));
	connect(EPSignalsController::shared(), SIGNAL(currentSignalWillChange(EPSignal*)),
			this, SLOT(disconnectFromSignal(EPSignal*)));

	connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)),
			APViewportHandler::shared(), SLOT(setOffset(int)));
	connect(APViewportHandler::shared(), SIGNAL(offsetDidChange(int)),
			ui->horizontalScrollBar, SLOT(setValue(int)));

	// update scroll bar
	connect(APViewportHandler::shared(), SIGNAL(zoomDidChange()),
			this, SLOT(updateScrollBar()));
	connect(EPSignalsController::shared(), SIGNAL(currentSignalDidChange()),
			this, SLOT(updateScrollBar()));
	connect(ui->epsignalWidget, SIGNAL(resized()),
			this, SLOT(updateScrollBar()));

	// update segment overlay
	connect(ui->epsprofileWidget, SIGNAL(focusedSegmentDidChange(EPSegment*)),
			ui->epsignalWidget->overlay(), SLOT(setSegment(EPSegment*)));

	connect(ui->epsignalWidget, SIGNAL(resized()),
			ui->panoramicWidget->knobWidget(), SLOT(updateKnobGeometry()));
	connect(ui->navWidget, SIGNAL(visibilityChanged(bool)),
			this, SLOT(navWidgetVisibilityChanged(bool)));
}

void MainWindow::setUpActionsAndMenus()
{
	/* seeker-related menus */
	ui->actionRun_all->setEnabled(false);
	ui->actionRun_marked->setEnabled(false);

	ui->actionAdd_segment->setEnabled(false);
	ui->actionAdd_segment->setData(APNotFound);

	/* add specific segment action */ {
		foreach (SegmentType type, APUtils::segmentTypes()) {
			QAction *action = new QAction(tr("Add %1 segment").arg(APUtils::stringWithSegmentType(type)), ui->menuEdit);
			action->setShortcut(QKeySequence(tr("f%1").arg(type + 1)));
			action->setStatusTip(tr("Add %1 segment").arg(APUtils::stringWithSegmentType(type)));
			action->setData((int)type);
			action->setEnabled(false);
			connect(action, SIGNAL(triggered()), SLOT(addSegment()));

			addSegmentActions << action;
		}

		ui->menuEdit->insertActions(ui->menuEdit->actions().at(3), addSegmentActions);
		addSegmentActions << ui->actionAdd_segment;
	}

	ui->actionRemove_Segment->setEnabled(false);
	connect(ui->epsprofileWidget, SIGNAL(focusedSegmentDidChange(bool)),
			ui->actionRemove_Segment, SLOT(setEnabled(bool)));

    ui->actionSave->setEnabled(false);
	ui->actionSave_As->setEnabled(false);
	ui->actionClose->setEnabled(false);

	ui->actionShowPanoramicWidget->setEnabled(false);
	ui->actionToggle_Mouse_Tracking->setEnabled(false);
	ui->actionGo_To->setEnabled(false);

	ui->actionRedo->setEnabled(false);
	ui->actionUndo->setEnabled(false);
}

void MainWindow::setUpPanoramic()
{
	ui->panoramicDockWidget->setMinimumWidth(QApplication::desktop()->width() / 2);
	ui->panoramicDockWidget->setGeometry(QRect(0, 0,
											   QApplication::desktop()->width() - 24,
											   ui->panoramicDockWidget->minimumHeight()));
	ui->panoramicDockWidget->move(12, 0);
	ui->panoramicDockWidget->hide();
	ui->panoramicDockWidget->setFloating(false);

	ui->prevToolButton->setEnabled(false);
	ui->nextToolButton->setEnabled(false);
}

void MainWindow::showPreferences()
{
	APPreferencesWidget::open();
}

void MainWindow::updateScrollBar()
{
	if (!EPSignalsController::hasActiveSignal())
		return; // do nothing, it supposed to be hidden

	bool needScrollBar = ui->epsignalWidget->isOversized();
	if (needScrollBar) {
		uint maximum = EPSignalsController::activeSignal()->numberOfPoints()
				- ui->epsignalWidget->numberOfPointsThatCanDraw();
		ui->horizontalScrollBar->setMaximum(maximum);

		ui->horizontalScrollBar->show();
	} else {
		ui->horizontalScrollBar->hide();
	}
}

void MainWindow::updateMenuAndActionsForSignal(EPSignal *signal)
{
	ui->actionShowPanoramicWidget->setEnabled(signal != NULL);
	ui->actionToggle_Mouse_Tracking->setEnabled(signal != NULL);
	ui->actionGo_To->setEnabled(signal != NULL);

	ui->actionRun_all->setEnabled(signal != NULL);
	ui->actionRun_marked->setEnabled(signal != NULL);

	ui->actionClose->setEnabled(signal != NULL);
	ui->actionSave->setEnabled(signal != NULL && signal->hasChanged());
	ui->actionSave_As->setEnabled(signal != NULL);

	ui->actionRedo->setEnabled(signal != NULL && !signal->profile()->editHandler->hasReachedRedoLimit());
	ui->actionUndo->setEnabled(signal != NULL && !signal->profile()->editHandler->hasReachedUndoLimit());

	if (signal) {
		ui->actionSave->connect(signal, SIGNAL(signalDidChanged(bool)), SLOT(setEnabled(bool)));

		ui->actionRedo->connect(signal->profile()->editHandler, SIGNAL(redoLimitReached(bool)), SLOT(setDisabled(bool)));
		ui->actionUndo->connect(signal->profile()->editHandler, SIGNAL(undoLimitReached(bool)), SLOT(setDisabled(bool)));
		signal->profile()->editHandler->connect(ui->actionRedo, SIGNAL(triggered()), SLOT(redo()));
		signal->profile()->editHandler->connect(ui->actionUndo, SIGNAL(triggered()), SLOT(undo()));
	}

	foreach (QAction *action, addSegmentActions) {
		action->setEnabled(signal != NULL);
	}
}

void MainWindow::updateInfoComponentsForSignal(EPSignal *signal)
{
	QSettings settings;

	if (settings.value(OpenInfoOnSignalChangeKey, false).toBool())
		ui->navWidget->show();

  ui->infoTab->setEnabled(signal != NULL);
	ui->segmentsTab->setEnabled(signal != NULL && !signal->profile()->isEmpty());

	if (signal == NULL)
		ui->tabWidget->setCurrentIndex(1);
//	else
//		ui->segmentsTab->connect(signal, SIGNAL(signalDidChanged(bool)), SLOT(setEnabled(bool)));

  ui->lengthLabel->setText( signal ? APUtils::formattedTime(signal->length(), "%hh %mm %ss") : "h m s" );
	ui->nameLineEdit->setText(signal ? signal->name() : "");
	ui->commentsTextEdit->setPlainText(signal ? signal->comments() : "");

  if (signal != NULL) {
    QStringList fileNames;
    foreach (QFileInfo fileInfo, signal->fileInfos())
      fileNames << fileInfo.fileName();
    ui->pathLabel->setText(fileNames.join("\n"));
  } else {
    ui->pathLabel->setText("path/to/file");
  }


	ui->navWidget->adjustSize();
}

void MainWindow::updateSegmentLabel()
{
	if (EPSignalsController::hasActiveSignal()) {
		EPSProfile *profile = EPSignalsController::activeSignal()->profile();
		ui->segmentsLabel->setText(tr("%1 segments found.")
								   .arg(profile->isEmpty() ? "No" : QString::number(profile->count())));
	} else {
		ui->segmentsLabel->setText("No signal found.");
	}
}

void MainWindow::updateUIComponentsForSignal(EPSignal *signal)
{
	QSettings settings;
	setWindowTitle(tr("AutoEPG%1").arg(signal == NULL ? "" : " ~ " + signal->fileInfo().fileName()));

	ui->horizontalScrollBar->setVisible(signal != NULL);
	updateScrollBar();

	ui->epsprofileWidget->setVisible(signal != NULL && !signal->profile()->isEmpty());
	if (ui->epsprofileWidget->isVisible() && ui->epsprofileWidget->height() < 10) {
		uint proposedHeight = ui->epsprofileWidget->maximumHeight();
		ui->splitter->setSizes(QList<int>() << ui->epsignalWidget->height() - proposedHeight << proposedHeight);
	}

	ui->epsprofileWidget->reset(); // reset any selected segment

	bool mayOpenPanoramic = settings.value(OpenPanoramicOnSignalChangeKey, false).toBool();
	if (mayOpenPanoramic)
		ui->panoramicDockWidget->setVisible(signal != NULL && mayOpenPanoramic);
	else if (!signal)
		ui->panoramicDockWidget->hide();

	updateInfoComponentsForSignal(signal);
	updateMenuAndActionsForSignal(signal);

	APViewportHandler::shared()->setOffset(0);

	if (signal != NULL) {
		ui->segmentsTableView->setModel(signal->profile()->arrangedObjects());
		connect(ui->segmentsTableView, SIGNAL(activated(QModelIndex)),
				signal->profile(), SLOT(activeIndexDidChange(QModelIndex)));
		signal->profile()->setSelectionModel(ui->segmentsTableView->selectionModel());
		ui->segmentsTableView->resizeColumnsToContents();

		connect(signal->profile(), SIGNAL(selectedSegmentDidChange(EPSegment*)),
				ui->epsprofileWidget, SLOT(setFocusedSegment(EPSegment*)));
		connect(ui->epsprofileWidget, SIGNAL(focusedSegmentDidChange(EPSegment*)),
				signal->profile(), SLOT(setSelectedSegment(EPSegment*)));
    connect(signal->profile(), SIGNAL(selectedSegmentDidChange(EPSegment*)),
            this, SLOT(scrollToSegment(EPSegment*)));
		connect(signal->profile(), SIGNAL(segmentsDidChange()),
				this, SLOT(updateSegmentLabel()));
	} else {
		ui->segmentsTableView->setModel(NULL);
	}

	updateSegmentLabel();
}

void MainWindow::writingDidEnd(bool success, QObject *signal, QString msg)
{
	if (success) {
		ui->pathLabel->setText( ((EPSignal *)signal)->fileInfo().filePath() );
    EPSignalsController::activeSignal()->setChanged(false);
	} else
		QMessageBox::warning(this, "Writing error", "An error ocurred while writing.\n\n" + msg);
}

void MainWindow::on_commentsTextEdit_textChanged()
{
	if (EPSignalsController::hasActiveSignal())
		EPSignalsController::activeSignal()->setComments(ui->commentsTextEdit->toPlainText());
}

void MainWindow::on_nameLineEdit_textChanged(const QString &arg1)
{
	if (EPSignalsController::hasActiveSignal())
		EPSignalsController::activeSignal()->setName(arg1);
}

void MainWindow::scrollToSegment(EPSegment *segment)
{
  EPSProfile *profile = EPSignalsController::activeSignal()->profile();
  int index = profile->indexOfObject(segment);
  if (index >= 0) {
    QModelIndex modelIndex = ui->segmentsTableView->model()->index(index, 0, QModelIndex());
    ui->segmentsTableView->scrollTo(modelIndex);
  }
}
