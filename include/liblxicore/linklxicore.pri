# Linking the lxicore library
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
      LXICORE_VERSION_MAJOR = $$system(cat ../../VERSION)
      LXICORE_VERSION_MAJOR ~= s/\\.[0-9]+.+/
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiCore$${LXICORE_VERSION_MAJOR}.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiCore$${LXICORE_VERSION_MAJOR}
    }
    win32-msvc2005|win32-msvc2008|win32-msvc2010 {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiCore.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiCore
    }
  }
}
