MODULE_NAME = lxistream_alsa
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStream

# Files
HEADERS += alsainput.h \
 alsaoutput.h \
 module.h

SOURCES += alsainput.cpp \
 alsaoutput.cpp \
 module.cpp

LIBS += -lasound
