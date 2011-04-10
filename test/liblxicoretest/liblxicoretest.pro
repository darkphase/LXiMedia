TEMPLATE = app
CONFIG += qtestlib
QT -= gui
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lxicoretest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

unix {
  # Prevent dependency with .so files
  QMAKE_LFLAGS += $${LXIMEDIA_DIR}/obj/LXiCore/*.o
  POST_TARGETDEPS += $${LXIMEDIA_DIR}/obj/LXiCore/*.o
} else {
  include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
}

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiCore
}

# Files
HEADERS += coretest.h
SOURCES += main.cpp \
    coretest.cpp
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Platform specific
unix {
    QMAKE_LFLAGS += -z \
        muldefs
}
win32 { 
    CONFIG += console
}
win32-g++ {
    system(mkdir ..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${LXIMEDIA_DIR}/bin)
    release {
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtTest4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtTestd4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}
