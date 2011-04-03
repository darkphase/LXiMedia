MODULE_NAME = lximediacenter.mediaplayer
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter-internal.pri)

# Files
HEADERS += configserver.h \
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
