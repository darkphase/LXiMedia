TEMPLATE = app
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lximediaplayer
include($${LXIMEDIA_DIR}/include/config.pri)
UI_DIR = $${OBJECTS_DIR}
INCLUDEPATH += .
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Files
SOURCES += main.cpp \
 mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui

RESOURCES = images.qrc
