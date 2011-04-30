# Linking the lximediacenter library
include($${PWD}/../liblxicore/linklxicore.pri)
include($${PWD}/../liblxiserver/linklxiserver.pri)
include($${PWD}/../liblxistream/linklxistream.pri)
include($${PWD}/../liblxistreamgui/linklxistreamgui.pri)

!contains(LIBS, -lLXiMediaCenter) {
  QT += sql

  !contains(INCLUDEPATH, $${PWD}/$${LXIMEDIA_DIR}/include/) {
    INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/
    DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/
  }

  unix {
    POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiMediaCenter.so
    LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiMediaCenter
  }
  win32 {
    POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiMediaCenter.dll
    LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiMediaCenter
  }
}
