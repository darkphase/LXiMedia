# Default build settings for LXiMedia
CONFIG += qt thread warn_on
INCLUDEPATH += $${PWD}/
DEPENDPATH += ./ $${PWD}/

# Version number
unix:VERSION = $$system(cat $${PWD}/../VERSION)

# Mac specific
macx {
  CONFIG -= app_bundle
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
}

# Visual Studio specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  DEFINES += __SSE__ __SSE2__
  QMAKE_CXXFLAGS += /MP
  QMAKE_CFLAGS += /MP
}

# Optimizations
CONFIG += mmx sse sse2
CONFIG -= 3dnow exceptions rtti

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

    macx {
      CONFIG += sse3

      QMAKE_CXXFLAGS += -msse3
      QMAKE_CFLAGS += -msse3
    }
  } else {
    macx {
      CONFIG += sse3 ssse3

      QMAKE_CXXFLAGS += -msse3 -mssse3
      QMAKE_CFLAGS += -msse3 -mssse3
    }
  }

  QMAKE_CFLAGS += -w -fno-common

  # All "hot" code is placed in plain old C files; these need to be fully
  # optimized, also in debug mode.
  QMAKE_CFLAGS_RELEASE -= -O2
  QMAKE_CFLAGS += -O3 -ffast-math
}

# Debug information is added in release to make stack tracing possible.
win32-g++ {
  !contains(QMAKE_CXXFLAGS_RELEASE, -g) {
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE -= -Wl,-s
  }
}
