#-------------------------------------------------
#
# Project created by QtCreator 2011-09-06T12:09:33
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = autopeps
TEMPLATE = app

#QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk

SOURCES += main.cpp\
        mainwindow.cpp \
    epsignal.cpp \
    epsegment.cpp \
    epsignalreader.cpp \
    epsprofile.cpp \
    epsignalscontroller.cpp \
    aputils.cpp \
    apstatuscontroller.cpp \
    apworker.cpp \
    approgressbar.cpp \
    apseeker.cpp \
    apnpseeker.cpp \
    epsignalwidget.cpp \
    apviewporthandler.cpp \
    epsprofilewidget.cpp \
    epsegmentoverlay.cpp \
    apzoomwidget.cpp \
    appanoramicwidget.cpp \
    apseekerwidget.cpp \
    appdseeker.cpp \
    appreferenceswidget.cpp \
    apcseeker.cpp \
    apsegmentdialog.cpp \
	apgseeker.cpp \
    epsignalwriter.cpp \
    apsequentialseeker.cpp \
    apcontroller.cpp \
    apsegmenttype.cpp \
    apsegmenttypescontroller.cpp \
    apfseeker.cpp \
    ape1seeker.cpp \
	apnewpdseeker.cpp \
    apseekerpickerdialog.cpp \
    apedithandler.cpp

HEADERS  += mainwindow.h \
    epsignal.h \
    epsegment.h \
    epsignalreader.h \
    APGlobals.h \
    epsprofile.h \
    epsignalscontroller.h \
    aputils.h \
    apstatuscontroller.h \
    apworker.h \
    approgressbar.h \
    apseeker.h \
    apnpseeker.h \
    epsignalwidget.h \
    apviewporthandler.h \
    epsprofilewidget.h \
    epsegmentoverlay.h \
    apzoomwidget.h \
    appanoramicwidget.h \
    apseekerwidget.h \
    appdseeker.h \
    appreferenceswidget.h \
    apcseeker.h \
    apsegmentdialog.h \
	apgseeker.h \
    epsignalwriter.h \
    apsequentialseeker.h \
    apcontroller.h \
    apsegmenttype.h \
    apsegmenttypescontroller.h \
    apfseeker.h \
    ape1seeker.h \
	apnewpdseeker.h \
    apseekerpickerdialog.h \
    apedithandler.h

FORMS    += mainwindow.ui \
    apzoomwidget.ui \
    apseekerwidget.ui \
    appreferenceswidget.ui \
    apsegmentdialog.ui \
    apseekerpickerdialog.ui

RESOURCES += \
    resources.qrc

ICON = autopeps.icns









































