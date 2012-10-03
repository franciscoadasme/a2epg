#include "apworker.h"

APWorker::APWorker(QObject *object, QObject *parent) :
	QThread(parent),
	_object(object)
{
}

QObject *APWorker::object()
{
	return _object;
}
