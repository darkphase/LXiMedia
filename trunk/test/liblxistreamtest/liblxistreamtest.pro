TEMPLATE = app
CONFIG += qtestlib
Qt -= gui
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lxistreamtest
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

unix {
  # Prevent dependency with .so files
  QMAKE_LFLAGS += $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o

  POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o
} else {
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
}

FILES_UNDER_TEST = $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/lxistream_dvdnav/*.o \
  $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/lxistream_ffmpeg/*.o \
  $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/lxistream_fftw/*.o \
  $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/lxistream_gui/*.o

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
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

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

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\QtTest4.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtTestd4.dll $${OUT_DIR} > NUL)
  }
}
