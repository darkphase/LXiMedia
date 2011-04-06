MODULE_NAME = lxistream_gui
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
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
