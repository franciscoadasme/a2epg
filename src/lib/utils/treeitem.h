#ifndef TREEITEM_H
#define TREEITEM_H

#include <QFont>
#include <QList>
#include <QSize>

namespace Utils {
class TreeItem
{
public:
    TreeItem(TreeItem *parent = nullptr);
    virtual ~TreeItem();

    bool add(TreeItem *item);
    TreeItem *at(int index) const;
    bool contains(TreeItem *item) const;
    int count() const;
    virtual bool createChildAt(int index) = 0;
    virtual QVariant dataAt(int column) const = 0;
    virtual QVariant decorationAt(int column) const = 0;
    bool deleteAt(int index);
    virtual QVariant editDataAt(int column) const = 0;
    virtual QFont fontAt(int column) const = 0;
    virtual int kind() const = 0;
    int indexOf(TreeItem *item) const;
    bool insertAt(int index, TreeItem *item);
    virtual bool isEditable(int column) const = 0;
    TreeItem *parent() const;
    int row() const;
    virtual bool setData(int column, const QVariant &value) const = 0;
    int size() const;
    virtual QSize sizeHint(int column) const;

protected:
    QList<TreeItem *> m_children;
    TreeItem *m_parent;
};
}

#endif // TREEITEM_H
