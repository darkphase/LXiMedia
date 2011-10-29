TEMPLATE = app
CONFIG += qtestlib
win32:CONFIG += console
QT -= gui
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lxivecintrintest
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

unix {
  contains(QMAKE_HOST.os, Linux) {
    system(cat /proc/cpuinfo | grep flags | grep mmx > /dev/null) {
      CONFIG += mmx
      QMAKE_CXXFLAGS += -mmmx
      QMAKE_CFLAGS += -mmmx
    }
    system(cat /proc/cpuinfo | grep flags | grep sse > /dev/null) {
      CONFIG += sse
      QMAKE_CXXFLAGS += -msse
      QMAKE_CFLAGS += -msse
    }
    system(cat /proc/cpuinfo | grep flags | grep sse2 > /dev/null) {
      CONFIG += sse2
      QMAKE_CXXFLAGS += -msse2
      QMAKE_CFLAGS += -msse2
    }
    system(cat /proc/cpuinfo | grep flags | grep sse3 > /dev/null) {
      CONFIG += sse3
      QMAKE_CXXFLAGS += -msse3
      QMAKE_CFLAGS += -msse3
    }
    system(cat /proc/cpuinfo | grep flags | grep ssse3 > /dev/null) {
      CONFIG += ssse3
      QMAKE_CXXFLAGS += -mssse3
      QMAKE_CFLAGS += -mssse3
    }
    system(cat /proc/cpuinfo | grep flags | grep sse4 > /dev/null) {
      CONFIG += sse4
      QMAKE_CXXFLAGS += -msse4
      QMAKE_CFLAGS += -msse4
    }
  }
}

# Files
HEADERS += vecbooltest.h \
    vecbooltest.hpp \
    vecfloattest.h \
    vecfloattest.hpp \
    vecinttest.h \
    vecinttest.hpp
SOURCES += main.cpp \
    scalartest.cpp

contains(CONFIG, mmx) {
    SOURCES += mmxtest.cpp
    DEFINES += ENABLE_MMX
}
contains(CONFIG, sse) {
    SOURCES += ssetest.cpp
    DEFINES += ENABLE_SSE
}
contains(CONFIG, sse2) {
    SOURCES += sse2test.cpp
    DEFINES += ENABLE_SSE2
}
contains(CONFIG, sse3) {
    SOURCES += sse3test.cpp
    DEFINES += ENABLE_SSE3
}
contains(CONFIG, ssse3) {
    SOURCES += ssse3test.cpp
    DEFINES += ENABLE_SSSE3
}
contains(CONFIG, sse4) {
    SOURCES += sse4test.cpp
    DEFINES += ENABLE_SSE4
}

# Run tests after link
unix:QMAKE_LFLAGS += -Wl,-rpath -Wl,.
unix:QMAKE_POST_LINK = cd $${DESTDIR} && ./$${TARGET} -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\QtTest4.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtTestd4.dll $${OUT_DIR} > NUL)
  }
}