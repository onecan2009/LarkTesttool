#-------------------------------------------------
#
# Project created by QtCreator 2016-04-13T16:36:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testtool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    serialmodbus.cpp

HEADERS  += mainwindow.h \
    serialmodbus.h

FORMS    += mainwindow.ui

LIBS += -lmodbus
