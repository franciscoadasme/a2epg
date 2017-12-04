#ifndef COLOR_H
#define COLOR_H

#include <QColor>
#include <QList>

namespace Utils {
namespace Color {
QString colorName(const QColor &color);
QString colorName(const QString &hexValue);
QColor randomColor();
QColor randomNamedColor();
QList<QString> uniqueColorNames();
QList<QColor> uniqueNamedColors();
}
}

#endif // COLOR_H
