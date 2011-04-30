# Linking the lxiserver library
include($${PWD}/../liblxicore/linklxicore.pri)
    
!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiServer) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiServer.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer
    }
    win32 {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiServer.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer
    }
  }
}
