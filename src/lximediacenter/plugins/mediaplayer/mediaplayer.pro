MODULE_NAME = lximediacenter_mediaplayer
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
QT += network xml sql

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiMediaCenter
}

# Files
HEADERS += configserver.h \
    filenode.h \
    mediadatabase.h \
    mediaplayersandbox.h \
    mediaplayerserver.h \
    module.h \
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
    musicserver.cpp \
    photoserver.cpp \
    photoserver.html.cpp \
    playlist.cpp \
    playlistserver.cpp \
    slideshownode.cpp \
    slideshownode.blend.c \
    tvshowserver.cpp

RESOURCES = images/mediaplayer_images.qrc
