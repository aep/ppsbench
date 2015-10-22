TEMPLATE = app
TARGET = ppsbench

QT+=widgets

QMAKE_CXXFLAGS += -std=c++11
CONFIG+=link_pkgconfig
PKGCONFIG+=libsigrokcxx

FORMS += mainwindow.ui
INCLUDEPATH += ./qt-charts

CONFIG+=debug


# Input
HEADERS += mainwindow.h
SOURCES += main.cpp mainwindow.cpp ./qt-charts/qxtlinechart.cpp
