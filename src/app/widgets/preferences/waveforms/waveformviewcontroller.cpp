#include "waveformviewcontroller.h"

#include <QDebug>
#include <QHeaderView>
#include <QItemEditorFactory>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include "core/epg/waveformsetstore.h"
#include "editors/colorlisteditor.h"
#include "datacolumn.h"
#include "itemkind.h"
#include "waveformtreemodel.h"

WaveformViewController::WaveformViewController(QTreeView *treeView) : QObject(),
    m_treeView(treeView)
{
    m_model = new WaveformTreeModel(EPG::WaveformSetStore::sharedInstance());
    m_treeView->setModel(m_model);

    setupUi();
}

bool WaveformViewController::askToDeleteItemAt(const QModelIndex &index) const
{
    QString name = m_model->data(index).toString();
    QString text;
    if (m_model->typeOfItemAt(index) == ItemKind::Waveform) {
        QString parentName = m_model->data(index.parent()).toString();
        text = tr("Do you want to delete the %1 waveform from the %2 set?")
               .arg(name).arg(parentName);
    } else {
        text = tr("Do you want to delete the %1 set?").arg(name);
    }

    QMessageBox dialog;
    dialog.setWindowTitle("Delete item");
    dialog.setText(text);
    dialog.setInformativeText("This change cannot be undone");
    dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dialog.setDefaultButton(QMessageBox::No);
    dialog.setMinimumSize(dialog.minimumSizeHint());

    if (dialog.exec() == QMessageBox::Yes)
        return true;

    return false;
}

void WaveformViewController::changeCurrentSetToSelectedItem()
{
    m_model->changeCurrentSetToItemAt(selectedIndex());
}

void WaveformViewController::createChildItem(const QModelIndex &parent) const
{
    if (!m_model->insertRow(m_model->rowCount(parent), parent))
        return;

    auto childIndex = m_model->index(m_model->rowCount(parent) - 1, 0, parent);
    m_treeView->selectionModel()->setCurrentIndex(
        childIndex,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_treeView->setExpanded(parent, true);
    m_treeView->setFocus();
}

void WaveformViewController::createChildForSelectedItem()
{
    createChildItem(selectedIndex());
}

void WaveformViewController::createRootChildItem()
{
    createChildItem(QModelIndex());
}

void WaveformViewController::deleteSelectedItem()
{
    QModelIndex index = selectedIndex();
    if (m_model->typeOfItemAt(index) == ItemKind::Root)
        return;

    if (askToDeleteItemAt(index))
        m_model->removeRow(index.row(), index.parent());
}

bool WaveformViewController::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched != m_treeView) || (event->type() != QEvent::KeyPress))
        return false;

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->matches(QKeySequence::Delete)
        || (keyEvent->key() == Qt::Key_Backspace))
        deleteSelectedItem();

    return false;
}

QModelIndex WaveformViewController::selectedIndex() const
{
    QModelIndexList indexes = m_treeView->selectionModel()->selectedRows();
    Q_ASSERT_X(indexes.count() <= 1, "selectedIndex", "multiple selection");
    return indexes.isEmpty() ? QModelIndex() : indexes.first();
}

void WaveformViewController::setupRootMenu(QMenu *menu) const
{
    menu->addAction("New Waveform Set", this,
                    &WaveformViewController::createRootChildItem);
}

void WaveformViewController::setupUi()
{
    m_treeView->installEventFilter(this);
    m_treeView->expandAll();
    m_treeView->setColumnWidth(DataColumn::Name, 80);
    m_treeView->setColumnWidth(DataColumn::Description, 200);
    m_treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    m_treeView->header()->setSectionsMovable(false);

    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(showContextMenu(QPoint)));

    // install custom color editor
    auto delegate
        = static_cast<QStyledItemDelegate *>(m_treeView->itemDelegate());
    QItemEditorFactory *factory = new QItemEditorFactory();
    QItemEditorCreatorBase *colorListEditor
        = new QStandardItemEditorCreator<ColorListEditor>();
    factory->registerEditor(QVariant::Color, colorListEditor);
    delegate->setItemEditorFactory(factory);
}

void WaveformViewController::setupWaveformMenu(QMenu *menu) const
{
    menu->addAction("Delete", this, &WaveformViewController::deleteSelectedItem,
                    QKeySequence::Delete);
}

void WaveformViewController::setupWaveformSetMenu(QMenu *menu) const
{
    menu->addAction("New Waveform", this,
                    &WaveformViewController::createChildForSelectedItem);
    menu->addAction("Make Active", this,
                    &WaveformViewController::changeCurrentSetToSelectedItem);
    menu->addSeparator();
    menu->addAction("Delete", this, &WaveformViewController::deleteSelectedItem,
                    QKeySequence::Delete);
}

void WaveformViewController::showContextMenu(const QPoint &point)
{
    QModelIndex index = m_treeView->indexAt(point);

    QMenu menu(m_treeView);
    switch (m_model->typeOfItemAt(index)) {
    case ItemKind::Root:
        setupRootMenu(&menu);
        break;
    case ItemKind::Set:
        setupWaveformSetMenu(&menu);
        break;
    case ItemKind::Waveform:
        setupWaveformMenu(&menu);
        break;
    default:
        break;
    }

    menu.exec(m_treeView->viewport()->mapToGlobal(point));
}
