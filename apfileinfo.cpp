#include "apfileinfo.h"
#include "QRegularExpression"
#include "QDebug"

APFileInfo::APFileInfo() : QFileInfo()
{
}

APFileInfo::APFileInfo(const QString &filePath) : QFileInfo(filePath)
{
}

QString APFileInfo::baseName() const
{
    if (isNumbered()) {
        const QString fileName = this->fileName();
        const int idx = fileName.indexOf(numberingRegexp());

        QString baseName = fileName.left(idx);
        const QRegularExpression regexp("[^a-zA-Z\\d]+$");
        baseName.remove(regexp);
        return baseName;
    } else {
        return QFileInfo::baseName();
    }
}

bool APFileInfo::belongsToSameRecordThat(const QString &filePath)
{
    return isNumbered() && filePath.contains(baseName());
}

QStringList APFileInfo::findRelatedFiles(TraversingFlags flags) const
{
    QStringList filePaths;
    QString index = numberingIndex();
    bool isZeroPadded = index.startsWith("0") && index.length() > 1;

    for (int i = 1; i < 10; i++) {
        index = QString::number(i);
        if (isZeroPadded) index.prepend("0");

        QString numberedFilePath = filePath();
        numberedFilePath.replace(numberingRegexp(), index);
        if (numberedFilePath != filePath()
            && !filePaths.contains(numberedFilePath)
            && QFileInfo(numberedFilePath).exists()) {
            filePaths << numberedFilePath;
        }
    }

    if (flags & IncludeAllFilePaths && !filePaths.contains(filePath())) {
        filePaths << filePath();
    }

    return filePaths;
}

QList<QStringList> APFileInfo::groupFilePathsBasedOnNumberedFilename(
        const QStringList &filePaths)
{
    QList<QStringList> groupedFilePaths;
    foreach (const QString filePath, filePaths) {
        bool belongsToDifferentRecord = true;
        for (int i = 0; i < groupedFilePaths.count(); i++) {
            QString referenceFilePath = groupedFilePaths.at(i).first();
            if (APFileInfo(filePath).belongsToSameRecordThat(referenceFilePath)) {
                groupedFilePaths[i].append(filePath);
                belongsToDifferentRecord = false;
                break;
            }
        }

        if (belongsToDifferentRecord) {
            groupedFilePaths.append(QStringList() << filePath);
        }
    }
    return groupedFilePaths;
}


bool APFileInfo::isAnyFilePathNumbered(const QStringList &filePaths)
{
    foreach (QString filePath, filePaths) {
        if (APFileInfo(filePath).isNumbered()) {
            return true;
        }
    }
    return false;
}

bool APFileInfo::isNumbered() const
{
    QRegularExpression regexp = numberingRegexp();
    return regexp.isValid() && regexp.match(fileName()).hasMatch();
}

QString APFileInfo::numberingIndex() const
{
    return numberingRegexp().match(fileName()).captured();
}

const QRegularExpression APFileInfo::numberingRegexp() const
{
    QString pattern;
    QString ext = suffix();
    if (ext == "dat") {
        pattern = "(?<=([^\\d]))0?\\d(?=\\.dat)";
    } else if (ext.startsWith("D")) {
        pattern = "(?<=\\.D0)\\d";
    } else {
        pattern = "(("; // invalid regex
    }

    return QRegularExpression(pattern);
}
