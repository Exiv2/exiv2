QT              += core
QT              -= gui

TARGET           = commandLineTool
CONFIG          += console
CONFIG          -= app_bundle

TEMPLATE         = app
SOURCES         += main.cpp

win32 {
  INCLUDEPATH   +=   $$quote(c:/MinGW/msys/1.0/local/include)
  LIBS          += -L$$quote(c:/MinGW/msys/1.0/local/include) -lexiv2.dll
}
