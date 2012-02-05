MODULE_NAME = lximediacenter_mediaplayer
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
QT += network

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

# Files
HEADERS += mediadatabase.h \
    mediaplayersandbox.h \
    mediaplayerserver.h \
    module.h \
    playlist.h \
    slideshownode.h

SOURCES += mediadatabase.cpp \
    mediaplayersandbox.cpp \
    mediaplayerserver.cpp \
    mediaplayerserver.html.cpp \
    module.cpp \
    playlist.cpp \
    slideshownode.cpp \
    slideshownode.blend.c

RESOURCES = mediaplayer.qrc

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = b1d6eb3e-736b-11e0-bcb5-fbf0641dd199
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
