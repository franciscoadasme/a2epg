#include "epsignalscontroller.h"
#include "epsignalreader.h"

#include <QDebug>
#include <QMessageBox>

#include "APGlobals.h"
#include "apstatuscontroller.h"
#include "mainwindow.h"
#include "aputils.h"
#include "epsignalwriter.h"

#define NameColumn 0
#define LengthColumn 1

EPSignalsController *EPSignalsController::_shared = NULL;

EPSignalsController::EPSignalsController(QObject *parent) :
	APController(parent)
{
	_activeSignal = NULL;
	connect(this, SIGNAL(activeObjectDidChanged(QObject*)),
			SLOT(updateActiveObject(QObject*)));
}

EPSignalsController *EPSignalsController::shared()
{
	if (_shared == NULL) {
		_shared = new EPSignalsController;
	}
	return _shared;
}

void EPSignalsController::appendSignal(EPSignal *signal)
{
	addObject(signal);
	setActiveSignal(signal);
}

EPSignal *EPSignalsController::activeSignal()
{
	return EPSignalsController::shared()->_activeSignal;
}

void EPSignalsController::closeSignal()
{
	EPSignal *signal = EPSignalsController::activeSignal();
	if (signal->hasChanged() && !handleUnsavedSignal(signal))
		return;

	remove();
	setActiveSignal((EPSignal *)selectedObject());
  delete signal;
}

int EPSignalsController::columnCount(const QModelIndex &) const
{
	return 2;
}

QVariant EPSignalsController::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= count())
		return QVariant();

	EPSignal *signal = (EPSignal *)objectAt(index.row());

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case NameColumn:
			return tr("%1%2").arg(signal->hasChanged() ? "*" : "")
					.arg(signal->name());
		case LengthColumn:
			return APUtils::formattedTime(signal->length(), "%hh %mm %ss");
		}
	} else if (role == Qt::FontRole) {
		if (index.column() == NameColumn) {
			QFont font;
			font.setBold(signal == activeSignal());
			return font;
		}
	}

	return QVariant();
}

bool EPSignalsController::hasActiveSignal()
{
	return EPSignalsController::activeSignal() != NULL;
}

bool EPSignalsController::handleUnsavedSignal(EPSignal *signal)
{
	QMessageBox msgBox(MainWindow::instance());
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("AutoEPG");
	msgBox.setText(signal->name() + " signal has been modified.");
	msgBox.setInformativeText("Do you want to save your changes?");
	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Save);

	int ret = msgBox.exec();
	if (ret == QMessageBox::Cancel) return false;
	else if (ret == QMessageBox::Save) {
		QString filePath = APUtils::runSaveDialog(EPSignalWriter::Epg);
		if (filePath.isEmpty()) return false;

		EPSignalWriter *writer = EPSignalWriter::writer(filePath, signal, EPSignalWriter::Epg);
		writer->start();
		writer->wait(2000);
	}

	return true;
}

QVariant EPSignalsController::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation == Qt::Vertical)
		return QVariant();

	return section == NameColumn ? "Name" : "Length";
}

void EPSignalsController::pushSignal(EPSignal *signal)
{
	shared()->appendSignal(signal);
}

void EPSignalsController::setActiveSignal(EPSignal *signal)
{
	if (_activeSignal == signal)
		return;

	emit currentSignalWillChange(_activeSignal);
	_activeSignal = signal;
	emit currentSignalDidChange(signal);
	emit currentSignalDidChange();

	setCurrentObject(signal);
}

void EPSignalsController::updateActiveObject(QObject *object)
{
	setActiveSignal((EPSignal *)object);
}

QList<EPSignal *> EPSignalsController::unsavedSignals()
{
	QList<EPSignal *> list;
	foreach (QObject *object, objects) {
		EPSignal *signal = (EPSignal *)object;
		if (signal->hasChanged())
			list << signal;
	}
	return list;
}
