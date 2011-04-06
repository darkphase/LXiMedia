TEMPLATE = app
CONFIG += qtestlib
QT += network \
    xml
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lxistreamtest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)
include($${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
include($${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

# Files
HEADERS += streamtest.h \
    dvdnavtest.h \
    ffmpegtest.h \
    iotest.h
SOURCES += main.cpp \
    streamtest.cpp \
    dvdnavtest.cpp \
    ffmpegtest.cpp \
    iotest.cpp
#    fingerprinttest.cpp \
#    graphtest.cpp \
#    performancetest.cpp
RESOURCES = test.qrc
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
#win32:QMAKE_POST_LINK = $(TARGET) -silent
# Prevent dependency with .so files
FILES_UNDER_TEST = $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    #$${LXIMEDIA_DIR}/obj/LXiStreamGl/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${LXIMEDIA_DIR}/obj/lxistream_dvdnav/*.o \
    $${LXIMEDIA_DIR}/obj/lxistream_ffmpeg/*.o \
    $${LXIMEDIA_DIR}/obj/lxistream_fftw/*.o \
    $${LXIMEDIA_DIR}/obj/lxistream_gui/*.o

# Platform specific
unix {
    LIBS += -lX11 \
        -lXext \
        -lXrandr \
        -lXtst \
        -lXv
    QMAKE_LFLAGS += -z \
        muldefs
    
    # OpenGL Shader Language
    #HEADERS += opengltest.h
    #SOURCES += opengltest.cpp
    #DEFINES += "ENABLE_GLSL"
    #FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/opengl/*.o
}
linux-g++ {
    # ALSA
    #HEADERS += alsatest.h
    #SOURCES += alsatest.cpp
    #DEFINES += "ENABLE_ALSA"
    #LIBS += -lasound
    #FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/alsa/*.o
    
    # V4L
    #SOURCES += v4ltest.cpp
    #DEFINES += "ENABLE_V4L"
    #FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/v4l/*.o
    
    # Linux DVB
    #SOURCES += linuxdvbtest.cpp
    #DEFINES += "ENABLE_LINUXDVB"
    #FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/linuxdvb/*.o
}
win32 { 
    CONFIG += console
}
QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}
