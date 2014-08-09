#ifndef APSETTINGS_H
#define APSETTINGS_H

#include <QSettings>

class APSettings : public QSettings
{
public:
    static void setValue(const QString &key, const QVariant &value);
    static QString stringValue(const QString &key,
                               const QVariant &defaultValue = QVariant());
    static QVariant value(const QString &key,
                          const QVariant &defaultValue = QVariant());
};

#endif // APSETTINGS_H
