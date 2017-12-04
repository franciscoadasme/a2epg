#include "colorlisteditor.h"

#include "utils/color.h"

ColorListEditor::ColorListEditor(QWidget *widget) : QComboBox(widget)
{
    populateList();
}

QColor ColorListEditor::color() const
{
    return qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
}

void ColorListEditor::setColor(const QColor color)
{
    setCurrentIndex(findData(color, int(Qt::DecorationRole)));
}

void ColorListEditor::populateList()
{
    QStringList colorNames = Utils::Color::uniqueColorNames();

    for (int i = 0; i < colorNames.size(); ++i) {
        QColor color(colorNames[i]);

        insertItem(i, colorNames[i]);
        setItemData(i, color, Qt::DecorationRole);
    }
}
