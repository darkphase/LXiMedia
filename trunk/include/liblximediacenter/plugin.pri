################################################################################
# Plugin build description                                                     #
################################################################################

TEMPLATE = lib
CONFIG += plugin qt thread warn_on
DESTDIR = $${LXIMEDIA_DIR}/bin/lximediacenter
TARGET = $${PLUGIN_NAME}
include($${LXIMEDIA_DIR}/include/config.pri)

unix {
    target.path = /usr/lib/lximediacenter
    INSTALLS += target
}

linux-g++|win32-g++ {
  # To make precompiled headers work properly
  QMAKE_CXXFLAGS += -UQT_PLUGIN
}
