TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiCore
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxicore
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblxicore

linux-g++|win32-g++ {
  # Generate/Use precompiled header
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiCore
}

# Files
HEADERS += $${LXIMEDIA_DIR}/include/LXiCore \
    $${LXIMEDIA_DIR}/include/liblxicore/sapplication.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sdaemon.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sfactory.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sfactory.hpp \
    $${LXIMEDIA_DIR}/include/liblxicore/sglobal.h \
    $${LXIMEDIA_DIR}/include/liblxicore/smodule.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sscheduler.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sscheduler.hpp \
    $${LXIMEDIA_DIR}/include/liblxicore/sserializable.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sstringparser.h
SOURCES += sapplication.cpp \
    sapplication.exchandler.cpp \
    sapplication.log.cpp \
    sfactory.cpp \
    smodule.cpp \
    sscheduler.cpp \
    sserializable.cpp \
    sstringparser.cpp \
    sstringparser.iso639.cpp
LIBS += -lbfd \
    -liberty

# Platform specific
unix {
    SOURCES += sdaemon.unix.cpp

    target.path = /usr/lib
    INSTALLS += target
}
win32 {
    SOURCES += sdaemon.win.cpp
}
win32-g++ { 
    system(mkdir ..\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${LXIMEDIA_DIR}/bin)
    release { 
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}
