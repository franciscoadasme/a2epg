#include "apedithandler.h"

#include <QDebug>
#include "aputils.h"
#include "mainwindow.h"

#define ActionSeparator ";"
#define InfoSeparator "::"

APEditHandler::APEditHandler(EPSProfile *theProfile, QObject *parent) :
    QObject(parent)
{
    profile = theProfile;
    isGrouped = false;
    current = -1;
}

APEditHandler::EditAction APEditHandler::actionFromString(QString action)
{
    if (action == "Create") return Create;
    if (action == "Update") return Update;
    if (action == "Remove") return Remove;
    return None;
}

void APEditHandler::beginGroup()
{
    isGrouped = true;
}

void APEditHandler::endGroup()
{
    isGrouped = false;
    history << QString(tmp.join(ActionSeparator));
    tmp.clear();

    current++;

    emit redoLimitReached(current == history.count() - 1);
    emit undoLimitReached(current < 0);

    _print("endGroup");
}

bool APEditHandler::hasReachedRedoLimit()
{
    return current == history.count() - 1;
}

bool APEditHandler::hasReachedUndoLimit()
{
    return current < 0;
}

void APEditHandler::move(Direction direction)
{
    if (direction == Forward) current++;

    QString joinedActions = history[current];
    QStringList actions = joinedActions.split(ActionSeparator);

    foreach (QString serializedAction, actions) {
        QStringList tokens = serializedAction.split(InfoSeparator);
        EditAction action = actionFromString(tokens.first());
        QString info = tokens.last();

        EPSegment *segment;

        switch (action) {
        case Create: // info is just a serialized segment
        case Remove: {
            segment = EPSegment::unserialize(info);
            if ((direction == Forward && action == Create)
                || (direction == Backward && action == Remove))
                profile->addSegment(segment, true);
            else
                profile->removeSegmentWithId(segment->id(), true);
            break;
        }

        case Update: { // info has format: id:property=prev-next
            QStringList idAndChange = info.split(":");
            int id = idAndChange.first().toInt();

            segment = profile->segmentWithId(id);
            if (!segment) {
                qCritical("APEditHandler::move() => no segment found for "
                          "requested id %i",
                          id);
            }
            segment->pushSerializedChange(idAndChange.last(), direction);
            break;
        }
        case None: break;
        }
    }

    if (direction == Backward) current--;

    profile->forceEmitOfSegmentsDidChange();
    // FIX to force overlay to update
    MainWindow::instance()->epsignalWidget()->overlay()->forceToUpdateGeometry();

    emit redoLimitReached(current == history.count() - 1);
    emit undoLimitReached(current < 0);

    _print("move");
}

void APEditHandler::redo()
{
    move(Forward);
}

APEditHandler *APEditHandler::saveAction(EditAction action, QString info)
{
    /* when adding a new action, we need to erase newer actions */
    int countOfActionsToRemove = history.count() - 1 - current;
    for (int i = 0; i < countOfActionsToRemove; i++)
        history.removeLast();

    tmp << stringWithAction(action) + InfoSeparator + info;

    /* when grouped, wait to endGroup call... force save otherwise */
    if (!isGrouped)
        endGroup();

    return this;
}

QString APEditHandler::stringWithAction(EditAction action)
{
    switch (action) {
    case Create: return "Create";
    case Update: return "Update";
    case Remove: return "Remove";
    default: return QString();
    }
}

void APEditHandler::undo()
{
    move(Backward);
}

void APEditHandler::_print(QString methodname)
{
    QStringList list;
    for (int i = 0; i < history.count(); i++)
        list << (current == i ? ">" : "") + history.at(i);
    qDebug() << "APEditHandler::" + methodname + "() =>" << list << current;
}
