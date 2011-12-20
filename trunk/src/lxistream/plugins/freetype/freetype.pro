MODULE_NAME = lxistream_freetype
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/freetype/freetype.pri)

# Files
HEADERS += module.h \
 subtitlerenderer.h
SOURCES += module.cpp \
 subtitlerenderer.cpp
RESOURCES += freetype.qrc

# Export plugin
SOURCES += export.cpp

include($${PWD}/$${LXIMEDIA_DIR}/src/lxistream/algorithms/linkalgorithms.pri)
