win32 {
  INCLUDEPATH += $${LXIMEDIA_DIR}/ext/dvdnav/include/
  LIBS += -L$${LXIMEDIA_DIR}/ext/dvdnav/bin.win32
}

LIBS += -ldvdnav
win32:LIBS += -ldvdread -ldl
