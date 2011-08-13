MODULE_NAME = lximediacenter_mediaplayer
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
QT += network xml sql

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

# Files
HEADERS += configserver.h \
    filenode.h \
    mediadatabase.h \
    mediaplayersandbox.h \
    mediaplayerserver.h \
    module.h \
    movieserver.h \
    musicserver.h \
    photoserver.h \
    playlist.h \
    playlistserver.h \
    slideshownode.h \
    tvshowserver.h

SOURCES += configserver.cpp \
    configserver.html.cpp \
    filenode.cpp \
    mediadatabase.cpp \
    mediaplayersandbox.cpp \
    mediaplayerserver.cpp \
    module.cpp \
    movieserver.cpp \
    musicserver.cpp \
    photoserver.cpp \
    photoserver.html.cpp \
    playlist.cpp \
    playlistserver.cpp \
    slideshownode.cpp \
    slideshownode.blend.c \
    tvshowserver.cpp

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = b1d6eb3e-736b-11e0-bcb5-fbf0641dd199
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
