unix {
  LIBS += -lexif
}

win32 {
  INCLUDEPATH += $${PWD}/include/
  LIBS += -L$${PWD}/bin.win32 -lexif
}
