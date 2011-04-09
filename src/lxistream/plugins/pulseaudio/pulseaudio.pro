MODULE_NAME = lxistream_pulseaudio
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiStream
}

# Files
HEADERS += pulseaudioinput.h \
 pulseaudiooutput.h \
 module.h

SOURCES += pulseaudioinput.cpp \
 pulseaudiooutput.cpp \
 module.cpp

LIBS += -lpulse-simple
