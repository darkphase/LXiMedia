PLUGIN_NAME = mediaplayer
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblximediacenter/plugin.pri)

# Files
SOURCES += configserver.cpp \
    configserver.html.cpp \
    mediaplayerbackend.cpp \
    mediadatabase.cpp \
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
    mediaplayerbackend.h \
    mediadatabase.h \
    mediaplayerserver.h \
    musicserver.h \
    photoserver.h \
    playlist.h \
    playlistserver.h \
    slideshownode.h \
    tvshowserver.h

RESOURCES = images/mediaplayer_images.qrc
