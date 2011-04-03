TEMPLATE = app
CONFIG += qtestlib
QT += network \
    sql \
    xml
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = liblximediacentertest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
include($${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

# Files
HEADERS += commontest.h
SOURCES += main.cpp \
    commontest.cpp
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
#win32:QMAKE_POST_LINK = $(DESTDIR)$(TARGET) -silent

# Prevent dependency with .so files
FILES_UNDER_TEST = $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiServer/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${LXIMEDIA_DIR}/obj/LXiMediaCenter/*.o

# Platform specific
unix { 
    LIBS += -lbfd \
        -liberty \
        -lX11 \
        -lXext \
        -lXrandr \
        -lXtst \
        -lXv \
        -lz
    QMAKE_LFLAGS += -z \
        muldefs
}
win32 { 
    CONFIG += console
    LIBS += -lbfd \
        -liberty \
        -lws2_32
    QMAKE_LFLAGS += -Wl,-allow-multiple-definition
}

QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}
