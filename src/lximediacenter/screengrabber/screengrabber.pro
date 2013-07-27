TEMPLATE = app
QT += network widgets
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin

TARGET = lximcscreengrabber

include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

# Files
SOURCES += main.cpp \
 screengrabber.cpp
HEADERS += screengrabber.h
RESOURCES = ../resources/screengrabber.qrc

unix {
  !macx {
    target.path = /usr/bin
    INSTALLS += target
  }
}

win32 {
  LIBS += -lws2_32
  CONFIG += windows
  RC_FILE = screengrabber.rc
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)

  release {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Gui.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Network.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Widgets.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\libGLESv2.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Guid.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Networkd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Widgetsd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\libGLESv2d.dll $${OUT_DIR} > NUL)
  }

  system(copy /Y $$(QTDIR)\\bin\\D3DCompiler_43.dll $${OUT_DIR} > NUL)

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vcapp
    GUID = 6ba196aa-1617-11e2-9c6a-002421558ad4
    DEFINES += _CRT_SECURE_NO_WARNINGS
  }
}
