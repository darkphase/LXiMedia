PLUGIN_NAME = dvdread
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${LXIMEDIA_DIR}/ext/dvdread/dvdread.pri)

# Files
HEADERS += formatprober.h \
    module.h \
    discreader.h
SOURCES += formatprober.cpp \
    module.cpp \
    discreader.cpp
