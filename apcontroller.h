#ifndef APCONTROLLER_H
#define APCONTROLLER_H

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>

class APController : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit APController(QObject *parent = 0);

	QAbstractItemModel *arrangedObjects();
	void setSelectionModel(QItemSelectionModel *model);
	QObject *selectedObject();
	QObject *currentObject();

	int rowCount(const QModelIndex &) const;

	void addObject(QObject *object);
	QObject *objectAt(int index) const;
	int indexOfObject(QObject *object) const;
	int count() const;
	QObject *take();

signals:
	void selectionDidChanged(QObject *);
	void activeObjectDidChanged(QObject *);

public slots:
	void activeIndexDidChange(QModelIndex active);
	void setSelectedObject(QObject *object);
	void setCurrentObject(QObject *object);
	void remove();

private slots:
	void itemSelectionDidChange(const QItemSelection & selected, const QItemSelection & deselected);

protected:
	QList<QObject *> objects;
	QItemSelectionModel *selectionModel;

private:
	QSortFilterProxyModel *proxyModel;
};

#endif // APCONTROLLER_H
