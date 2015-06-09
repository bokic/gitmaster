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

SOURCES += main.cpp qgitmastermainwindow.cpp qgitrepository.cpp qgit.cpp qgitrepotreeitemdelegate.cpp
HEADERS +=          qgitmastermainwindow.h   qgitrepository.h   qgit.h   qgitrepotreeitemdelegate.h
FORMS   +=          qgitmastermainwindow.ui  qgitrepository.ui

RESOURCES += resource.qrc

win32:LIBS += -L$$_PRO_FILE_PWD_/bin

LIBS += -lgit2
