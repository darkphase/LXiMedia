TEMPLATE = app
QT += xml
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximediaplayer
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
UI_DIR = $${OBJECTS_DIR}
INCLUDEPATH += .
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStream

# Files
SOURCES += main.cpp \
 mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui

RESOURCES = images.qrc

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vcapp
  GUID = d9d4df56-736b-11e0-b8a6-4b90e26b66e7
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
