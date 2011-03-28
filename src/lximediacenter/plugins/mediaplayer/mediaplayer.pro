PLUGIN_NAME = mediaplayer
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblximediacenter/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter-internal.pri)

# Files
SOURCES += configserver.cpp \
    configserver.html.cpp \
    mediadatabase.cpp \
    mediaplayerbackend.cpp \
    mediaplayersandbox.cpp \
    mediaplayerserver.cpp \
    musicserver.cpp \
    photoserver.cpp \
    photoserver.html.cpp \
    playlist.cpp \
    playlistserver.cpp \
    slideshownode.cpp \
    slideshownode.blend.c \
    tvshowserver.cpp

HEADERS += configserver.h \
    mediadatabase.h \
    mediaplayerbackend.h \
    mediaplayersandbox.h \
    mediaplayerserver.h \
    musicserver.h \
    photoserver.h \
    playlist.h \
    playlistserver.h \
    slideshownode.h \
    tvshowserver.h

RESOURCES = images/mediaplayer_images.qrc
