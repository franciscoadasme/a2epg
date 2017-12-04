#include "waveformitem.h"

#include "core/epg/waveformset.h"
#include "utils/color.h"
#include "itemkind.h"

using namespace Utils;

enum DataColumn {
    Name,
    Description,
    Color
};

WaveformItem::WaveformItem(EPG::Waveform *waveform, TreeItem *parent) :
    TreeItem(parent),
    m_waveform(waveform)
{
}

WaveformItem::~WaveformItem()
{
    m_waveform->set()->deleteWaveform(m_waveform);
}

bool WaveformItem::createChildAt(int index)
{
    Q_UNUSED(index);

    return false;
}

QVariant WaveformItem::dataAt(int column) const
{
    switch (column) {
    case DataColumn::Name:
        return m_waveform->name();
    case DataColumn::Description:
        return m_waveform->description();
    case DataColumn::Color:
        return Color::colorName(m_waveform->color());
    default:
        return QVariant();
    }
}

QVariant WaveformItem::decorationAt(int column) const
{
    switch (column) {
    case DataColumn::Color:
        return m_waveform->color();
    default:
        return QVariant();
    }
}

bool WaveformItem::deleteAt(int index)
{
    Q_UNUSED(index);
    return false;
}

QVariant WaveformItem::editDataAt(int column) const
{
    switch (column) {
    case DataColumn::Color:
        return m_waveform->color();
    default:
        return dataAt(column);
    }
}

QFont WaveformItem::fontAt(int column) const
{
    Q_UNUSED(column);

    return QFont();
}

int WaveformItem::kind() const
{
    return (int)ItemKind::Waveform;
}

bool WaveformItem::insertAt(int index, TreeItem *item)
{
    Q_UNUSED(index);
    Q_UNUSED(item);

    return false;
}

bool WaveformItem::isEditable(int column) const
{
    Q_UNUSED(column);

    return true;
}

bool WaveformItem::setData(int column, const QVariant &value) const
{
    switch (column) {
    case DataColumn::Name:
        return m_waveform->setName(value.toString());
    case DataColumn::Description:
        return m_waveform->setDescription(value.toString());
    case DataColumn::Color:
        return m_waveform->setColor(value.value<QColor>());
    default:
        return false;
    }
}

QSize WaveformItem::sizeHint(int column) const
{
    Q_UNUSED(column);

    return QSize(0, 20);
}

EPG::Waveform *WaveformItem::waveform() const
{
    return m_waveform;
}
