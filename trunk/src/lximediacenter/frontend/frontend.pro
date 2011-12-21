TEMPLATE = app
QT += network webkit xml
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin

macx {
  TARGET = LXiMediaCenter
} else {
  TARGET = lximcfrontend
}

include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)

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
    system(copy /Y $$(QTDIR)\\bin\\QtCore4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtGui4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtNetwork4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\phonon4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtWebKit4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtXml4.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtCored4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtGuid4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtNetworkd4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\phonond4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtWebKitd4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtXmld4.dll $${OUT_DIR} > NUL)
  }
}
win32-g++ {
  system(copy /Y $$(QTDIR)\\bin\\libgcc_s_dw2-1.dll $${OUT_DIR} > NUL)
  system(copy /Y $$(QTDIR)\\bin\\mingwm10.dll $${OUT_DIR} > NUL)
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vcapp
  GUID = 3335c322-7366-11e0-ada8-f32ee8bcccba
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
