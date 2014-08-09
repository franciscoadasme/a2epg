#ifndef EPSEGMENT_H
#define EPSEGMENT_H

#include <QObject>
#include <QColor>
#include <QPair>

#include "apsegmenttype.h"

class EPSegment : public QObject
{
    Q_OBJECT
public:
    explicit EPSegment(int type,
                       unsigned int start,
                       unsigned int end,
                       int id = APNotFound,
                       QObject *parent = 0);

    int id();
    unsigned int start();
    void setStart(unsigned int start, bool silent = false);
    unsigned int end();
    void setEnd(unsigned int end, bool silent = false);
    unsigned int length();
    APSegmentType *type();
    void setType(APSegmentType *type, bool silent = false);
    void setType(int type, bool silent = false);
    QString comments();
    EPSegment *setComments(QString someComments);
    bool isCommented();

    QString description(bool verbose = false);
    int duration();
    bool satisfyDurationRestriction();

    bool haveTranscientValues();
    uint transcientStart();
    uint transcientEnd();
    void setTranscientRange(uint start, uint end);
    void setTranscientStart(uint start);
    void setTranscientEnd(uint end);
    void resetTranscientValues();
    void pushChanges();
    QList<EPSegment *> findCollisions();

    QString serialize();
    static EPSegment *unserialize(QString info);
    void pushSerializedChange(QString change, Direction direction);
    QString lastChangeInfo();

signals:
    void endsDidChanged();
    void endsDidChanged(float, float);
    void transcientEndsDidChanged();
    void typeDidChanged();
    void typeDidChanged(int);
    void typeRestored(int);
    void commentsDidChanged();

private:
    int _id;
    APSegmentType *_type;
    unsigned int _start;
    unsigned int _end;
    QString _comments;

    uint _transcientStart, _transcientEnd;
    QPair<QString, uint> _lastState;

    static EPSegment *_temporalSegment;
};

#endif // EPSEGMENT_H
