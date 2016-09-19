QT += core
QT -= gui

CONFIG += c++11

TARGET = commandLineTool
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

QMAKE_CXXFLAGS = -std=c++98

win32 {
    INCLUDEPATH  +=    $$quote(c:/temp/dist/mingw/include)
    INCLUDEPATH  +=    $$quote(c:/temp/dist/mingw/include)
    LIBS         += -L $$quote(c:/temp/dist/mingw/include) -lexiv2.dll
}
