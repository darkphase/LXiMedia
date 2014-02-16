TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiMediaCenter
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter

DEFINES += S_BUILD_LIBLXIMEDIACENTER

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/pupnp/pupnp.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/export.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/backendserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/client.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/connectionmanager.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/contentdirectory.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/mediaprofiles.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/mediareceiverregistrar.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/mediaserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/mediastream.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/rootdevice.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/upnp.h

SOURCES += backendserver.cpp \
 client.cpp \
 connectionmanager.cpp \
 contentdirectory.cpp \
 mediaprofiles.cpp \
 mediareceiverregistrar.cpp \
 mediaserver.cpp \
 mediaserver.html.cpp \
 mediastream.cpp \
 rootdevice.cpp \
 upnp.cpp

HEADERS += lximediacenterprivate.h \
 ixmlstructures.h
SOURCES += lximediacenterprivate.cpp \
 ixmlstructures.cpp

unix {
  LIBS += -lz

  !macx {
    target.path = /usr/lib
    INSTALLS += target
  }
}

win32 {
  LIBS += -lws2_32
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)

  release {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Gui.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Test.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\libGLESv2.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Guid.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Testd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\libGLESv2d.dll $${OUT_DIR} > NUL)
  }

  system(copy /Y $$(QTDIR)\\bin\\D3DCompiler_43.dll $${OUT_DIR} > NUL)

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = f4e525ea-7365-11e0-aef3-cba632b16f97
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
  }
}
