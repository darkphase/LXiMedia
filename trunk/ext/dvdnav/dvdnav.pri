win32 {
  INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/ext/dvdnav/include/
  LIBS += -L$${PWD}/$${LXIMEDIA_DIR}/ext/dvdnav/bin.win32
}

LIBS += -ldvdnav
win32:LIBS += -ldvdread -ldl
