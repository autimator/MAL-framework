#-------------------------------------------------
#
# Project created by QtCreator 2018-12-23T14:02:10
#
#-------------------------------------------------

QT       += core gui \
            printsupport \
            sql \


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = text_finder_example
TEMPLATE = app

INCLUDEPATH += "C:\Program Files (x86)\Project\include"
LIBS += -L"C:\Program Files (x86)\Project\lib" -ldlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        widget.cpp \
    settings.cpp \
    game.cpp \
    game_creator.cpp \
    comparison.cpp \
    algorithms.cpp \
    comp_data.cpp \
    comp_algo.cpp \
    roundrobin_tab.cpp \
    replicator.cpp \
    qcustomplot.cpp


HEADERS += \
        widget.h \
    settings.h \
    game.h \
    game_creator.h \
    comparison.h \
    algorithms.h \
    comp_data.h \
    comp_algo.h \
    roundrobin_tab.h \
    replicator.h \
    qcustomplot.h


FORMS += \
        widget.ui \
    settings.ui \
    game_creator.ui \
    algorithms.ui \
    roundrobin_tab.ui \
    replicator.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=

DISTFILES +=

