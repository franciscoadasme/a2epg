#ifndef WAVEFORMITEM_H
#define WAVEFORMITEM_H

#include "core/epg/waveform.h"
#include "utils/treeitem.h"

class WaveformItem : public TreeItem
{
public:
    WaveformItem(EPG::Waveform *waveform, TreeItem *parent);
    ~WaveformItem();

    bool createChildAt(int index) override;
    QVariant dataAt(int column) const override;
    QVariant decorationAt(int column) const override;
    bool deleteAt(int index);
    QVariant editDataAt(int column) const override;
    QFont fontAt(int column) const override;
    int kind() const override;
    bool insertAt(int index, TreeItem *item);
    bool isEditable(int column) const override;
    bool setData(int column, const QVariant &value) const override;
    QSize sizeHint(int column) const override;
    EPG::Waveform *waveform() const;

private:
    EPG::Waveform *m_waveform;
};

#endif // WAVEFORMITEM_H
