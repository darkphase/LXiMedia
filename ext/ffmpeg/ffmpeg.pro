TEMPLATE = subdirs
CONFIG += ordered

FFMPEG_VERSION = libav-0.6.2
FFMPEG_HEADERS = $${FFMPEG_VERSION}/libavcodec/avcodec.h \
 $${FFMPEG_VERSION}/libavformat/avformat.h \
 $${FFMPEG_VERSION}/libavformat/avio.h \
 $${FFMPEG_VERSION}/libavutil/attributes.h \
 $${FFMPEG_VERSION}/libavutil/avutil.h \
 $${FFMPEG_VERSION}/libavutil/common.h \
 $${FFMPEG_VERSION}/libavutil/error.h \
 $${FFMPEG_VERSION}/libavutil/intfloat_readwrite.h \
 $${FFMPEG_VERSION}/libavutil/log.h \
 $${FFMPEG_VERSION}/libavutil/mathematics.h \
 $${FFMPEG_VERSION}/libavutil/mem.h \
 $${FFMPEG_VERSION}/libavutil/pixfmt.h \
 $${FFMPEG_VERSION}/libavutil/rational.h \
 $${FFMPEG_VERSION}/libswscale/swscale.h

win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system($${BZIP2} -fdk $${PWD}/bin.win32/libavcodec.a.bz2)
  system($${BZIP2} -fdk $${PWD}/bin.win32/libavformat.a.bz2)
  system($${BZIP2} -fdk $${PWD}/bin.win32/libavutil.a.bz2)
  system($${BZIP2} -fdk $${PWD}/bin.win32/libswscale.a.bz2)

  system($${BZIP2} -fdk $${PWD}/libav_0.6.2.orig.tar.bz2)
  system($${TAR} -x -f libav_0.6.2.orig.tar $${FFMPEG_HEADERS})
  system(del /S /Q include > NUL 2>&1)
  system(rmdir /S /Q include > NUL 2>&1)
  system(ren $${FFMPEG_VERSION} include > NUL)
  system(del /S /Q libav_0.6.2.orig.tar > NUL)
  system(copy /Y bin.win32\\avconfig.h include\\libavutil\\ > NUL)
}
