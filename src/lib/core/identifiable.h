#ifndef IDENTIFIABLE_H
#define IDENTIFIABLE_H

class Identifiable
{
public:
    Identifiable();
    virtual ~Identifiable();

    unsigned int id() const;

protected:
   unsigned int m_id;
};

#endif // IDENTIFIABLE_H
