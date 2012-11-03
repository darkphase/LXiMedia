MODULE_NAME = lxistreamdevice_pulseaudio
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)

# Files
HEADERS += pulseaudioinput.h \
 pulseaudiooutput.h \
 module.h \
    pulseaudiodevices.h

SOURCES += pulseaudioinput.cpp \
 pulseaudiooutput.cpp \
 module.cpp \
    pulseaudiodevices.cpp

# Export plugin
SOURCES += export.cpp

LIBS += -lpulse-simple
