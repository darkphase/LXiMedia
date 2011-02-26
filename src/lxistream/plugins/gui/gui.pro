# Globals
PLUGIN_NAME = gui
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Files
HEADERS += formatprober.h \
    module.h \
    imageencoder.h \
    imagedecoder.h
SOURCES += formatprober.cpp \
    module.cpp \
    imageencoder.cpp \
    imagedecoder.cpp
