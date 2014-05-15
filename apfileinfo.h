#ifndef APFILEINFO_H
#define APFILEINFO_H

#include <QFileInfo>

class APFileInfo : public QFileInfo
{
public:
  enum TraversingFlags {
    IncludeAllFilePaths        = 0x01,
    ExcludeRepresentedFilePath = 0x02
  };

  APFileInfo();
  APFileInfo(QString const & filePath);
  static bool isAnyFilePathNumbered(QStringList const & filePaths);
  static QList<QStringList> groupFilePathsBasedOnNumberedFilename(
    QStringList const & filePaths);

  QString baseName() const;
  bool belongsToSameRecordThat(QString const & filePath);
  QStringList findRelatedFiles(
    TraversingFlags flags = IncludeAllFilePaths) const;
  bool isNumbered() const;

private:
  QString numberingIndex() const;
  const QRegularExpression numberingRegexp() const;
};

#endif // APFILEINFO_H
