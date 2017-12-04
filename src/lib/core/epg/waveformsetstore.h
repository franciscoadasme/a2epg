#ifndef EPG_WAVEFORMSETSTORE_H
#define EPG_WAVEFORMSETSTORE_H

#include <QList>

namespace EPG {
class WaveformSet;

class WaveformSetStore
{
public:
    WaveformSetStore();
    ~WaveformSetStore();

    static WaveformSetStore *sharedInstance();

    void add(WaveformSet *set);
    WaveformSet *at(int index) const;
    int count() const;
    WaveformSet *createSet(const QString &name);
    WaveformSet *createSetAt(int index, const QString &name);
    WaveformSet *currentSet() const;
    void deleteAt(int index);
    void deleteSet(WaveformSet *set);
    WaveformSet *findByName(const QString &name) const;
    int indexOf(WaveformSet *set) const;
    void insertAt(int index, WaveformSet *set);
    bool isCurrentSet(int index) const;
    bool isCurrentSet(WaveformSet *set) const;
    void setCurrentSet(int index);
    void setCurrentSet(WaveformSet *set);
    int size() const;
    QList<WaveformSet *> sets() const;

private:
    static WaveformSetStore *instance;

    QList<WaveformSet *> m_sets;
    WaveformSet *m_currentSet;

    WaveformSet *createDefaultSet();
    void restoreSets();
};
}

#endif // EPG_WAVEFORMSETSTORE_H
