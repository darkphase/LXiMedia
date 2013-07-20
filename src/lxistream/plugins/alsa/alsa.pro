MODULE_NAME = lxistreamdevice_alsa
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)

# Files
HEADERS += alsainput.h \
 alsaoutput.h \
 module.h \
    alsamixer.h

SOURCES += alsainput.cpp \
 alsaoutput.cpp \
 module.cpp \
    alsamixer.cpp

LIBS += -lasound

# Export plugin
SOURCES +=
