unix {
  LIBS += -lfftw3f
}

win32 {
  INCLUDEPATH += $${PWD}/include/
  LIBS += -L$${PWD}/bin.win32 -lfftw3f
}
