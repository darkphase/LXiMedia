unix {
  contains(QMAKE_HOST.os, Linux) {
    INCLUDEPATH += /usr/include/freetype2
  }

  LIBS += -lfreetype
}
