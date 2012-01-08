TEMPLATE = subdirs
CONFIG += ordered

FREETYPE_VERSION = freetype-2.4.8
FREETYPE_HEADERS = $${FREETYPE_VERSION}/include/ft2build.h \
 $${FREETYPE_VERSION}/include/freetype/config/ftconfig.h \
 $${FREETYPE_VERSION}/include/freetype/config/ftheader.h \
 $${FREETYPE_VERSION}/include/freetype/config/ftoption.h \
 $${FREETYPE_VERSION}/include/freetype/config/ftstdlib.h \
 $${FREETYPE_VERSION}/include/freetype/freetype.h \
 $${FREETYPE_VERSION}/include/freetype/fterrdef.h \
 $${FREETYPE_VERSION}/include/freetype/fterrors.h \
 $${FREETYPE_VERSION}/include/freetype/ftimage.h \
 $${FREETYPE_VERSION}/include/freetype/ftmoderr.h \
 $${FREETYPE_VERSION}/include/freetype/ftsystem.h \
 $${FREETYPE_VERSION}/include/freetype/fttypes.h

macx {
  system(mkdir -p $${OUT_PWD}/bin.macx)
  system(bzip2 -cdk $${PWD}/bin.macx/libfreetype.a.bz2 > $${OUT_PWD}/bin.macx/libfreetype.a)

  system(bzip2 -cdk $${PWD}/freetype-2.4.8.tar.bz2 > $${OUT_PWD}/freetype-2.4.8.tar)
  system(cd $${OUT_PWD} && tar -x -f freetype-2.4.8.tar $${FREETYPE_HEADERS})
  system(cd $${OUT_PWD} && rm -rf include)
  system(cd $${OUT_PWD} && mv $${FREETYPE_VERSION}/include include)
  system(cd $${OUT_PWD} && rm -rf $${FREETYPE_VERSION})
  system(rm $${OUT_PWD}/freetype-2.4.8.tar)
}
win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system(mkdir $$replace(OUT_PWD,/,\\)\\bin.win32 > NUL 2>&1)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libfreetype.a.bz2 > $${OUT_PWD}/bin.win32/libfreetype.a)

  system($${BZIP2} -cdk $${PWD}/freetype-2.4.8.tar.bz2 > $${OUT_PWD}/freetype-2.4.8.tar)
  system(cd $$replace(OUT_PWD,/,\\) && $${TAR} -x -f freetype-2.4.8.tar $${FREETYPE_HEADERS})
  system(cd $$replace(OUT_PWD,/,\\) && del /S /Q include > NUL 2>&1)
  system(cd $$replace(OUT_PWD,/,\\) && rmdir /S /Q include > NUL 2>&1)
  system(cd $$replace(OUT_PWD,/,\\) && move $${FREETYPE_VERSION}\\include include > NUL)
  system(cd $$replace(OUT_PWD,/,\\) && rmdir /S /Q $${FREETYPE_VERSION} > NUL)
  system(del /S /Q $$replace(OUT_PWD,/,\\)\\freetype-2.4.8.tar > NUL 2>&1)
}
