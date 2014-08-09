#ifndef APEDITHANDLER_H
#define APEDITHANDLER_H

#include <QObject>

#include "epsprofile.h"
#include "epsegment.h"
#include "APGlobals.h"

class EPSProfile;

class APEditHandler : public QObject
{
    Q_OBJECT
public:
    explicit APEditHandler(EPSProfile *theProfile, QObject *parent = 0);

    enum EditAction {
        Create,
        Update,
        Remove,
        None
    };

    void beginGroup();
    void endGroup();
    APEditHandler *saveAction(EditAction action, QString info);

    bool hasReachedUndoLimit();
    bool hasReachedRedoLimit();

signals:
    void undoLimitReached(bool);
    void redoLimitReached(bool);

public slots:
    void undo();
    void redo();

private:
    QStringList history;
    EPSProfile *profile;
    QStringList tmp;
    bool isGrouped;
    int current;

    QString stringWithAction(EditAction action);
    EditAction actionFromString(QString action);
    void move(Direction direction);

    void _print(QString methodname);
};

#endif // APEDITHANDLER_H
