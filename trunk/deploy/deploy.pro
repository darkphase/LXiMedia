TEMPLATE = subdirs
CONFIG += ordered

macx {
  release {
    OUT_DIR = $${OUT_PWD}/../bin

    system(mkdir -p $${OUT_DIR})
    system(cp $${PWD}/../VERSION $${OUT_DIR})
    system(cp $${PWD}/macx/*.sh $${OUT_DIR})
  }
}
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\..\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  system(copy /Y $$replace(PWD,/,\\)\\..\\COPYING $${OUT_DIR} > NUL)
  system(copy /Y $$replace(PWD,/,\\)\\..\\README $${OUT_DIR} > NUL)
  system(copy /Y $$replace(PWD,/,\\)\\..\\VERSION $${OUT_DIR} > NUL)

  release {
    system(copy /Y $$replace(PWD,/,\\)\\win32\\*.nsi $${OUT_DIR} > NUL)
  }
}
