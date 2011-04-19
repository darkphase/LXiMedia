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

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStream

# Files
SOURCES += main.cpp \
 mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui

RESOURCES = images.qrc
