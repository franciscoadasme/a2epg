#ifndef APWORKER_H
#define APWORKER_H

#include <QThread>

class APWorker : public QThread
{
    Q_OBJECT
public:
    explicit APWorker(QObject *object, QObject *parent = 0);

    QObject *object();

signals:
    void workLengthDidChange(int);
    void workDidBegin();
    void workDidBegin(QString);
    void workDidEnd();
    void workDidEnd(bool, QObject *, QString);
    void workThrowsMessage(QString);
    void progressDidChange(int);

private:
    QObject *_object;

};

#endif // APWORKER_H
