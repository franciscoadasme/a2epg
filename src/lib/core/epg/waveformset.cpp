#include "waveformset.h"

#include <QDebug>
#include "waveform.h"
#include "waveformsetstore.h"

namespace EPG {
WaveformSet::WaveformSet(const QString &name, WaveformSetStore *store) :
    Identifiable(),
    m_name(name),
    m_store(store)
{
}

WaveformSet::~WaveformSet()
{
    qDebug() << "WaveformSet::~WaveformSet(" << name() << ")";
    qDeleteAll(m_waveforms);
    m_waveforms.clear();
}

void WaveformSet::add(Waveform *waveform)
{
    insertAt(size(), waveform);
}

Waveform *WaveformSet::at(int index) const
{
    if ((index >= 0) && (index < m_waveforms.size()))
        return m_waveforms.at(index);

    return nullptr;
}

int WaveformSet::count() const
{
    return size();
}

Waveform *WaveformSet::createWaveform(const QString &name, const QColor &color)
{
    return createWaveformAt(size(), name, color);
}

Waveform *WaveformSet::createWaveform(const QString &name,
                                      const QString &description,
                                      const QColor &color)
{
    return createWaveformAt(size(), name, description, color);
}

Waveform *WaveformSet::createWaveformAt(int index, const QString &name,
                                        const QColor &color)
{
    Waveform *waveform = new Waveform(name, color, this);
    insertAt(index, waveform);
    return waveform;
}

Waveform *WaveformSet::createWaveformAt(int index, const QString &name,
                                        const QString &description,
                                        const QColor &color)
{
    Waveform *waveform = new Waveform(name, description, color, this);
    insertAt(index, waveform);
    return waveform;
}

void WaveformSet::deleteAt(int index)
{
    deleteWaveform(at(index));
}

void WaveformSet::deleteWaveform(Waveform *waveform)
{
    if (waveform == nullptr)
        return;

    m_waveforms.removeOne(waveform);
    delete waveform;
}

Waveform *WaveformSet::findByName(const QString &name) const
{
    foreach(Waveform * waveform, m_waveforms) {
        if (waveform->name() == name)
            return waveform;
    }

    return nullptr;
}

int WaveformSet::indexOf(Waveform *waveform) const
{
    return m_waveforms.indexOf(waveform);
}

void WaveformSet::insertAt(int index, Waveform *waveform)
{
    m_waveforms.insert(index, waveform);
}

bool WaveformSet::isNameValid(const QString &name)
{
    // TODO add minimum length validation
    WaveformSet *other = m_store->findByName(name);
    return (other == nullptr) || (other == this);
}

QString WaveformSet::name() const
{
    return m_name;
}

bool WaveformSet::setName(const QString &name)
{
    if ((m_name == name) || !isNameValid(name))
        return false;

    m_name = name;
    return true;
}

int WaveformSet::size() const
{
    return m_waveforms.size();
}

WaveformSetStore *WaveformSet::store() const
{
    return m_store;
}

QList<Waveform *> WaveformSet::waveforms() const
{
    return m_waveforms;
}

QDebug operator <<(QDebug dbg, const WaveformSet &set)
{
    QDebugStateSaver stateSaver(dbg);

    dbg.nospace() << "EPG::WaveformSet<" << set.id() << ">("
                  << set.name() << ")["
                  << set.size() << "]";
    return dbg.maybeSpace();
}
}
