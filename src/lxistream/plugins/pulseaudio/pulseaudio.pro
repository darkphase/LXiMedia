MODULE_NAME = lxistream_pulseaudio
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStream

# Files
HEADERS += pulseaudioinput.h \
 pulseaudiooutput.h \
 module.h

SOURCES += pulseaudioinput.cpp \
 pulseaudiooutput.cpp \
 module.cpp

LIBS += -lpulse-simple
