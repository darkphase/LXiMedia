unix {
  LIBS += -lexif
}

win32 {
  INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/ext/exif/include/
  LIBS += -L$${PWD}/$${LXIMEDIA_DIR}/ext/exif/bin.win32 -lexif
}
