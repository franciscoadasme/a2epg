#ifndef SETITEM_H
#define SETITEM_H

#include "core/epg/waveformset.h"
#include "utils/treeitem.h"

namespace EPG {
class Waveform;
}

class WaveformSetItem : public TreeItem
{
public:
    WaveformSetItem(EPG::WaveformSet *set, TreeItem *parent);
    ~WaveformSetItem();

    bool createChildAt(int index) override;
    QVariant dataAt(int column) const override;
    QVariant decorationAt(int column) const override;
    QVariant editDataAt(int column) const override;
    QFont fontAt(int column) const override;
    int kind() const override;
    bool insertAt(int index, EPG::Waveform *waveform);
    bool isEditable(int column) const override;
    bool setData(int column, const QVariant &value) const override;
    QSize sizeHint(int column) const override;

private:
    EPG::WaveformSet *m_set;
};

#endif // SETITEM_H
