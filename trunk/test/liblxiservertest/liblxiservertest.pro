TEMPLATE = app
CONFIG += qtestlib
QT += network \
    xml
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = liblxiservertest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

# Files
HEADERS += httpservertest.h \
    sandboxtest.h
SOURCES += main.cpp \
    httpservertest.cpp \
    sandboxtest.cpp

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
#win32:QMAKE_POST_LINK = $(DESTDIR)$(TARGET) -silent

# Prevent dependency with .so files
FILES_UNDER_TEST = $${LXIMEDIA_DIR}/obj/LXiServer/*.o

# Platform specific
unix { 
}
win32 { 
    CONFIG += console
    LIBS += -lws2_32
}

QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}
