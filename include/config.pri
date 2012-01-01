# Default build settings for LXiMedia
CONFIG += qt thread warn_on
INCLUDEPATH += $${OUT_PWD}/$${LXIMEDIA_DIR}/include/ $${PWD}/
DEPENDPATH += ./ $${OUT_PWD}/$${LXIMEDIA_DIR}/include/ $${PWD}/

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
    QMAKE_CXXFLAGS += -mtune=generic -march=i686 -mmmx -msse -msse2 -mfpmath=sse
    QMAKE_CFLAGS += -mtune=generic -march=i686 -mmmx -msse -msse2 -mfpmath=sse

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

  QMAKE_CFLAGS_RELEASE -= -O2
  QMAKE_CFLAGS += -O3 -ffast-math
}

# Needed because of a bug in GCC 4.4
win32-g++ {
  QMAKE_CXXFLAGS += -mincoming-stack-boundary=2
  QMAKE_CFLAGS += -mincoming-stack-boundary=2
}
