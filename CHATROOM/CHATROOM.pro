#-------------------------------------------------
#
# Project created by QtCreator 2018-06-09T15:41:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CHATROOM
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
INCLUDEPATH += "/usr/local/include"
LIBS += "/usr/local/lib/libboost_system.a"
LIBS += "/usr/local/lib/libboost_thread.a"
LIBS += "/usr/local/lib/libboost_chrono.a"
DEFINES += QT_DEPRECATED_WARNINGS
#INCLUDEPATH += "/usr/local/Cellar/boost/1.67.0_1/include"
#LIBS += "/usr/local/Cellar/boost/1.67.0_1/lib/libboost_system.a"
#LIBS += "/usr/local/Cellar/boost/1.67.0_1/lib/libboost_thread-mt.a"
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    message.cpp \
    client.cpp \
    Error_handle.cpp \
    login.cpp \
    regist.cpp \
    regist2.cpp \
    connection_base.cpp \
    chat_record.cpp

HEADERS += \
        mainwindow.h \
    Message.hpp \
    client.h \
    Error_handle.hpp \
    client.h \
    login.h \
    regist.h \
    regist2.h \
    defs.hpp \
    connection_base.h \
    chat_record.h

FORMS += \
        mainwindow.ui \
    login.ui \
    regist.ui \
    regist2.ui \
    chat_record.ui

RESOURCES += \
    icon.qrc
