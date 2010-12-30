PLUGIN_NAME = pulseaudio
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += pulseaudioinput.h \
 pulseaudiooutput.h \
 module.h

SOURCES += pulseaudioinput.cpp \
 pulseaudiooutput.cpp \
 module.cpp

LIBS += -lpulse-simple
