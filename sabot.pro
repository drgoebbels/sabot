#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T18:32:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sabot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        crypt.c\
        general.c\
        database.c\
        sanet.c

HEADERS  += mainwindow.h\
         crypt.h\
         database.h\
         sanet.h

FORMS    += mainwindow.ui
