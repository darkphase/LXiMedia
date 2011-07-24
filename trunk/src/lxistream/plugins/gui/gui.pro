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

# Export plugin
SOURCES += export.cpp

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = 39d287b8-7359-11e0-bd3d-83fae582b1ff
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
