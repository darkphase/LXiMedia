TEMPLATE = app
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lximctrayicon

# Duplicate code to keep memory usage low
CONFIG += qt warn_on
QT += network xml
OBJECTS_DIR = $${LXIMEDIA_DIR}/obj/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
INCLUDEPATH += $${LXIMEDIA_DIR}/include/ $${LXIMEDIA_DIR}/include/liblximediacenter
DEPENDPATH += $${LXIMEDIA_DIR}/include/ $${LXIMEDIA_DIR}/include/liblximediacenter
DEFINES += TRAYICON_ONLY

SOURCES += ../liblximediacenter/globalsettings.cpp
HEADERS += $${LXIMEDIA_DIR}/include/liblximediacenter/globalsettings.h

linux-g++|win32-g++ {
  # Optimize for size instead of speed
  QMAKE_CXXFLAGS_RELEASE -= -O2
  QMAKE_CXXFLAGS_RELEASE += -Os
}

unix|win32-g++ {
  VERSION = $$system(cat ../../../VERSION)
  VERSION_MAJOR = $${VERSION}
  VERSION_MAJOR ~= s/\\.[0-9]+.+/
}

include($${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiServer
}

# Files
SOURCES += main.cpp \
 trayicon.cpp

HEADERS += trayicon.h

RESOURCES = ../liblximediacenter/images/trayicon_images.qrc

unix {
    target.path = /usr/bin
    INSTALLS += target
}

win32 {
  LIBS += -lws2_32
  CONFIG += windows
  RC_FILE = trayicon.rc
}
