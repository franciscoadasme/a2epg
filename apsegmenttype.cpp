#include "apsegmenttype.h"

APSegmentType::APSegmentType(int id, QString name, QColor color, QObject *parent) :
    QObject(parent)
{
    _id = id;
    _name = name;
    _color = color;
}

bool APSegmentType::operator ==(SegmentType type) const
{
    return id() == (int)type;
}

bool APSegmentType::operator ==(const APSegmentType & type) const
{
    return id() == type.id();
}

QColor APSegmentType::color() const
{
    return _color;
}

int APSegmentType::id() const
{
    return _id;
}

QString APSegmentType::name() const
{
    return _name;
}

QString APSegmentType::serialize() const
{
    QString serializedRgb = tr("%1-%2-%3").arg(color().red())
                            .arg(color().green())
                            .arg(color().blue());
    return tr("%1:%2:%3").arg(id()).arg(name()).arg(serializedRgb);
}

void APSegmentType::setColor(QColor color)
{
    _color = color;
    emit propertyDidChanged();
}

void APSegmentType::setName(QString name)
{
    _name = name;
    emit propertyDidChanged();
}

APSegmentType *APSegmentType::unserialize(QString serialized)
{
    QStringList tokens = serialized.split(":");
    Q_ASSERT_X(tokens.count() == 3,
               "APSegmentType::unserialize()",
               "SegmentType representation does not conform format");

    int id = tokens[0].toInt();
    QString name = tokens[1];
    QStringList colorRgb = tokens[2].split("-");
    QColor color(colorRgb[0].toInt(), colorRgb[1].toInt(), colorRgb[2].toInt());

    return new APSegmentType(id, name, color);
}
