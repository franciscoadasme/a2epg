#include "waveformsetstore.h"

#include <QDebug>
#include <QColor>
#include "waveformset.h"

namespace EPG {
WaveformSetStore *WaveformSetStore::instance = nullptr;

WaveformSetStore::WaveformSetStore()
{
    m_currentSet = nullptr;
}

WaveformSetStore::~WaveformSetStore()
{
    qDeleteAll(m_sets);
    m_sets.clear();
}

void WaveformSetStore::add(WaveformSet *set)
{
    insertAt(size(), set);
}

WaveformSet *WaveformSetStore::at(int index) const
{
    if ((index >= 0) && (index < m_sets.size()))
        return m_sets.at(index);

    return nullptr;
}

int WaveformSetStore::count() const
{
    return size();
}

WaveformSet *WaveformSetStore::createDefaultSet()
{
    WaveformSet *set = createSet("Standard");
    set->createWaveform("Np", "Non-probing", QColor("gray"));
    set->createWaveform(
        "C", "Intercellular stylet penetration/pathway phase", QColor("black"));
    set->createWaveform("E1", "Phloem salivation", QColor("cyan"));
    set->createWaveform("E2", "Phloem ingestion", QColor(Qt::darkCyan));
    set->createWaveform("F", "Mechanical work", QColor("goldenrod"));
    set->createWaveform("G", "Xylem ingestion", QColor("blue"));
    set->createWaveform("Pd", "Intracellular punctures", QColor("red"));
    return set;
}

WaveformSet *WaveformSetStore::createSet(const QString &name)
{
    return createSetAt(size(), name);
}

WaveformSet *WaveformSetStore::createSetAt(int index, const QString &name)
{
    WaveformSet *set = new WaveformSet(name, this);
    insertAt(index, set);

    if (m_currentSet == nullptr)
        setCurrentSet(set);

    return set;
}

WaveformSet *WaveformSetStore::currentSet() const
{
    return m_currentSet;
}

void WaveformSetStore::deleteAt(int index)
{
    deleteSet(at(index));
}

void WaveformSetStore::deleteSet(WaveformSet *set)
{
    if (set == nullptr)
        return;

    if (m_currentSet == set) {
        int index = m_sets.indexOf(set);
        setCurrentSet(at(index > 0 ? index - 1 : 1));
    }

    m_sets.removeOne(set);
    delete set;
}

WaveformSet *WaveformSetStore::findByName(const QString &name) const
{
    foreach(WaveformSet * set, m_sets) {
        if (set->name() == name)
            return set;
    }

    return nullptr;
}

int WaveformSetStore::indexOf(WaveformSet *set) const
{
    return m_sets.indexOf(set);
}

void WaveformSetStore::insertAt(int index, WaveformSet *set)
{
    m_sets.insert(index, set);
}

bool WaveformSetStore::isCurrentSet(int index) const
{
    return isCurrentSet(at(index));
}

bool WaveformSetStore::isCurrentSet(WaveformSet *set) const
{
    return m_currentSet == set;
}

void WaveformSetStore::restoreSets()
{
    createDefaultSet();
}

void WaveformSetStore::setCurrentSet(int index)
{
    WaveformSet *set = at(index);
    if (set != nullptr)
        setCurrentSet(set);
}

void WaveformSetStore::setCurrentSet(WaveformSet *set)
{
    m_currentSet = set;
}

WaveformSetStore *WaveformSetStore::sharedInstance()
{
    if (instance == nullptr) {
        instance = new WaveformSetStore();
        instance->restoreSets();
    }

    return instance;
}

int WaveformSetStore::size() const
{
    return m_sets.size();
}

QList<WaveformSet *> WaveformSetStore::sets() const
{
    return m_sets;
}
}
