QT              += core
QT              -= gui

TARGET           = commandLineTool
CONFIG          += console
CONFIG          -= app_bundle

TEMPLATE         = app
SOURCES         += main.cpp

win32 {
  INCLUDEPATH   +=   $$quote(c:/Qt/5.14.2/mingw73_64/include)
  INCLUDEPATH   +=   /usr/local/include
  LIBS          += -L$$quote(c:/Qt/5.14.2/mingw73_64/include) -L/usr/local/lib -lexiv2
}
