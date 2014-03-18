#-------------------------------------------------
#
# Project created by QtCreator 2014-03-17T14:34:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sabot
TEMPLATE = app lib


SOURCES += main.cpp\
        mainwindow.cpp\
        crypt.c\
        database.c\
        sanet.c\
        general.c \
    messagelist.cpp

HEADERS  += mainwindow.h\
            crypt.h\
            database.h\
            sanet.h \
    messagelist.h

FORMS    += mainwindow.ui


QT += designer
CONFIG += console plugin

target.path = $$[QT_INSTALL_LUGINS]/designer
INSTALLS += target
