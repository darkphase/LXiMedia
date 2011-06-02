TEMPLATE = app
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lxistreamfiletest
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)

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
  $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/lxistream_ffmpeg/*.o

# Files
HEADERS += filetester.h
SOURCES += main.cpp \
    filetester.cpp
LIBS += -lbfd \
    -liberty

# Platform specific
unix {
    !macx {
        LIBS += -lX11 \
            -lXext \
            -lXrandr \
            -lXtst \
            -lXv
    }

    QMAKE_LFLAGS += -z \
        muldefs
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
