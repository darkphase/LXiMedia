TEMPLATE = app
QT += network xml
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximcbackend
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter
}

# Files
SOURCES += main.cpp \
    backend.cpp \
    backend.css.cpp \
    backend.html.cpp \
    sandbox.cpp
HEADERS += backend.h \
    sandbox.h
RESOURCES = images/backend_images.qrc \
    ../../../ext/flowplayer/flowplayer.qrc

# Platform specific
unix { 
    target.path = /usr/bin
    INSTALLS += target
}
win32 { 
    CONFIG += console
}
