#include "waveformsetitem.h"

#include "core/epg/waveform.h"
#include "core/epg/waveformsetstore.h"
#include "utils/color.h"
#include "itemkind.h"
#include "waveformitem.h"

using namespace Utils;

enum DataColumn {
    Name
};

WaveformSetItem::WaveformSetItem(EPG::WaveformSet *set, TreeItem *parent) :
    TreeItem(parent),
    m_set(set)
{
    for (int i = 0; i < m_set->size(); ++i)
        insertAt(i, m_set->at(i));
}

WaveformSetItem::~WaveformSetItem()
{
    m_set->store()->deleteSet(m_set);
}

bool WaveformSetItem::createChildAt(int index)
{
    // TODO select the farthest color by hue to the existing ones
    // create a new waveform constructor that sets the color automatically
    auto *waveform
        = m_set->createWaveformAt(index, "", "", Color::randomNamedColor());
    return insertAt(index, waveform);
}

QVariant WaveformSetItem::dataAt(int column) const
{
    switch (column) {
    case DataColumn::Name:
        return m_set->name();
    default:
        return QVariant();
    }
}

QVariant WaveformSetItem::decorationAt(int column) const
{
    Q_UNUSED(column);

    return QVariant();
}

QVariant WaveformSetItem::editDataAt(int column) const
{
    return dataAt(column);
}

QFont WaveformSetItem::fontAt(int column) const
{
    QFont font;

    switch (column) {
    case DataColumn::Name:
        font.setBold(m_set->store()->isCurrentSet(m_set));
        break;
    default:
        break;
    }

    return font;
}

int WaveformSetItem::kind() const
{
    return (int)ItemKind::Set;
}

bool WaveformSetItem::insertAt(int index, EPG::Waveform *waveform)
{
    return TreeItem::insertAt(index, new WaveformItem(waveform, this));
}

bool WaveformSetItem::isEditable(int column) const
{
    switch (column) {
    case DataColumn::Name:
        return true;
    default:
        return false;
    }
}

bool WaveformSetItem::setData(int column, const QVariant &value) const
{
    switch (column) {
    case DataColumn::Name:
        return m_set->setName(value.toString());
    default:
        return false;
    }
}

QSize WaveformSetItem::sizeHint(int column) const
{
    Q_UNUSED(column);

    return QSize(0, 28);
}
