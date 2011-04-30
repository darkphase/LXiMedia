MODULE_NAME = lxistream_gui
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamGui

# Files
HEADERS += formatprober.h \
    module.h \
    imageencoder.h \
    imagedecoder.h
SOURCES += formatprober.cpp \
    module.cpp \
    imageencoder.cpp \
    imagedecoder.cpp
