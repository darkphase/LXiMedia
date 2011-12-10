TEMPLATE = app
QT += network xml
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin

macx {
  TARGET = LXiMediaCenter
} else {
  TARGET = lximctrayicon
}

include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)

macx {
  CONFIG += app_bundle
  ICON = $${PWD}/$${LXIMEDIA_DIR}/src/lxicore/liblxicore/lximedia.icns
}

# Files
SOURCES += main.cpp \
 trayicon.cpp

HEADERS += trayicon.h

unix {
  !macx {
    target.path = /usr/bin
    INSTALLS += target
  }
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
