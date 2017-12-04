#include "waveformtreemodel.h"

#include <QFont>
#include <QSize>
#include "utils/color.h"
#include "datacolumn.h"
#include "itemkind.h"

#define DATA_PLACEHOLDER_NONE "<none>"

using namespace Utils;

WaveformTreeModel::WaveformTreeModel(EPG::WaveformSetStore *store,
                                     QObject *parent) :
    QAbstractItemModel(parent)
{
    m_rootItem = new WaveformSetStoreItem(store);
}

TreeItem *WaveformTreeModel::at(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_rootItem;
    return static_cast<TreeItem *>(index.internalPointer());
}

void WaveformTreeModel::changeCurrentSetToItemAt(const QModelIndex &index) const
{
    if (typeOfItemAt(index) != ItemKind::Set)
        return;

    m_rootItem->setCurrentSet(index.row());
}

int WaveformTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return DATA_COLUMN_COUNT_WAVEFORM;
}

QVariant WaveformTreeModel::data(const QModelIndex &index, int role) const
{
    TreeItem *item = at(index);
    if ((item->kind() == ItemKind::Root) || (item->kind() == ItemKind::Unknown))
        return QVariant();

    int column = index.column();

    switch (role) {
    case Qt::DecorationRole:
        return item->decorationAt(column);
    case Qt::DisplayRole:
        return displayValue(item->dataAt(column));
    case Qt::EditRole:
        return item->editDataAt(column);
    case Qt::FontRole:
        return item->fontAt(column);
    case Qt::SizeHintRole:
        return item->sizeHint(column);
    default:
        return QVariant();
    }
}

QVariant WaveformTreeModel::displayValue(const QVariant &value) const
{
    switch (value.type()) {
    case QVariant::String:
    {
        QString text = value.toString();
        return text.isEmpty() ? DATA_PLACEHOLDER_NONE : text;
    }
    default:
        return value;
    }
}

Qt::ItemFlags WaveformTreeModel::flags(const QModelIndex &index) const
{
    if (typeOfItemAt(index) == ItemKind::Root)
        return 0;

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (at(index)->isEditable(index.column()))
        flags = Qt::ItemIsEditable | flags;
    return flags;
}

QVariant WaveformTreeModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const
{
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
        return QVariant();

    switch (section) {
    case DataColumn::Name:
        return "Name";
    case DataColumn::Description:
        return "Description";
    case DataColumn::Color:
        return "Color";
    default:
        return QVariant();
    }
}

QModelIndex WaveformTreeModel::index(int row, int column,
                                     const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *item = at(parent)->at(row);
    if (item != nullptr)
        return createIndex(row, column, item);
    return QModelIndex();
}

bool WaveformTreeModel::insertRows(int row, int count,
                                   const QModelIndex &parent)
{
    if (typeOfItemAt(parent) == ItemKind::Waveform)
        return false;

    int startPos = rowCount(parent);
    beginRemoveRows(parent, startPos, startPos + count - 1);
    for (int i = 0; i < count; ++i)
        at(parent)->createChildAt(row + i);
    endRemoveRows();

    return true;
}

QModelIndex WaveformTreeModel::parent(const QModelIndex &index) const
{
    if ((typeOfItemAt(index) == ItemKind::Root)
        || (typeOfItemAt(index) == ItemKind::Unknown))
        return QModelIndex();

    TreeItem *item = at(index)->parent();
    if (item == m_rootItem)
        return QModelIndex();
    return createIndex(item->row(), 0, item);
}

bool WaveformTreeModel::removeRows(int row, int count,
                                   const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i)
        at(parent)->deleteAt(row + i);
    endRemoveRows();

    return true;
}

int WaveformTreeModel::rowCount(const QModelIndex &parent) const
{
    return at(parent)->count();
}

bool WaveformTreeModel::setData(const QModelIndex &index, const QVariant &value,
                                int role)
{
    if (role != Qt::EditRole)
        return false;

    bool result = at(index)->setData(index.column(), value);
    if (result)
        emit dataChanged(index, index);
    return result;
}

int WaveformTreeModel::typeOfItemAt(const QModelIndex &index) const
{
    TreeItem *item = at(index);
    if (item != nullptr)
        return item->kind();
    return ItemKind::Unknown;
}
