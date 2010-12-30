# Globals
PLUGIN_NAME = winmm
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += winmmaudiodevice.h \
 winmmaudioinput.h \
 winmmaudiooutput.h \
 module.h

SOURCES += winmmaudiodevice.cpp \
 winmmaudioinput.cpp \
 winmmaudiooutput.cpp \
 module.cpp

LIBS += -lwinmm
