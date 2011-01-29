# Default build settings for LXiMedia
CONFIG += qt thread warn_on
QT += network xml
OBJECTS_DIR = $${LXIMEDIA_DIR}/obj/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
INCLUDEPATH += $${LXIMEDIA_DIR}/include/
DEPENDPATH += ./ $${LXIMEDIA_DIR}/include/

# Precompiled header
exists($$_PRO_FILE_PWD_/precompiled.h) {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = precompiled.h
  QMAKE_CXXFLAGS += -include precompiled.h
}

# Version number
unix|win32-g++ {
  VERSION = $$system(cat ../VERSION)
  VERSION_MAJOR = $${VERSION}
  VERSION_MAJOR ~= s/\\.[0-9]+.+/
}

# Optimizations
include($${LXIMEDIA_DIR}/include/optimize.pri)

# Debug information is added in release to make stack tracing possible.
linux-g++|win32-g++ {
  !contains(QMAKE_CXXFLAGS_RELEASE, -g) {
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_CFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE -= -Wl,-s
  }
}
