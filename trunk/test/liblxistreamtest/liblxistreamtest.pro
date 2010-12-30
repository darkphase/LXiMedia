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
include($${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
include($${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

# Files
HEADERS += coretest.h \
    iotest.h
SOURCES += main.cpp \
    coretest.cpp \
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
FILES_UNDER_TEST = $${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    #$${LXIMEDIA_DIR}/obj/LXiStreamGl/*.o \
    $${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${LXIMEDIA_DIR}/obj/gui/*.o \
    $${LXIMEDIA_DIR}/obj/fftw/*.o

# Platform specific
unix {
    LIBS += -lXrandr \
        -lXtst \
        -lXv
    QMAKE_LFLAGS += -z \
        muldefs
    
    # OpenGL Shader Language
    #HEADERS += opengltest.h
    #SOURCES += opengltest.cpp
    #DEFINES += "ENABLE_GLSL"
    #FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/opengl/*.o
    
    # AVCodec / FFMPEG
    HEADERS += ffmpegtest.h
    SOURCES += ffmpegtest.cpp
    DEFINES += "ENABLE_FFMPEG"
    FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/ffmpeg/*.o
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
    
    # AVCodec / FFMPEG
    HEADERS += ffmpegtest.h
    SOURCES += ffmpegtest.cpp
    DEFINES += "ENABLE_FFMPEG"
    FILES_UNDER_TEST += $${LXIMEDIA_DIR}/obj/ffmpeg/*.o
}
QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}