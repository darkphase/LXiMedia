unix {
  LIBS += -lfftw3f
}

win32 {
  INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/ext/fftw/include/
  LIBS += -L$${PWD}/$${LXIMEDIA_DIR}/ext/fftw/bin.win32 -lfftw3f
}
