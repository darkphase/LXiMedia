unix {
  LIBS += -lexif
}

win32 {
  INCLUDEPATH += $${LXIMEDIA_DIR}/ext/exif/include/
  LIBS += -L$${LXIMEDIA_DIR}/ext/exif/bin.win32 -lexif
}
