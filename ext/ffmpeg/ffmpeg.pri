win32 {
  INCLUDEPATH += $${PWD}/include/
  LIBS += -L$${PWD}/bin.win32
}

win32-g++ {
  QMAKE_LFLAGS += -Wl,-allow-multiple-definition

  # Required for 32-bit Windows/MingW to prevent crashing SSE code on unaligned
  # stack data.
  QMAKE_CXXFLAGS += -mstackrealign
}

unix {
  exists( /usr/include/ffmpeg/avcodec.h ) {
    DEFINES += USE_FFMPEG_OLD_PATH
  }
  exists( /usr/local/include/ffmpeg/avcodec.h ) {
    DEFINES += USE_FFMPEG_OLD_PATH
  }
}

LIBS += -lavformat -lavcodec -lavutil -lswscale
win32:LIBS += -lws2_32
