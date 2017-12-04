#include "color.h"

#include <algorithm>
#include <QMap>

namespace Utils {
namespace Color {
QMap<QString, QString> hexArgbNames();

QString colorName(const QColor &color)
{
    return colorName(color.name(QColor::HexArgb));
}

QString colorName(const QString &hexValue)
{
    return hexArgbNames().value(hexValue);
}

QMap<QString, QString> hexArgbNames()
{
    static QMap<QString, QString> hexArgbNames;
    if (hexArgbNames.isEmpty()) {
        foreach(QString name, QColor::colorNames()) {
            hexArgbNames.insert(QColor(name).name(QColor::HexArgb), name);
        }
    }

    return hexArgbNames;
}

QColor randomColor()
{
    return QColor::fromHsl(qrand() % 359, 255, 128);
}

QColor randomNamedColor()
{
    return uniqueColorNames().at(qrand() % uniqueColorNames().size());
}

QList<QString> uniqueColorNames()
{
    QList<QString> names;
    foreach(QColor color, uniqueNamedColors()) {
        names.append(colorName(color));
    }
    return names;
}

QList<QColor> uniqueNamedColors()
{
    static QList<QColor> uniqueNamedColors;
    if (uniqueNamedColors.isEmpty()) {
        foreach(QString name, QColor::colorNames()) {
            QColor color(name);
            if ((name == "transparent") || uniqueNamedColors.contains(color))
                continue;
            uniqueNamedColors.append(color);
        }

        std::sort(uniqueNamedColors.begin(), uniqueNamedColors.end());
    }

    return uniqueNamedColors;
}
}
}

static bool operator<(const QColor &a, const QColor &b)
{
    return a.hslHueF() < b.hslHueF();
}
