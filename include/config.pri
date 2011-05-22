# Default build settings for LXiMedia
CONFIG += qt thread warn_on
OBJECTS_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
INCLUDEPATH += $${PWD}/
DEPENDPATH += ./ $${PWD}/

# Version number
unix:VERSION = $$system(cat $${PWD}/../VERSION)

# Optimizations
CONFIG += mmx sse sse2

unix {
  # Reduce export symbol table size and binary size.
  QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
  QMAKE_CFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
}

unix|win32-g++ {
  !contains(QMAKE_HOST.arch, x86_64) {
    # All floating point operations are to be performed on the SSE2 unit
    QMAKE_CXXFLAGS += -march=i686 -mmmx -msse -msse2 -mfpmath=sse
    QMAKE_CFLAGS += -march=i686 -mmmx -msse -msse2 -mfpmath=sse
  }

  QMAKE_CFLAGS += -w

  # Improve floating point performance
  QMAKE_CXXFLAGS += -fno-math-errno -fno-signed-zeros
  QMAKE_CFLAGS += -fno-math-errno -fno-signed-zeros -fno-common -ffast-math

  # All "hot" code is placed in plain old C files; these need to be fully
  # optimized, also in debug mode.
  QMAKE_CFLAGS_RELEASE -= -O2
  QMAKE_CFLAGS += -O3
}

# Debug information is added in release to make stack tracing possible.
win32-g++ {
  !contains(QMAKE_CXXFLAGS_RELEASE, -g) {
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE -= -Wl,-s
  }
}

# Multithreaded build
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  DEFINES += __SSE__
  QMAKE_CXXFLAGS += /MP
  QMAKE_CFLAGS += /MP
}
