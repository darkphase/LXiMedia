FFMPEG_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/ffmpeg
include($${PWD}/ffmpeg-version.pri)

win32-g++|unix {
  INCLUDEPATH += $${FFMPEG_DIR}/libav-$${LIBAV_VERSION}/

  LIBS += \
    -L$${FFMPEG_DIR}/libav-$${LIBAV_VERSION}/libavformat/ \
    -L$${FFMPEG_DIR}/libav-$${LIBAV_VERSION}/libavcodec/ \
    -L$${FFMPEG_DIR}/libav-$${LIBAV_VERSION}/libavutil/ \
    -L$${FFMPEG_DIR}/libav-$${LIBAV_VERSION}/libswscale/

  !contains(QMAKE_HOST.os, Linux) {
    LIBS += \
      -L$${FFMPEG_DIR}/lame-$${LAME_VERSION}/libmp3lame/.libs/ \
      -L$${FFMPEG_DIR}/x264-$${X264_VERSION}/
  }
}

unix:LIBS += -lavformat -lavcodec -lavutil -lswscale -lmp3lame -lx264
unix:contains(QMAKE_HOST.os, Linux):QMAKE_LFLAGS += -Wl,-Bsymbolic
win32-g++:LIBS += -lavformat -lavcodec -lavutil -lswscale -lmp3lame -lx264 -lws2_32
win32-g++:QMAKE_LFLAGS += -Wl,-allow-multiple-definition

win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += \
  $$replace(FFMPEG_DIR,/,\\)\\libav-$${LIBAV_VERSION}\\libavformat\\libavformat.a \
  $$replace(FFMPEG_DIR,/,\\)\\libav-$${LIBAV_VERSION}\\libavcodec\\libavcodec.a \
  $$replace(FFMPEG_DIR,/,\\)\\libav-$${LIBAV_VERSION}\\libavutil\\libavutil.a \
  $$replace(FFMPEG_DIR,/,\\)\\libav-$${LIBAV_VERSION}\\libswscale\\libswscale.a \
  $$replace(FFMPEG_DIR,/,\\)\\lame-$${LAME_VERSION}\\libmp3lame\\.libs\\libmp3lame.a \
  $$replace(FFMPEG_DIR,/,\\)\\x264-$${X264_VERSION}\\x264.a \
  ws2_32.lib
