#include "waveformsetstoreitem.h"

#include "core/epg/waveformset.h"
#include "itemkind.h"
#include "waveformsetitem.h"

WaveformSetStoreItem::WaveformSetStoreItem(EPG::WaveformSetStore *store) :
    TreeItem(),
    m_store(store)
{
    for (int i = 0; i < m_store->size(); ++i)
        insertAt(i, m_store->at(i));
}

bool WaveformSetStoreItem::createChildAt(int index)
{
    EPG::WaveformSet *set = m_store->createSetAt(index, "");
    return insertAt(index, set);
}

QVariant WaveformSetStoreItem::dataAt(int column) const
{
    Q_UNUSED(column);

    return QVariant();
}

QVariant WaveformSetStoreItem::decorationAt(int column) const
{
    Q_UNUSED(column);

    return QVariant();
}

QVariant WaveformSetStoreItem::editDataAt(int column) const
{
    Q_UNUSED(column);

    return QVariant();
}

QFont WaveformSetStoreItem::fontAt(int column) const
{
    Q_UNUSED(column);

    return QFont();
}

int WaveformSetStoreItem::kind() const
{
    return (int)ItemKind::Root;
}

bool WaveformSetStoreItem::insertAt(int index, EPG::WaveformSet *set)
{
    return TreeItem::insertAt(index, new WaveformSetItem(set, this));
}

bool WaveformSetStoreItem::isEditable(int column) const
{
    Q_UNUSED(column);

    return false;
}

void WaveformSetStoreItem::setCurrentSet(int index) const
{
    m_store->setCurrentSet(index);
}

bool WaveformSetStoreItem::setData(int column, const QVariant &value) const
{
    Q_UNUSED(column);
    Q_UNUSED(value);

    return false;
}
