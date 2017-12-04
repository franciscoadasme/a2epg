#ifndef EPG_WAVEFORMSET_H
#define EPG_WAVEFORMSET_H

#include <QColor>
#include <QList>
#include <QString>
#include <QUuid>
#include "../identifiable.h"

namespace EPG {
class Waveform;
class WaveformSetStore;

class WaveformSet : public Identifiable
{
public:
    explicit WaveformSet(const QString &name, WaveformSetStore *store);
    ~WaveformSet();

    void add(Waveform *waveform);
    Waveform *at(int index) const;
    int count() const;
    Waveform *createWaveform(const QString &name, const QColor &color);
    Waveform *createWaveform(const QString &name, const QString &description,
                             const QColor &color);
    Waveform *createWaveformAt(int index, const QString &name,
                               const QColor &color);
    Waveform *createWaveformAt(int index, const QString &name,
                               const QString &description, const QColor &color);
    void deleteAt(int index);
    void deleteWaveform(Waveform *waveform);
    Waveform *findByName(const QString &name) const;
    int indexOf(Waveform *waveform) const;
    void insertAt(int index, Waveform *waveform);
    QString name() const;
    bool setName(const QString &name);
    int size() const;
    WaveformSetStore *store() const;
    QList<Waveform *> waveforms() const;

private:
    QString m_name;
    WaveformSetStore *m_store;
    QList<Waveform *> m_waveforms;

    bool isNameValid(const QString &name);
};

QDebug operator <<(QDebug debug, const WaveformSet &set);
}

#endif // EPG_WAVEFORMSET_H
