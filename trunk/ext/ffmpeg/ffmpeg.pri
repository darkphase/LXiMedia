FFMPEG_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/ffmpeg

macx:INCLUDEPATH += $${FFMPEG_DIR}/include/
macx:LIBS += -L$${FFMPEG_DIR}/bin.macx

unix:LIBS += -lavformat -lavcodec -lavutil -lswscale -lmp3lame

win32:include($${PWD}/../gnuwin32/gnuwin32.pri)
win32:INCLUDEPATH += $${FFMPEG_DIR}/include/
win32-g++:LIBS += -L$${FFMPEG_DIR}/bin.win32 -lavformat -lavcodec -lavutil -lswscale -lmp3lame -lws2_32
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(FFMPEG_DIR,/,\\)\\bin.win32\\libavformat.a $$replace(FFMPEG_DIR,/,\\)\\bin.win32\\libavcodec.a $$replace(FFMPEG_DIR,/,\\)\\bin.win32\\libavutil.a $$replace(FFMPEG_DIR,/,\\)\\bin.win32\\libswscale.a $$replace(FFMPEG_DIR,/,\\)\\bin.win32\\libmp3lame.a ws2_32.lib

win32-g++ {
  QMAKE_LFLAGS += -Wl,-allow-multiple-definition
}
