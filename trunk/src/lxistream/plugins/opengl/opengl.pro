# Globals
PLUGIN_NAME = opengl
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgl/linklxistreamgl.pri)
QT += gui opengl

# Files
unix {
  HEADERS += deinterlace.h
  SOURCES += deinterlace.cpp
}

HEADERS += module.h
SOURCES += module.cpp
