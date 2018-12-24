#-------------------------------------------------
#
# Project created by QtCreator 2013-02-17T03:34:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyEditor
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    texteditor.cpp \
    configdialog.cpp \
    configpages/configeditorpage.cpp \
    configpages/configurationpage.cpp \
    finddialog.cpp \
    addressbar.cpp \
    highlighter.cpp \
    outline.cpp \
    replacedialog.cpp \
    grepdialog.cpp \
    tagsmakedialog.cpp

HEADERS  += mainwindow.h \
    texteditor.h \
    configdialog.h \
    configpages/configeditorpage.h \
    configpages/configurationpage.h \
    finddialog.h \
    addressbar.h \
    highlighter.h \
    outline.h \
    replacedialog.h \
    grepdialog.h \
    tagsmakedialog.h

FORMS    += configdialog.ui \
    configpages/configeditorpage.ui \
    configpages/configurationpage.ui \
    finddialog.ui \
    replacedialog.ui \
    grepdialog.ui \
    tagsmakedialog.ui

RESOURCES += \
    res.qrc

#QTPLUGIN += qjpcodecs4
#LIBS += -LD:\Qt\4.8.4\plugins\codecs
#CONFIG += static











