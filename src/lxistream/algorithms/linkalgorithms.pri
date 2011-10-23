# Linking the algorithms library
!contains(LINKED, LXIStreamAlgorithms) {
  LINKED += LXIStreamAlgorithms

  unix|win32-g++ {
    LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lalgorithms
    POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libalgorithms.a
  }

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\algorithms.lib
    POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/algorithms.lib
  }
}
