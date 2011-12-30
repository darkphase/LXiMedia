unix:LIBS += -lsmbclient

!contains(QMAKE_HOST.arch, x86_64) {
  DEFINES += _FILE_OFFSET_BITS=64
}
