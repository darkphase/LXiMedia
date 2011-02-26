PLUGIN_NAME = alsa
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)

# Files
HEADERS += alsainput.h \
 alsaoutput.h \
 module.h

SOURCES += alsainput.cpp \
 alsaoutput.cpp \
 module.cpp

LIBS += -lasound
