SOURCES += \
    $$PWD/main.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/widgets/preferences/preferenceswidget.cpp \
    $$PWD/widgets/preferences/waveforms/waveformviewcontroller.cpp \
    $$PWD/widgets/preferences/waveforms/waveformtreemodel.cpp \
    $$PWD/editors/colorlisteditor.cpp \
    $$PWD/widgets/preferences/waveforms/waveformitem.cpp \
    $$PWD/widgets/preferences/waveforms/waveformsetitem.cpp \
    $$PWD/widgets/preferences/waveforms/waveformsetstoreitem.cpp

HEADERS += \
    $$PWD/mainwindow.h \
    $$PWD/widgets/preferences/preferenceswidget.h \
    $$PWD/widgets/preferences/waveforms/waveformviewcontroller.h \
    $$PWD/widgets/preferences/waveforms/waveformtreemodel.h \
    $$PWD/editors/colorlisteditor.h \
    $$PWD/widgets/preferences/waveforms/itemkind.h \
    $$PWD/widgets/preferences/waveforms/waveformitem.h \
    $$PWD/widgets/preferences/waveforms/waveformsetitem.h \
    $$PWD/widgets/preferences/waveforms/datacolumn.h \
    $$PWD/widgets/preferences/waveforms/waveformsetstoreitem.h

FORMS += \
    $$PWD/mainwindow.ui \
    $$PWD/widgets/preferences/preferenceswidget.ui

INCLUDEPATH += \
    src/lib \
    src/app
