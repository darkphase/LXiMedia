TEMPLATE = app
CONFIG += qtestlib
QT -= gui
QT += network xml
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lxiservertest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

unix {
  # Prevent dependency with .so files
  QMAKE_LFLAGS += $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiServer/*.o

  POST_TARGETDEPS += $${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${LXIMEDIA_DIR}/obj/LXiServer/*.o
} else {
  include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
  include($${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
}

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiServer
}

# Files
HEADERS += httpenginetest.h \
    sandboxtest.h
SOURCES += main.cpp \
    httpenginetest.cpp \
    sandboxtest.cpp
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Platform specific
unix { 
}
win32 { 
    CONFIG += console
    LIBS += -lws2_32
}
win32-g++ {
    system(mkdir ..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${LXIMEDIA_DIR}/bin)
    release {
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetwork4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtTest4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetworkd4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtTestd4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}
