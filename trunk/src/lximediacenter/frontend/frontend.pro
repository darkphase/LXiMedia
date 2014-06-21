TEMPLATE = app
QT += webkitwidgets
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin

macx {
  TARGET = LXiMediaCenter
} else {
  TARGET = lximcfrontend
}

include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

macx {
  CONFIG += app_bundle
  ICON = $${PWD}/$${LXIMEDIA_DIR}/src/lxicore/liblxicore/lximedia.icns
}

# Files
SOURCES += main.cpp \
 frontend.cpp \
 frontend.html.cpp
HEADERS += frontend.h
RESOURCES = ../resources/frontend.qrc

unix {
  !macx {
    target.path = /usr/bin
    INSTALLS += target
  }
}

win32 {
  LIBS += -lws2_32
  CONFIG += windows
  RC_FILE = frontend.rc
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)

  release {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Gui.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Multimedia.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5MultimediaWidgets.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Network.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5OpenGL.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Positioning.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5PrintSupport.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Qml.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Quick.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Sensors.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Sql.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5WebKit.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5WebKitWidgets.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Widgets.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Guid.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Multimediad.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5MultimediaWidgetsd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Networkd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5OpenGLd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Positioningd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5PrintSupportd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Qmld.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Quickd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Sensorsd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Sqld.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5WebKitd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5WebKitWidgetsd.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\Qt5Widgetsd.dll $${OUT_DIR} > NUL)
  }

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vcapp
    GUID = 3335c322-7366-11e0-ada8-f32ee8bcccba
    DEFINES += _CRT_SECURE_NO_WARNINGS
  }
}
