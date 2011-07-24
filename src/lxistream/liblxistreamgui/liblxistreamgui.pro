TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiStreamGui
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/exif/exif.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui

DEFINES += S_BUILD_LIBLXISTREAMGUI

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamGui

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamGui \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/export.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/sdisplay.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/simage.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/svideoview.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/svumeter.h
SOURCES += sdisplay.cpp \
    simage.cpp \
    svideoview.cpp \
    svumeter.cpp

# Nodes
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/nodes/svideogeneratornode.h
SOURCES += nodes/svideogeneratornode.cpp

HEADERS += lxistreamguiprivate.h
SOURCES += lxistreamguiprivate.cpp

# Platform specific
unix { 
  macx {
    SOURCES += sdisplay.macx.cpp \
        svideoview.macx.cpp
  } else {
    SOURCES += sdisplay.x11.cpp \
        svideoview.x11.cpp
    LIBS += -lX11 \
        -lXext \
        -lXrandr \
        -lXtst \
        -lXv

    target.path = /usr/lib
    INSTALLS += target
  }
}
win32 {
  SOURCES += sdisplay.win.cpp \
    svideoview.win.cpp
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  system(mkdir $${OUT_DIR}\\imageformats > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\QtCore4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtGui4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtXml4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\imageformats\\qjpeg4.dll $${OUT_DIR}\\imageformats > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\imageformats\\qtiff4.dll $${OUT_DIR}\\imageformats > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtCored4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtGuid4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtXmld4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\imageformats\\qjpegd4.dll $${OUT_DIR}\\imageformats > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\imageformats\\qtiffd4.dll $${OUT_DIR}\\imageformats > NUL)
  }
}
win32-g++ {
  system(copy /Y $$(QTDIR)\\bin\\libgcc_s_dw2-1.dll $${OUT_DIR} > NUL)
  system(copy /Y $$(QTDIR)\\bin\\mingwm10.dll $${OUT_DIR} > NUL)
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = 4d862d4a-734b-11e0-9782-a7eb248805c9
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
