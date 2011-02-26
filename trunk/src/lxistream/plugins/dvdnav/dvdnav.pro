PLUGIN_NAME = dvdnav
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)

# Files
HEADERS += formatprober.h \
    module.h \
    discreader.h
SOURCES += formatprober.cpp \
    module.cpp \
    discreader.cpp
