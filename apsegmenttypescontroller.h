#ifndef APSEGMENTTYPESCONTROLLER_H
#define APSEGMENTTYPESCONTROLLER_H

#include "apcontroller.h"
#include "apsegmenttype.h"

#define SegmentTypesPersistenceKey "SegmentTypesPersistence"

class APSegmentTypesController : public APController
{
    Q_OBJECT
public:
    static APSegmentTypesController *shared();
    void addSegmentType(APSegmentType *type);
    void addSegmentType(QString name, QColor color);
    void remove();
    bool removable();

    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    static QList<int> typeIds();
    static QList<APSegmentType *> types();
    static APSegmentType *typeForId(int id);
    static APSegmentType *typeAt(int index);
    static int indexOfType(APSegmentType *type);

public slots:
    void savePersistence();

private:
    static APSegmentTypesController *_instance;
    explicit APSegmentTypesController(QObject *parent = 0);

    void loadSegmentTypes();
    static QList<APSegmentType *> defaultSegmentTypes();
    int usableIdentifier();
};

#endif // APSEGMENTTYPESCONTROLLER_H
