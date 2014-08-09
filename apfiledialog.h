#ifndef APFILEDIALOG_H
#define APFILEDIALOG_H

#include <QFileDialog>

class APFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    static QList<QStringList> getGroupedOpenFileNames(
            QWidget *parent = 0,
            QString const & caption = QString());
    static QString lastVisitedLocation();

private:
    static bool askWhetherShouldLoadRelatedFilesOf(
            QStringList const & filePaths);
    static QString openFileFilterPatterns();
};

#endif // APFILEDIALOG_H
