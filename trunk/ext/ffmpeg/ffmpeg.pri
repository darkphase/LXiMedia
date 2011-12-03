macx:INCLUDEPATH += $${PWD}/include/
macx:LIBS += -L$${PWD}/bin.macx

unix:LIBS += -lavformat -lavcodec -lavutil -lswscale

win32:include($${PWD}/../gnuwin32/gnuwin32.pri)
win32:INCLUDEPATH += $${PWD}/include/
win32-g++:LIBS += -L$${PWD}/bin.win32 -lavformat -lavcodec -lavutil -lswscale -lws2_32
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(PWD,/,\\)\\bin.win32\\libavformat.a $$replace(PWD,/,\\)\\bin.win32\\libavcodec.a $$replace(PWD,/,\\)\\bin.win32\\libavutil.a $$replace(PWD,/,\\)\\bin.win32\\libswscale.a ws2_32.lib

win32-g++ {
  QMAKE_LFLAGS += -Wl,-allow-multiple-definition
}
