#include "apsegmenttypescontroller.h"

#include <QSettings>
#include "aputils.h"
#include <QColor>

#define ColorColumn 0
#define NameColumn 1

#define UserIdentifier 1000

APSegmentTypesController *APSegmentTypesController::_instance = NULL;

APSegmentTypesController::APSegmentTypesController(QObject *parent) :
    APController(parent)
{
    loadSegmentTypes();
}

APSegmentTypesController *APSegmentTypesController::shared()
{
    if (_instance == NULL)
        _instance = new APSegmentTypesController();
    return _instance;
}

void APSegmentTypesController::addSegmentType(APSegmentType *type)
{
    connect(type, SIGNAL(propertyDidChanged()), SLOT(savePersistence()));
    addObject(type);

    savePersistence();
}

void APSegmentTypesController::addSegmentType(QString name, QColor color)
{
    addSegmentType(new APSegmentType(usableIdentifier(), name, color));
}

int APSegmentTypesController::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant APSegmentTypesController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= count())
        return QVariant();

    APSegmentType *type = (APSegmentType *)objectAt(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (index.column() == NameColumn)
            return type->name();
        break;
    case Qt::FontRole:
        if (index.column() == NameColumn) {
            QFont font;
            font.setBold(type->id() < UserIdentifier);
            return font;
        }
        break;
    case Qt::ToolTipRole:
        return type->id() < UserIdentifier ? "Default type" : "Created by user";
    case Qt::DecorationRole:
        if (index.column() == ColorColumn)
            return type->color();
    }

    return QVariant();
}

QList<APSegmentType *> APSegmentTypesController::defaultSegmentTypes()
{
    QList<APSegmentType *> types;
    foreach (SegmentType type, APUtils::segmentTypes()) {
        types << new APSegmentType(type,
                                   APUtils::stringWithSegmentType(type),
                                   APUtils::colorForSegmentType(type));
    }
    return types;
}

Qt::ItemFlags APSegmentTypesController::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    APSegmentType *type = (APSegmentType *)objectAt(index.row());
    if (type->id() > UserIdentifier && index.column() == NameColumn)
        flags = flags | Qt::ItemIsEditable;

    return flags;
}

QVariant APSegmentTypesController::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
    if (role != Qt::DisplayRole || orientation == Qt::Vertical)
        return QVariant();

    return section == NameColumn ? "Name" : "Color";
}

int APSegmentTypesController::indexOfType(APSegmentType *type)
{
    return shared()->objects.indexOf(type);
}

void APSegmentTypesController::loadSegmentTypes()
{
    QList<APSegmentType *> types;
    bool shouldSave = false;

    QSettings settings;
    if (settings.contains(SegmentTypesPersistenceKey)) {
        QString representation = settings.value(SegmentTypesPersistenceKey)
                                 .toString();
        QStringList typeTokens = representation.split(",");
        foreach (QString typeToken, typeTokens)
            types << APSegmentType::unserialize(typeToken);
    } else {
        types = APSegmentTypesController::defaultSegmentTypes();
        shouldSave = true;
    }

    foreach (APSegmentType *type, types)
        addSegmentType(type);

    if (shouldSave)
        savePersistence();
}

bool APSegmentTypesController::removable()
{
    APSegmentType *type = (APSegmentType *)selectedObject();
    return type != NULL && type->id() > UserIdentifier;
}

void APSegmentTypesController::remove()
{
    APController::remove();
    savePersistence();
}

void APSegmentTypesController::savePersistence()
{
    QStringList list;
    foreach (QObject *obj, objects) {
        list << ((APSegmentType *)obj)->serialize();
    }

    QSettings settings;
    settings.setValue(SegmentTypesPersistenceKey, list.join(", "));
}

bool APSegmentTypesController::setData(const QModelIndex &index,
                                       const QVariant &value,
                                       int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        APSegmentType *type = (APSegmentType *)objectAt(index.row());

        switch (index.column()) {
        case NameColumn:
            type->setName(value.toString());
            break;
        case ColorColumn:
            type->setColor(value.value<QColor>());
            break;
        }

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

APSegmentType *APSegmentTypesController::typeAt(int index)
{
    return (APSegmentType *)shared()->objects[index];
}

APSegmentType *APSegmentTypesController::typeForId(int id)
{
    foreach (QObject *obj, shared()->objects) {
        APSegmentType *type = (APSegmentType *)obj;
        if (type->id() == id)
            return type;
    }
    return new APSegmentType(APNotFound, "Unknown", Qt::darkGray);
}

QList<int> APSegmentTypesController::typeIds()
{
    QList<int> list;
    foreach (QObject *obj, shared()->objects)
        list << ((APSegmentType *)obj)->id();
    return list;
}

QList<APSegmentType *> APSegmentTypesController::types()
{
    QList<APSegmentType *> list;
    foreach (QObject *obj, shared()->objects)
        list << (APSegmentType *)obj;
    return list;
}

int APSegmentTypesController::usableIdentifier()
{
    APSegmentType *lastType = (APSegmentType *)objectAt(count() - 1);

    bool createdByUser = lastType->id() > UserIdentifier;
    int identifier = createdByUser ? lastType->id() + 1 : UserIdentifier + 1;
    return identifier;
}
