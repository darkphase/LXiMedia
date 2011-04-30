unix:LIBS += -lfftw3f
win32:INCLUDEPATH += $${PWD}/include/
win32-g++:LIBS += -L$${PWD}/bin.win32 -lfftw3f
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(PWD,/,\\)\\bin.win32\\libfftw3f.a
win32-msvc2005|win32-msvc2008|win32-msvc2010:SOURCES += $$replace(PWD,/,\\)\\..\\msvc_compat.c
