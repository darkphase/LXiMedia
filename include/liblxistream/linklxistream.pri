# Linking the lxistream library
include($${PWD}/../liblxicore/linklxicore.pri)

!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStream) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiStream.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStream
    }
    win32 {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiStream.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStream
    }
  }
}
