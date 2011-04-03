TEMPLATE = lib
CONFIG += dll
QT += opengl
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiStreamGl
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxistream $${LXIMEDIA_DIR}/include/liblxistreamgl
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblxistream $${LXIMEDIA_DIR}/include/liblxistreamgl

include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += $${LXIMEDIA_DIR}/include/LXiStreamGl \
 $${LXIMEDIA_DIR}/include/liblxistreamgl/sglsystem.h \
 $${LXIMEDIA_DIR}/include/liblxistreamgl/sglvideoview.h \
 $${LXIMEDIA_DIR}/include/liblxistreamgl/stexturebuffer.h

SOURCES += sglsystem.cpp \
 sglvideoview.cpp \
 stexturebuffer.cpp

# Optimized C implementations
SOURCES += stexturebuffer.convert.c

# Platform specific
unix {
  HEADERS += $${LXIMEDIA_DIR}/include/liblxistreamgl/sglshader.h
  SOURCES += sglshader.cpp
  DEFINES += ENABLE_GLSL
}

win32-g++ {
  system(mkdir ..\..\..\bin\ > NUL 2>&1)

  release {
    system(cp -u $$(QTDIR)/bin/QtOpenGL4.dll -t $${LXIMEDIA_DIR}/bin)
  }
}
