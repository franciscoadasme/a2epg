#include "waveform.h"

#include <QDebug>
#include "waveformset.h"

namespace EPG {
Waveform::Waveform() : Identifiable()
{
    m_set = nullptr;
}

Waveform::Waveform(const Waveform &other) : Identifiable()
{
    m_id = other.m_id;
    m_color = other.m_color;
    m_description = other.m_description;
    m_name = other.m_name;
    m_set = other.m_set;
}

Waveform::Waveform(const QString &name, const QColor &color, WaveformSet *set) :
    Waveform(name, "", color, set)
{
}

Waveform::Waveform(const QString &name, const QString &description,
                   const QColor &color, WaveformSet *set) :
    Identifiable(),
    m_color(color),
    m_description(description),
    m_name(name),
    m_set(set)
{
}

Waveform::~Waveform()
{
    qDebug() << "Waveform::~Waveform(" << name() << ")";
}

QColor Waveform::color() const
{
    return m_color;
}

QString Waveform::description() const
{
    return m_description;
}

bool Waveform::isNameValid(const QString &name)
{
    // TODO add length {2,4} validation
    Waveform *other = m_set->findByName(name);
    return (other == nullptr) || (other == this);
}

QString Waveform::name() const
{
    return m_name;
}

WaveformSet *Waveform::set() const
{
    return m_set;
}

bool Waveform::setColor(const QColor &color)
{
    // TODO add validation for uniqueness
    if (m_color == color) return false;

    m_color = color;
    return true;
}

bool Waveform::setDescription(const QString &description)
{
    if (m_description == description) return false;

    m_description = description;
    return true;
}

bool Waveform::setName(const QString &name)
{
    if ((m_name == name) || !isNameValid(name))
        return false;

    m_name = name;
    return true;
}

QDebug operator <<(QDebug dbg, const Waveform &waveform)
{
    QDebugStateSaver stateSaver(dbg);

    dbg.nospace() << "EPG::Waveform<" << waveform.id() << ">("
                  << waveform.name() << ", "
                  << waveform.description().left(20) << ", "
                  << waveform.color() << ")";
    return dbg.maybeSpace();
}
}
