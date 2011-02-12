################################################################################
# Plugin build description                                                     #
################################################################################

TEMPLATE = lib
CONFIG += plugin
DEFINES += PLUGIN_NAME=$${PLUGIN_NAME}
CONFIG += qt thread warn_on
QT -= gui
DESTDIR = $${LXIMEDIA_DIR}/bin/liblxistream
TARGET = $${PLUGIN_NAME}
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

unix {
    target.path = /usr/lib/liblxistream
    INSTALLS += target
}
