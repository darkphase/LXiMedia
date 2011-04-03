# Linking the lxicore library
LXICORE_VERSION_MAJOR = $$system(cat ../../VERSION)
LXICORE_VERSION_MAJOR ~= s/\\.[0-9]+.+/

!contains(INCLUDEPATH, $${LXIMEDIA_DIR}/include/) {
  INCLUDEPATH += $${LXIMEDIA_DIR}/include/
  DEPENDPATH += $${LXIMEDIA_DIR}/include/
}
    
!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiCore) {
    unix {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/libLXiCore.so
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiCore
    }

    win32-g++ {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiCore$${LXICORE_VERSION_MAJOR}.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiCore$${LXICORE_VERSION_MAJOR}
    }
  }
}
