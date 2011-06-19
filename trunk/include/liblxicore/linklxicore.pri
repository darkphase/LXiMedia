# Linking the lxicore library
!contains(LINKED, LXiCore) {
  LINKED += LXiCore

  !contains(CONFIG, staticlib) {
    unix|win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiCore
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiCore.lib
  }
}
