#ifndef EPSIGNALSCONTROLLER_H
#define EPSIGNALSCONTROLLER_H

#include "apcontroller.h"
#include "epsignal.h"

class EPSignalsController : public APController
{
    Q_OBJECT
public:
	static EPSignalsController *shared();

	static bool hasActiveSignal();
	static EPSignal *activeSignal();
	static void pushSignal(EPSignal *signal);

	int columnCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	QList<EPSignal *> unsavedSignals();

signals:
	void currentSignalDidChange(EPSignal *);
	void currentSignalDidChange();
	void currentSignalWillChange(EPSignal *);

public slots:
	void updateActiveObject(QObject *object);
	void setActiveSignal(EPSignal *signal);
	void closeSignal();

protected:
	explicit EPSignalsController(QObject *parent = 0);
	EPSignalsController(const EPSignalsController &);
	EPSignalsController &operator= (const EPSignalsController &);

private:
	EPSignal *_activeSignal;

	void appendSignal(EPSignal *signal);
	bool handleUnsavedSignal(EPSignal *signal);

	static EPSignalsController *_shared;
};

#endif // EPSIGNALSCONTROLLER_H
