TEMPLATE = app
CONFIG += qtestlib
QT += network \
    sql \
    xml
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lximediacentertest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
include($${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

unix {
  # Prevent dependency with .so files
  QMAKE_LFLAGS += $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiServer/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${LXIMEDIA_DIR}/obj/LXiMediaCenter/*.o

  POST_TARGETDEPS += $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiServer/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${LXIMEDIA_DIR}/obj/LXiMediaCenter/*.o
} else {
  include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
  include($${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
  include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
  include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
  include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
}

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiMediaCenter
}

# Files
HEADERS += commontest.h
SOURCES += main.cpp \
    commontest.cpp
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

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
