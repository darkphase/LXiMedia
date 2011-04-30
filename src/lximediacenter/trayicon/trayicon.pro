TEMPLATE = app
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximctrayicon

# Duplicate code to keep memory usage low
CONFIG += qt warn_on
QT += network xml
OBJECTS_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/ $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/ $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter
DEFINES += TRAYICON_ONLY
DEFINES += S_BUILD_LIBLXIMEDIACENTER

SOURCES += ../liblximediacenter/globalsettings.cpp
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/globalsettings.h

linux-g++|win32-g++ {
  # Optimize for size instead of speed
  QMAKE_CXXFLAGS_RELEASE -= -O2
  QMAKE_CXXFLAGS_RELEASE += -Os
}

unix:VERSION = $$system(cat $${PWD}/$${LXIMEDIA_DIR}/VERSION)

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)

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

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vcapp
  GUID = 3335c322-7366-11e0-ada8-f32ee8bcccba
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
