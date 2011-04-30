TEMPLATE = app
QT += network xml
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximcbackend
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

# Files
SOURCES += main.cpp \
    backend.cpp \
    backend.css.cpp \
    backend.html.cpp \
    sandbox.cpp
HEADERS += $${PWD}/backend.h \
    $${PWD}/sandbox.h
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

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vcapp
  GUID = 1d1d07bc-7366-11e0-bb6e-ab91c88aba17
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
