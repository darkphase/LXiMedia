MODULE_NAME = lxistreamdevice_xcbcapture
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)

# Files
HEADERS += module.h \
 screeninput.h

SOURCES += module.cpp \
 screeninput.cpp

# Export plugin
SOURCES +=

LIBS += -lxcb -lxcb-shm -lxcb-xinerama
