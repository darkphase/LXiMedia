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

unix:LIBS += -lavformat -lavcodec -lavutil -lswscale
win32:INCLUDEPATH += $${PWD}/include/
win32-g++:LIBS += -L$${PWD}/bin.win32 -lavformat -lavcodec -lavutil -lswscale -lws2_32
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(PWD,/,\\)\\bin.win32\\libavformat.a $$replace(PWD,/,\\)\\bin.win32\\libavcodec.a $$replace(PWD,/,\\)\\bin.win32\\libavutil.a $$replace(PWD,/,\\)\\bin.win32\\libswscale.a ws2_32.lib
win32-msvc2005|win32-msvc2008|win32-msvc2010:SOURCES += $$replace(PWD,/,\\)\\..\\msvc_compat.c
