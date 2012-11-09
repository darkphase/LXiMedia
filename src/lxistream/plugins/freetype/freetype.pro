MODULE_NAME = lxistream_freetype
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/freetype/freetype.pri)

unix:INCLUDEPATH += /usr/include/freetype2

# Files
HEADERS += module.h \
 subtitlerenderer.h
SOURCES += module.cpp \
 subtitlerenderer.cpp
RESOURCES += freetype.qrc

# Export plugin
SOURCES += export.cpp

include($${PWD}/$${LXIMEDIA_DIR}/src/lxistream/algorithms/linkalgorithms.pri)

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = 81855346-3a38-11e1-a0ef-773296681d14
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
