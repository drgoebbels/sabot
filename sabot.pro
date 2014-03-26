#-------------------------------------------------
#
# Project created by QtCreator 2014-03-26T00:41:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sabot
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        crypt.c \
        database.c \
        sanet.c \
        general.c \
        loginprompt.cpp \
        servers.c

HEADERS  += mainwindow.h \
            sanet.h \
            database.h  \
            crypt.h \
            general.h \
            loginprompt.h \
            servers.h

FORMS    += mainwindow.ui \
    loginprompt.ui
