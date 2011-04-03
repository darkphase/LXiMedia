MODULE_NAME = lxistream.alsa
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)

# Files
HEADERS += alsainput.h \
 alsaoutput.h \
 module.h

SOURCES += alsainput.cpp \
 alsaoutput.cpp \
 module.cpp

LIBS += -lasound
