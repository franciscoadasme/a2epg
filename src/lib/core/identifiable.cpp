#include "identifiable.h"

#include <QUuid>

Identifiable::Identifiable()
{
    m_id = qHash(QUuid::createUuid());
}

Identifiable::~Identifiable()
{
}

unsigned int Identifiable::id() const
{
    return m_id;
}
