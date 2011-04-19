# Default build settings for LXiMedia
CONFIG += qt thread warn_on
OBJECTS_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
INCLUDEPATH += $${PWD}/
DEPENDPATH += ./ $${PWD}/

# Version number
unix|win32-g++ {
  VERSION = $$system(cat ../VERSION)
  VERSION_MAJOR = $${VERSION}
  VERSION_MAJOR ~= s/\\.[0-9]+.+/
}

# Optimizations
include($${PWD}/optimize.pri)

# Reduce export symbol table size and binary size.
linux-g++ {
  QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
  QMAKE_CFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
}

# Debug information is added in release to make stack tracing possible.
linux-g++|win32-g++ {
  !contains(QMAKE_CXXFLAGS_RELEASE, -g) {
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_CFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE -= -Wl,-s
  }
}
