TEMPLATE = app
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lxistreamfiletest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)
include($${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)

# Files
HEADERS += filetester.h
SOURCES += main.cpp \
    filetester.cpp
LIBS += -lbfd \
    -liberty

# Prevent dependency with .so files
FILES_UNDER_TEST = $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    #$${LXIMEDIA_DIR}/obj/LXiStreamGl/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${LXIMEDIA_DIR}/obj/lxistream_dvdnav/*.o \
    $${LXIMEDIA_DIR}/obj/lxistream_ffmpeg/*.o

# Platform specific
unix {
    LIBS += -lX11 \
        -lXext \
        -lXrandr \
        -lXtst \
        -lXv
    QMAKE_LFLAGS += -z \
        muldefs
}
win32 { 
    CONFIG += console
}
QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}
