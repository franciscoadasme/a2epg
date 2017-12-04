#ifndef COLORLISTEDITOR_H
#define COLORLISTEDITOR_H

#include <QColor>
#include <QComboBox>
#include <QObject>

class ColorListEditor : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor USER true)

public:
    ColorListEditor(QWidget *widget = nullptr);

    QColor color() const;
    void setColor(const QColor color);

private:
    void populateList();
};

#endif // COLORLISTEDITOR_H
