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

SOURCES += ../liblximediacenter/ssdpclient.cpp \
 ../liblximediacenter/globalsettings.cpp

HEADERS += $${LXIMEDIA_DIR}/include/liblximediacenter/ssdpclient.h \
 $${LXIMEDIA_DIR}/include/liblximediacenter/globalsettings.h

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

# Generate version.h
unix {
  system(mkdir -p $${OBJECTS_DIR})
  system(echo \\\"$${VERSION}\\\" > $${OBJECTS_DIR}/version.h)
  INCLUDEPATH += $${OBJECTS_DIR}/
}

win32 {
  BSOBJECTS_DIR = $$replace(OBJECTS_DIR, /, \\)
  system(if not exist $${BSOBJECTS_DIR} mkdir $${BSOBJECTS_DIR})
  system(echo \"$${VERSION}\" > $${BSOBJECTS_DIR}\\version.h)
  INCLUDEPATH += $${OBJECTS_DIR}/
}

win32 {
  LIBS += -lws2_32
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
  CONFIG += windows
  RC_FILE = trayicon.rc
}
