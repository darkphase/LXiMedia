# Linking the lximediacenter library
include($${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

LXIMEDIACENTER_VERSION_MAJOR = $$system(cat ../../VERSION)
LXIMEDIACENTER_VERSION_MAJOR ~= s/\\.[0-9]+.+/

!contains(LIBS, -lLXiMediaCenter) {
  QT += sql

  !contains(INCLUDEPATH, $${LXIMEDIA_DIR}/include/) {
    INCLUDEPATH += $${LXIMEDIA_DIR}/include/
    DEPENDPATH += $${LXIMEDIA_DIR}/include/
  }

  unix {
    POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/libLXiMediaCenter.so
    LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiMediaCenter
  }

  win32 {
    POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiMediaCenter$${LXIMEDIACENTER_VERSION_MAJOR}.dll
    LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiMediaCenter$${LXIMEDIACENTER_VERSION_MAJOR}
  }
}
