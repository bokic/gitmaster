#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T23:20:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gitmaster
TEMPLATE = app

SOURCES += main.cpp qgitmastermainwindow.cpp qgitrepository.cpp
HEADERS +=          qgitmastermainwindow.h   qgitrepository.h
FORMS   +=          qgitmastermainwindow.ui  qgitrepository.ui

RESOURCES += resource.qrc
