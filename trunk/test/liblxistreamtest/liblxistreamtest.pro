TEMPLATE = app
QT += testlib
win32:CONFIG += console
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lxistreamtest
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Files
HEADERS += streamtest.h \
    dvdnavtest.h \
    ffmpegtest.h \
    filetester.h \
    iotest.h
SOURCES += main.cpp \
    streamtest.cpp \
    dvdnavtest.cpp \
    ffmpegtest.cpp \
    filetester.cpp \
    iotest.cpp
RESOURCES = test.qrc

# Performance test
HEADERS += performancetest.h
SOURCES += performancetest.cpp

include($${PWD}/$${LXIMEDIA_DIR}/src/lxistream/algorithms/linkalgorithms.pri)

# Run tests after link
unix:QMAKE_LFLAGS += -Wl,-rpath -Wl,.
unix:QMAKE_POST_LINK = cd $${DESTDIR} && ./$${TARGET} -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

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
    TEMPLATE = vcapp
    GUID = dfcf438e-3a3e-11e1-bb9f-67ca06b08532
    DEFINES += _CRT_SECURE_NO_WARNINGS
  }
}
