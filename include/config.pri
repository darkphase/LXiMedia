# Default build settings for LXiMedia
CONFIG += qt thread warn_on
INCLUDEPATH += $${OUT_PWD}/$${LXIMEDIA_DIR}/include/ $${PWD}/
DEPENDPATH += ./ $${OUT_PWD}/$${LXIMEDIA_DIR}/include/ $${PWD}/
include(platform.pri)

# Version number
unix:VERSION = $$system(cat $${PWD}/../VERSION)

# Mac specific
macx {
  CONFIG -= app_bundle
  QMAKE_MAC_SDK = macosx10.9
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

# Visual Studio specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  DEFINES += __SSE__ __SSE2__
  !contains(QMAKE_HOST.arch, x86_64) {
    QMAKE_CXXFLAGS += /arch:SSE2
    QMAKE_CFLAGS += /arch:SSE2
  }
  
  QMAKE_CXXFLAGS += /MP
  QMAKE_CFLAGS += /MP
}

# Optimizations
CONFIG += mmx sse sse2
CONFIG -= 3dnow exceptions rtti

unix {
  # Make sure the debug version can run locally without install.
  debug {
    QMAKE_LFLAGS += -Wl,-rpath -Wl,.
  }

  # Reduce export symbol table size and binary size.
  QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
  QMAKE_CFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
}

QMAKE_CXXFLAGS += $${PLATFORM_CFLAGS}
QMAKE_CFLAGS += $${PLATFORM_CFLAGS}

unix|win32-g++ {
  QMAKE_CFLAGS += -w -fno-common

  QMAKE_CFLAGS_RELEASE -= -O2
  QMAKE_CFLAGS += -O3 -ffast-math
}
