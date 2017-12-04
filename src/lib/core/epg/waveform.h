#ifndef EPG_WAVEFORM_H
#define EPG_WAVEFORM_H

#include <QColor>
#include <QMetaType>
#include "../identifiable.h"

namespace EPG {
class WaveformSet;

class Waveform : public Identifiable
{
public:
    Waveform();
    Waveform(const Waveform &other);
    explicit Waveform(const QString &name, const QColor &color,
                      WaveformSet *set);
    explicit Waveform(const QString &name, const QString &description,
                      const QColor &color, WaveformSet *set);
    ~Waveform();

    QColor color() const;
    QString description() const;
    QString name() const;
    WaveformSet *set() const;
    bool setColor(const QColor &color);
    bool setDescription(const QString &description);
    bool setName(const QString &name);

private:
    QColor m_color;
    QString m_description, m_name;
    WaveformSet *m_set;

    bool isNameValid(const QString &name);
};

QDebug operator <<(QDebug debug, const Waveform &waveform);
}

Q_DECLARE_METATYPE(EPG::Waveform)

#endif // EPG_WAVEFORM_H
