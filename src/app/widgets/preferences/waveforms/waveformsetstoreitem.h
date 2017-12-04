#ifndef STOREITEM_H
#define STOREITEM_H

#include "core/epg/waveformsetstore.h"
#include "utils/treeitem.h"

namespace EPG {
class WaveformSet;
}

class WaveformSetStoreItem : public TreeItem
{
public:
    WaveformSetStoreItem(EPG::WaveformSetStore *store);

    bool createChildAt(int index) override;
    QVariant dataAt(int column) const override;
    QVariant decorationAt(int column) const override;
    QVariant editDataAt(int column) const override;
    QFont fontAt(int column) const override;
    int kind() const override;
    bool insertAt(int index, EPG::WaveformSet *set);
    bool isEditable(int column) const override;
    void setCurrentSet(int index) const;
    bool setData(int column, const QVariant &value) const override;

private:
    EPG::WaveformSetStore *m_store;
};

#endif // STOREITEM_H
