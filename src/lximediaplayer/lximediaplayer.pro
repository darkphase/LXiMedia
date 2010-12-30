TEMPLATE = app
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lximediaplayer
include($${LXIMEDIA_DIR}/include/config.pri)
UI_DIR = $${OBJECTS_DIR}
INCLUDEPATH += .
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Generate version.h
unix {
  system(mkdir -p $${OBJECTS_DIR})
  system(echo \\\"$${VERSION}\\\" > $${OBJECTS_DIR}/version.h)
  INCLUDEPATH += $${OBJECTS_DIR}/
}

win32 {
  BSOBJECTS_DIR = $$replace(OBJECTS_DIR, /, \\)
  system(if not exist $${BSOBJECTS_DIR} mkdir $${BSOBJECTS_DIR})
  system(echo \"$${VERSION}\" > $${BSOBJECTS_DIR}\version.h)
  INCLUDEPATH += $${OBJECTS_DIR}/
}

# Files
SOURCES += main.cpp \
 mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui

RESOURCES = images.qrc
