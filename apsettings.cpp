#include "apsettings.h"

void APSettings::setValue(const QString &key, const QVariant &value)
{
  QSettings settings;
  settings.setValue(key, value);
}

QString APSettings::stringValue(
  const QString &key,
  const QVariant &defaultValue)
{
  return value(key, defaultValue).toString();
}

QVariant APSettings::value(
  const QString &key,
  const QVariant &defaultValue)
{
  QSettings settings;
  return settings.value(key, defaultValue);
}
