TEMPLATE = subdirs
CONFIG += ordered

FFMPEG_VERSION = ffmpeg-0.5.2
FFMPEG_HEADERS = $${FFMPEG_VERSION}/libavcodec/avcodec.h \
 $${FFMPEG_VERSION}/libavformat/avformat.h \
 $${FFMPEG_VERSION}/libavformat/avio.h \
 $${FFMPEG_VERSION}/libavutil/avutil.h \
 $${FFMPEG_VERSION}/libavutil/common.h \
 $${FFMPEG_VERSION}/libavutil/intfloat_readwrite.h \
 $${FFMPEG_VERSION}/libavutil/log.h \
 $${FFMPEG_VERSION}/libavutil/mathematics.h \
 $${FFMPEG_VERSION}/libavutil/mem.h \
 $${FFMPEG_VERSION}/libavutil/pixfmt.h \
 $${FFMPEG_VERSION}/libavutil/rational.h \
 $${FFMPEG_VERSION}/libswscale/swscale.h

win32 {
    system(bzip2 -fdk /$$replace(PWD,:,/)/bin.win32/libavcodec.a.bz2)
    system(bzip2 -fdk /$$replace(PWD,:,/)/bin.win32/libavformat.a.bz2)
    system(bzip2 -fdk /$$replace(PWD,:,/)/bin.win32/libavutil.a.bz2)
    system(bzip2 -fdk /$$replace(PWD,:,/)/bin.win32/libswscale.a.bz2)
    system(bzip2 -cdk /$$replace(PWD,:,/)/ffmpeg_0.5.2.orig.tar.bz2 | tar -x $${FFMPEG_HEADERS} && rm -rf include && mv $${FFMPEG_VERSION} include)
}
