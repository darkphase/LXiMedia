MODULE_NAME = lxistreamdevice_winmm
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)

# Files
HEADERS += winmmaudioinput.h \
 winmmaudiooutput.h \
 module.h

SOURCES += winmmaudioinput.cpp \
 winmmaudiooutput.cpp \
 module.cpp

# Export plugin
SOURCES +=

LIBS += -lwinmm
