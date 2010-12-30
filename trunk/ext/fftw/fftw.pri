unix {
  LIBS += -lfftw3f
}

win32 {
  INCLUDEPATH += $${LXIMEDIA_DIR}/ext/fftw/include/
  LIBS += -L$${LXIMEDIA_DIR}/ext/fftw/bin.win32 -lfftw3f
}
