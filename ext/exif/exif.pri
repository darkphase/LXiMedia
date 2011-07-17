macx:INCLUDEPATH += $${PWD}/include/
macx:LIBS += -L$${PWD}/bin.macx

unix:LIBS += -lexif

win32:include($${PWD}/../gnuwin32/gnuwin32.pri)
win32:INCLUDEPATH += $${PWD}/include/
win32-g++:LIBS += -L$${PWD}/bin.win32 -lexif
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(PWD,/,\\)\\bin.win32\\libexif.a