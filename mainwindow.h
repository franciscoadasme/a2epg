#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <QMainWindow>

#include "epsignal.h"
#include "epsignalwidget.h"
#include "epsprofilewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow *instance();
    ~MainWindow();

    EPSignalWidget *epsignalWidget();
    EPSProfileWidget *epsprofileWidget();

public slots:
    void about();
    void aboutQt();
    void addSegment();
    void closeCurrentSignal();
    void goTo();
    void openFile();
    void runAll();
    void runChecked();
    void save();
    void saveAs();
    void saveAll();
    void showPreferences();
    void showPreferences(int tabIndex);
    void showScanPreferences();
    void updateSegmentLabel();
    void updateInfoSegments();
    void navWidgetVisibilityChanged(bool);

    void disconnectFromSignal(EPSignal *oldSignal);
    void updateUIComponentsForSignal(EPSignal *signal);
    void updateScrollBar();

    void readingDidEnd(bool success, QObject *, QString msg);
    void writingDidEnd(bool success, QObject *, QString msg);

    void removeFocusedSegmentAction();
    void scrollToSegment(EPSegment *);

protected:
    explicit MainWindow(QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);

private slots:
    void on_commentsTextEdit_textChanged();
    void on_nameLineEdit_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    static MainWindow *_instance;
    QList<QAction *> addSegmentActions;

    void setUpConnections();
    void setUpActionsAndMenus();
    void setUpPanoramic();

    void updateInfoComponentsForSignal(EPSignal *signal);
    void updateMenuAndActionsForSignal(EPSignal *signal);

    void runSearchEngines(QList<SegmentType> typesToRun, bool shouldFillGaps);
};

#endif // MAINWINDOW_H
