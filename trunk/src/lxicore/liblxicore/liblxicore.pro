TEMPLATE = lib
CONFIG += dll
QT -= gui
LXIMEDIA_DIR = ../../..
macx {
    DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiMediaCenter.app/Contents/Frameworks
} else {
    DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
}
TARGET = LXiCore
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore

DEFINES += S_BUILD_LIBLXICORE

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiCore

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiCore \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/export.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sapplication.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sdaemon.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sfactory.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/smemorypool.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/smodule.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sstringparser.h
SOURCES += sapplication.cpp \
    sfactory.cpp \
    smemorypool.cpp \
    smodule.cpp \
    sstringparser.cpp \
    sstringparser.iso639.cpp
RESOURCES = liblxicore.qrc

# Platform specific
unix {
  SOURCES += sdaemon.unix.cpp

  !macx {
    target.path = /usr/lib
    INSTALLS += target
  }
}
win32 {
  SOURCES += sdaemon.win.cpp
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  system(mkdir $${OUT_DIR}\\platforms > NUL 2>&1)

  release {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Core.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\platforms\\qwindows.dll $${OUT_DIR}\\platforms > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Cored.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\platforms\\qwindowsd.dll $${OUT_DIR}\\platforms > NUL)
  }

  win32-g++ {
    system(copy /Y $$(QTDIR)\\bin\\libgcc_s_*.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\libstdc*.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\libwinpthread*.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\icudt*.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\icuin*.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\icuuc*.dll $${OUT_DIR} > NUL)
  }
  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = 1aab8ba4-734b-11e0-ab38-ff414b1aa9c5
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
  }
}
