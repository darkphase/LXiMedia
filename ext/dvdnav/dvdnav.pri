win32 {
  INCLUDEPATH += $${PWD}/include/
  LIBS += -L$${PWD}/bin.win32
}

LIBS += -ldvdnav
win32:LIBS += -ldvdread -ldl
