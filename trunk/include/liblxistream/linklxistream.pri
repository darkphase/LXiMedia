# Linking the lxistream library
include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)

LXISTREAM_VERSION_MAJOR = $$system(cat ../../VERSION)
LXISTREAM_VERSION_MAJOR ~= s/\\.[0-9]+.+/

!contains(INCLUDEPATH, $${LXIMEDIA_DIR}/include/) {
  INCLUDEPATH += $${LXIMEDIA_DIR}/include/
  DEPENDPATH += $${LXIMEDIA_DIR}/include/
}
    
!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStream) {
    unix {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/libLXiStream.so
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiStream
    }

    win32-g++ {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiStream$${LXISTREAM_VERSION_MAJOR}.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiStream$${LXISTREAM_VERSION_MAJOR}
    }
  }
}
