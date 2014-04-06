TEMPLATE = app
QT += concurrent widgets
QT -= gui
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximcbackend
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

# Files
SOURCES += main.cpp \
    backend.cpp \
    backend.html.cpp \
    setup.cpp \
    sandbox.cpp
HEADERS += $${PWD}/backend.h \
    $${PWD}/setup.h
RESOURCES = ../resources/backend.qrc

# Platform specific
unix { 
  !macx {
    target.path = /usr/bin
    INSTALLS += target
  }
}
win32 { 
  CONFIG += console
  LIBS += -ladvapi32
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vcapp
    GUID = 1d1d07bc-7366-11e0-bb6e-ab91c88aba17
    DEFINES += _CRT_SECURE_NO_WARNINGS
  }
}
