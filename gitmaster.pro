#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T23:20:05
#
#-------------------------------------------------

QT       += core gui

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release)::TARGET = gitmaster_d
CONFIG(release, debug|release)::TARGET = gitmaster

DESTDIR    = $$PWD/bin
TEMPLATE = app

win32:INCLUDEPATH += include

SOURCES += main.cpp qgitmastermainwindow.cpp qgitrepository.cpp qnewrepositorydialog.cpp qgitclonerepositorydialog.cpp qgit.cpp qgitrepotreeitemdelegate.cpp qgitrepotreewidget.cpp
HEADERS +=          qgitmastermainwindow.h   qgitrepository.h   qnewrepositorydialog.h   qgitclonerepositorydialog.h   qgit.h   qgitrepotreeitemdelegate.h   qgitrepotreewidget.h   structs.h
FORMS   +=          qgitmastermainwindow.ui  qgitrepository.ui  qnewrepositorydialog.ui  qgitclonerepositorydialog.ui

RESOURCES += resource.qrc

win32:LIBS += -L$$_PRO_FILE_PWD_/bin

LIBS += -lgit2
