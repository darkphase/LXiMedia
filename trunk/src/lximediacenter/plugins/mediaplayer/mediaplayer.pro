PLUGIN_NAME = mediaplayer
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblximediacenter/plugin.pri)

# Files
SOURCES += configserver.cpp \
    configserver.html.cpp \
    mediaplayerbackend.cpp \
    mediadatabase.cpp \
    mediadatabase.scan.cpp \
    mediaplayerserver.cpp \
    photoserver.cpp \
    photoserver.html.cpp \
    slideshownode.cpp \
    slideshownode.blend.c \
    tvshowserver.cpp

HEADERS += configserver.h \
    mediaplayerbackend.h \
    mediadatabase.h \
    mediaplayerserver.h \
    photoserver.h \
    slideshownode.h \
    tvshowserver.h

RESOURCES = images/mediaplayer_images.qrc

# Files (Servers)
#SOURCES += #clipserver.cpp \
    #homevideoserver.cpp \
    #movieserver.cpp \
    #movieserver.html.cpp \
    #musicserver.cpp \
    #musicserver.html.cpp \
    #tvshowserver.cpp \
    #playlist.cpp \
    #playlistnode.cpp
#HEADERS += #clipserver.h \
    #homevideoserver.h \
    #movieserver.h \
    #musicserver.h \
    #playlist.h \
    #playlistnode.h
