# Linking the lxistream library
include($${PWD}/../liblxicore/linklxicore.pri)

LXISTREAM_VERSION_MAJOR = $$system(cat ../../VERSION)
LXISTREAM_VERSION_MAJOR ~= s/\\.[0-9]+.+/
    
!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStream) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiStream.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStream
    }

    win32-g++ {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiStream$${LXISTREAM_VERSION_MAJOR}.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStream$${LXISTREAM_VERSION_MAJOR}
    }
  }
}
