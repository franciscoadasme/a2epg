#ifndef EPGWAVEFORMTREEMODEL_H
#define EPGWAVEFORMTREEMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include "utils/treeitem.h"
#include "waveformsetstoreitem.h"

namespace EPG {
class WaveformSetStore;
}

class WaveformTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit WaveformTreeModel(EPG::WaveformSetStore *store,
                               QObject *parent = nullptr);

    void changeCurrentSetToItemAt(const QModelIndex &index) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role) override;
    int typeOfItemAt(const QModelIndex &index) const;

private:
    WaveformSetStoreItem *m_rootItem;

    Utils::TreeItem *at(const QModelIndex &index) const;
    QVariant displayValue(const QVariant &value) const;
};

#endif // EPGWAVEFORMTREEMODEL_H
