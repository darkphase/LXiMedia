# Linking the lxicore library
!contains(CONFIG, staticlib) {
  !contains(LINKED, LXiCore) {
    LINKED += LXiCore
  
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiCore.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiCore
    }
    
    win32:POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiCore.dll
    win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiCore
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiCore.lib
  }
}
