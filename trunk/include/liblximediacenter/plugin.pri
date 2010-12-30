################################################################################
# Plugin build description                                                     #
################################################################################

TEMPLATE = lib
CONFIG += plugin qt thread warn_on
DESTDIR = $${LXIMEDIA_DIR}/bin/lximediacenter
TARGET = $${PLUGIN_NAME}
DEFINES += PLUGIN_NAME=$${PLUGIN_NAME}
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

unix {
    target.path = /usr/lib/lximediacenter
    INSTALLS += target
}
