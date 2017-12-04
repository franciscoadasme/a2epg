#ifndef WAVEFORMVIEWCONTROLLER_H
#define WAVEFORMVIEWCONTROLLER_H

#include <QMenu>
#include <QObject>
#include <QTreeView>

class WaveformTreeModel;

class WaveformViewController : public QObject
{
    Q_OBJECT
public:
    explicit WaveformViewController(QTreeView *treeView);

signals:

private slots:
    void changeCurrentSetToSelectedItem();
    void createChildForSelectedItem();
    void createRootChildItem();
    void deleteSelectedItem();
    void showContextMenu(const QPoint &point);

private:
    QTreeView *m_treeView;
    WaveformTreeModel *m_model;

    bool askToDeleteItemAt(const QModelIndex &index) const;
    void createChildItem(const QModelIndex &parent) const;
    bool eventFilter(QObject *watched, QEvent *event);
    QModelIndex selectedIndex() const;
    void setupRootMenu(QMenu *menu) const;
    void setupUi();
    void setupWaveformMenu(QMenu *menu) const;
    void setupWaveformSetMenu(QMenu *menu) const;
};

#endif // WAVEFORMVIEWCONTROLLER_H
