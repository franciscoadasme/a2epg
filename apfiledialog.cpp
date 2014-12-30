#include "apfiledialog.h"
#include "QApplication"
#include "QMessageBox"
#include "APGlobals.h"
#include "apsettings.h"
#include "apfileinfo.h"

bool APFileDialog::askWhetherShouldLoadRelatedFilesOf(
        const QStringList &filePaths)
{
    bool shouldLoadRelatedFiles = false;
    foreach (QString filePath, filePaths) {
        if (APFileInfo(filePath).isNumbered()) {
            QMessageBox dialog(QApplication::activeWindow());
            dialog.setWindowTitle(tr("A2EPG ~ Opening files"));
            dialog.setIcon(QMessageBox::Question);
            dialog.setText(tr("One or more files appear to be part of a larger "
                              "recording."));
            dialog.setInformativeText(tr("Do you want to load existing files "
                                         "that belong to the same record as "
                                         "well?"));
            dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            dialog.setDefaultButton(QMessageBox::Yes);

            shouldLoadRelatedFiles = dialog.exec() == QMessageBox::Yes;
            break;
        }
    }
    return shouldLoadRelatedFiles;
}

QList<QStringList> APFileDialog::getGroupedOpenFileNames(QWidget *parent,
                                                         const QString &caption)
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
                                parent,
                                caption,
                                lastVisitedLocation(),
                                openFileFilterPatterns());

    if (!filePaths.isEmpty()) {
        APSettings::setValue(LastLocationVisitedKey,
                             QFileInfo(filePaths.first()).path());
        if (askWhetherShouldLoadRelatedFilesOf(filePaths)) {
            foreach (QString filePath, filePaths) {
                QStringList relatedFiles = APFileInfo(filePath).findRelatedFiles(
                                               APFileInfo::ExcludeRepresentedFilePath);
                foreach (QString relatedFilePath, relatedFiles) {
                    if (!filePaths.contains(relatedFilePath)) {
                        filePaths << relatedFilePath;
                    }
                }
            }
            filePaths.sort();
        }
    }

    return APFileInfo::groupFilePathsBasedOnNumberedFilename(filePaths);
}

QString APFileDialog::lastVisitedLocation()
{
    return APSettings::stringValue(LastLocationVisitedKey, QDir::homePath());
}

QString APFileDialog::openFileFilterPatterns()
{
    static QString filterPatterns;
    if (filterPatterns.isEmpty()) {
        QStringList filters;
        filters << tr("All Supported files (*.epg *.D0* *.dat)")
                << tr("Electrical Penetration Graph (*.epg)")
                << tr("PROBE/Stylet+a Acquisition Data (*.D0*)")
                << tr("Raw data/ASCII (*.dat)");
        filterPatterns = filters.join(";;");
    }
    return filterPatterns;
}
