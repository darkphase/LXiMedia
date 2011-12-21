TEMPLATE = subdirs
CONFIG += ordered

FFMPEG_VERSION = libav-0.7.2
FFMPEG_HEADERS = $${FFMPEG_VERSION}/libavcodec/avcodec.h \
 $${FFMPEG_VERSION}/libavcodec/version.h \
 $${FFMPEG_VERSION}/libavformat/avformat.h \
 $${FFMPEG_VERSION}/libavformat/avio.h \
 $${FFMPEG_VERSION}/libavformat/version.h \
 $${FFMPEG_VERSION}/libavutil/attributes.h \
 $${FFMPEG_VERSION}/libavutil/audioconvert.h \
 $${FFMPEG_VERSION}/libavutil/avutil.h \
 $${FFMPEG_VERSION}/libavutil/common.h \
 $${FFMPEG_VERSION}/libavutil/cpu.h \
 $${FFMPEG_VERSION}/libavutil/dict.h \
 $${FFMPEG_VERSION}/libavutil/error.h \
 $${FFMPEG_VERSION}/libavutil/intfloat_readwrite.h \
 $${FFMPEG_VERSION}/libavutil/log.h \
 $${FFMPEG_VERSION}/libavutil/mathematics.h \
 $${FFMPEG_VERSION}/libavutil/mem.h \
 $${FFMPEG_VERSION}/libavutil/pixfmt.h \
 $${FFMPEG_VERSION}/libavutil/samplefmt.h \
 $${FFMPEG_VERSION}/libavutil/rational.h \
 $${FFMPEG_VERSION}/libswscale/swscale.h

macx {
  system(mkdir -p $${OUT_PWD}/bin.macx)
  system(bzip2 -cdk $${PWD}/bin.macx/libavcodec.a.bz2 > $${OUT_PWD}/bin.win32/libavcodec.a)
  system(bzip2 -cdk $${PWD}/bin.macx/libavformat.a.bz2 > $${OUT_PWD}/bin.win32/libavformat.a)
  system(bzip2 -cdk $${PWD}/bin.macx/libavutil.a.bz2 > $${OUT_PWD}/bin.win32/libavutil.a)
  system(bzip2 -cdk $${PWD}/bin.macx/libswscale.a.bz2 > $${OUT_PWD}/bin.win32/libswscale.a)
  system(bzip2 -cdk $${PWD}/bin.macx/libmp3lame.a.bz2 > $${OUT_PWD}/bin.win32/libmp3lame.a)

  system(bzip2 -cdk $${PWD}/libav_0.7.2.orig.tar.bz2 > $${OUT_PWD}/libav_0.7.2.orig.tar)
  system(cd $${OUT_PWD} && tar -x -f libav_0.7.2.orig.tar $${FFMPEG_HEADERS})
  system(cd $${OUT_PWD} && rm -rf include)
  system(cd $${OUT_PWD} && mv $${FFMPEG_VERSION} include)
  system(rm $${OUT_PWD}/libav_0.7.2.orig.tar)
  system(cp bin.macx/avconfig.h $${OUT_PWD}/include/libavutil/)
}
win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system(mkdir $$replace(OUT_PWD,/,\\)\\bin.win32 > NUL 2>&1)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libavcodec.a.bz2 > $${OUT_PWD}/bin.win32/libavcodec.a)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libavformat.a.bz2 > $${OUT_PWD}/bin.win32/libavformat.a)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libavutil.a.bz2 > $${OUT_PWD}/bin.win32/libavutil.a)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libswscale.a.bz2 > $${OUT_PWD}/bin.win32/libswscale.a)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libmp3lame.a.bz2 > $${OUT_PWD}/bin.win32/libmp3lame.a)

  system($${BZIP2} -cdk $${PWD}/libav_0.7.2.orig.tar.bz2 > $${OUT_PWD}/libav_0.7.2.orig.tar)
  system(cd $$replace(OUT_PWD,/,\\) && $${TAR} -x -f libav_0.7.2.orig.tar $${FFMPEG_HEADERS})
  system(cd $$replace(OUT_PWD,/,\\) && del /S /Q include > NUL 2>&1 && rmdir /S /Q include > NUL 2>&1)
  system(cd $$replace(OUT_PWD,/,\\) && ren $${FFMPEG_VERSION} include > NUL)
  system(del /S /Q $$replace(OUT_PWD,/,\\)\\libav_0.7.2.orig.tar > NUL 2>&1)
  system(copy /Y bin.win32\\avconfig.h $$replace(OUT_PWD,/,\\)\\include\\libavutil\\ > NUL)
}
