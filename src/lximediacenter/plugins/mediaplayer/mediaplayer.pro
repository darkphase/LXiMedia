PLUGIN_NAME = mediaplayer
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblximediacenter/plugin.pri)

# Files
SOURCES += clipserver.cpp \
    configserver.cpp \
    configserver.html.cpp \
    homevideoserver.cpp \
    mediaplayerbackend.cpp \
    mediadatabase.cpp \
    mediadatabase.scan.cpp \
    mediaserver.cpp \
    movieserver.cpp \
    movieserver.html.cpp \
    musicserver.cpp \
    musicserver.html.cpp \
    photoserver.cpp \
    photoserver.html.cpp \
    playlist.cpp \
    playlistnode.cpp \
    slideshownode.cpp \
    slideshownode.blend.c \
    tvshowserver.cpp
HEADERS += clipserver.h \
    configserver.h \
    homevideoserver.h \
    mediaplayerbackend.h \
    mediadatabase.h \
    mediaserver.h \
    movieserver.h \
    musicserver.h \
    photoserver.h \
    playlist.h \
    playlistnode.h \
    slideshownode.h \
    tvshowserver.h
RESOURCES = images/mediaplayer_images.qrc
