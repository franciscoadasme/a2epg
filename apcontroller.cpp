#include "apcontroller.h"

#include <QDebug>

APController::APController(QObject *parent) :
    QAbstractTableModel(parent)
{
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(this);
    proxyModel->setDynamicSortFilter(true);

    this->setObjectName("model");
    proxyModel->setObjectName("proxyModel");

    selectionModel = NULL;
}

void APController::activeIndexDidChange(QModelIndex active)
{
    QModelIndex index = proxyModel->mapToSource(active);
    emit activeObjectDidChanged(objects[index.row()]);
}

void APController::addObject(QObject *object)
{
    beginInsertRows(QModelIndex(), count(), count()); {
        objects << object;
    } endInsertRows();
}

QAbstractItemModel *APController::arrangedObjects()
{
    return proxyModel;
}

int APController::count() const
{
    return objects.count();
}

QObject *APController::currentObject()
{
    if (!selectionModel) return NULL;

    QModelIndex currentIndex = selectionModel->currentIndex();
    if (!currentIndex.isValid())
        return NULL;

    QModelIndex mappedIndex = proxyModel->mapToSource(currentIndex);
    Q_ASSERT_X(mappedIndex.row() >= 0 && mappedIndex.row() < count(),
               "APController::currentObject()",
               "Index out of range");
    return objects[mappedIndex.row()];
}

int APController::indexOfObject(QObject *object) const
{
    return objects.indexOf(object);
}

void APController::itemSelectionDidChange(const QItemSelection &selected,
                                          const QItemSelection &)
{
    if (selected.count() == 0) {
        emit selectionDidChanged(NULL);
        return;
    }

    Q_ASSERT_X(selected.count() == 1,
               "APController::itemSelectionDidChange",
               "Multi-selection detected");

    QItemSelection selectionMappedToSource =
            proxyModel->mapSelectionToSource(selected);
    QModelIndex index = selectionMappedToSource.indexes().first();

    emit selectionDidChanged(objects[index.row()]);
}

QObject *APController::objectAt(int index) const
{
    Q_ASSERT_X(index >= 0 && index < count(),
               "APController::objectAt()",
               "Index out of range");
    return index < 0 || index >= count() ? NULL : objects[index];
}

void APController::remove()
{
    take();
}

int APController::rowCount(const QModelIndex &) const
{
    return objects.count();
}

QObject *APController::selectedObject()
{
    if (!selectionModel->hasSelection()) return NULL;
    QModelIndexList indices = selectionModel->selectedRows();
    Q_ASSERT_X(indices.count() == 1,
               "APController::remove()",
               "Multi-selection detected");

    QModelIndex index = indices.first();
    return objects[index.row()];
}

void APController::setCurrentObject(QObject *object)
{
    int row = objects.indexOf(object);
    QModelIndex modelIndex = index(row, 0, QModelIndex());
    QModelIndex mappedIndex = proxyModel->mapFromSource(modelIndex);
    selectionModel->setCurrentIndex(mappedIndex,
                                    QItemSelectionModel::ClearAndSelect
                                    | QItemSelectionModel::Rows);
}

void APController::setSelectedObject(QObject *object)
{
    int row = objects.indexOf(object);
    QModelIndex modelIndex = index(row, 0, QModelIndex());
    QModelIndex mappedIndex = proxyModel->mapFromSource(modelIndex);
    selectionModel->select(mappedIndex,
                           QItemSelectionModel::ClearAndSelect
                           | QItemSelectionModel::Rows);
}

void APController::setSelectionModel(QItemSelectionModel *model)
{
    if (selectionModel)
        selectionModel->disconnect(this);

    selectionModel = model;
    if (selectionModel)
        connect(selectionModel,
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                SLOT(itemSelectionDidChange(QItemSelection,QItemSelection)));
}

QObject *APController::take()
{
    QModelIndex index;
    if (selectionModel->hasSelection()) {
        QModelIndexList indices = selectionModel->selectedRows();
        Q_ASSERT_X(indices.count() == 1,
                   "APController::remove()",
                   "Multi-selection detected");
        index = indices.first();
    } else {
        index = selectionModel->currentIndex();
        if (!index.isValid())
            return NULL;
    }

    index = proxyModel->mapToSource(index);
    QObject *object = objectAt(index.row());

    beginRemoveRows(QModelIndex(), index.row(), index.row()); {
        objects.removeAt(index.row());
    } endRemoveRows();

    return object;
}
