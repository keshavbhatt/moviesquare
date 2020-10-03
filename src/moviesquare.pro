#-------------------------------------------------
#
# Project created by QtCreator 2020-10-03T09:24:09
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = moviesquare
TEMPLATE = app

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

VERSION = 0.0.3
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"


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
        discover/discover.cpp \
        discover/item.cpp \
        discover/moviedetail.cpp \
        error.cpp \
        main.cpp \
        mainwindow.cpp \
        remotepixmaplabel.cpp \
        request.cpp \
        rungaurd.cpp \
        utils.cpp \
        widgets/RangeSlider.cpp \
        widgets/ccheckboxitemdelegate.cpp \
        widgets/circularprogessbar.cpp \
        widgets/controlbutton.cpp \
        widgets/cverticallabel.cpp \
        widgets/elidedlabel.cpp \
        widgets/scrolltext.cpp \
        widgets/spoiler.cpp \
        widgets/waitingspinnerwidget.cpp

HEADERS += \
        discover/discover.h \
        discover/item.h \
        discover/moviedetail.h \
        error.h \
        mainwindow.h \
        remotepixmaplabel.h \
        request.h \
        rungaurd.h \
        utils.h \
        widgets/RangeSlider.h \
        widgets/ccheckboxitemdelegate.h \
        widgets/circularprogessbar.h \
        widgets/controlbutton.h \
        widgets/cverticallabel.h \
        widgets/elidedlabel.h \
        widgets/scrolltext.h \
        widgets/spoiler.h \
        widgets/waitingspinnerwidget.h

FORMS += \
        discover/Oitem.ui \
        discover/discover.ui \
        discover/item.ui \
        discover/moviedetail.ui \
        discover/title_action.ui \
        error.ui \
        mainwindow.ui

# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

icon.files = icons/moviesquare.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = moviesquare.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop

RESOURCES += \
    icons.qrc \
    qbreeze.qrc \
    resources.qrc
