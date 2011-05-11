win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  SOURCES += $$replace(PWD,/,\\)\\codepage.c
  LIBS += $$replace(PWD,/,\\)\\libgcc.a $$replace(PWD,/,\\)\\libcoldname.a $$replace(PWD,/,\\)\\libmingwex.a
  QMAKE_LFLAGS += /IGNORE:4049 /IGNORE:4217
}
