MODULE_NAME = lxistream.pulseaudio
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)

# Files
HEADERS += pulseaudioinput.h \
 pulseaudiooutput.h \
 module.h

SOURCES += pulseaudioinput.cpp \
 pulseaudiooutput.cpp \
 module.cpp

LIBS += -lpulse-simple
