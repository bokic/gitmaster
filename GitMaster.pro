#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T23:20:05
#
#-------------------------------------------------

QT       += core gui

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gitmaster
TEMPLATE = app

SOURCES += main.cpp qgitmastermainwindow.cpp qgitrepository.cpp qgit.cpp qgitrepotreeitemdelegate.cpp
HEADERS +=          qgitmastermainwindow.h   qgitrepository.h   qgit.h   qgitrepotreeitemdelegate.h
FORMS   +=          qgitmastermainwindow.ui  qgitrepository.ui

RESOURCES += resource.qrc

LIBS    += -lgit2
