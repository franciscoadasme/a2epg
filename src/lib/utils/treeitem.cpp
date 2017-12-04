#include "treeitem.h"

TreeItem::TreeItem(TreeItem *parent) : m_parent(parent)
{
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_children);
    m_children.clear();
}

bool TreeItem::add(TreeItem *item)
{
    return insertAt(m_children.size(), item);
}

TreeItem *TreeItem::at(int index) const
{
    if ((index >= 0) && (index < m_children.size()))
        return m_children.at(index);

    return nullptr;
}

bool TreeItem::contains(TreeItem *item) const
{
    return m_children.contains(item);
}

int TreeItem::count() const
{
    return size();
}

bool TreeItem::deleteAt(int index)
{
    TreeItem *item = at(index);
    delete item;
    m_children.removeAt(index);
    return true;
}

int TreeItem::indexOf(TreeItem *item) const
{
    return m_children.indexOf(item);
}

bool TreeItem::insertAt(int index, TreeItem *item)
{
    if (contains(item))
        return false;

    m_children.insert(index, item);
    return true;
}

TreeItem *TreeItem::parent() const
{
    return m_parent;
}

int TreeItem::row() const
{
    if (m_parent == nullptr)
        return -1;
    return m_parent->indexOf((TreeItem *)this);
}

int TreeItem::size() const
{
    return m_children.size();
}

QSize TreeItem::sizeHint(int column) const
{
    Q_UNUSED(column);

    return QSize();
}
