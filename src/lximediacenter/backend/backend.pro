TEMPLATE = app
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lximcbackend
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

# Files
SOURCES += main.cpp \
    backend.cpp \
    backend.css.cpp \
    backend.html.cpp
HEADERS += backend.h
RESOURCES = images/backend_images.qrc \
    ../../../ext/flowplayer/flowplayer.qrc

# Platform specific
unix { 
    SOURCES += unixdaemon.cpp
    HEADERS += unixdaemon.h
    target.path = /usr/bin
    INSTALLS += target
}
win32 { 
    CONFIG += console
    SOURCES += windowsservice.cpp
    HEADERS += windowsservice.h
}
