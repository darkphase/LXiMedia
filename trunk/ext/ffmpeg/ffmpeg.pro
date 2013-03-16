TEMPLATE = subdirs
CONFIG += ordered
include($${PWD}/../../include/platform.pri)
include($${PWD}/ffmpeg-version.pri)

macx {
  system(mkdir -p $${OUT_PWD})

  !exists($${OUT_PWD}/lame-$${LAME_VERSION}/libmp3lame/.libs/libmp3lame.a) {
    # Extract
    system(cp $${PWD}/lame_$${LAME_VERSION}.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf lame_$${LAME_VERSION}.orig.tar.bz2)

    system(mkdir -p $${OUT_PWD}/lame-$${LAME_VERSION}/include/lame)
    system(cp $${OUT_PWD}/lame-$${LAME_VERSION}/include/lame.h $${OUT_PWD}/lame-$${LAME_VERSION}/include/lame)

    # Compile
    system(cd $${OUT_PWD}/lame-$${LAME_VERSION} && sh configure --enable-static --disable-shared CFLAGS=\"-w $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/lame-$${LAME_VERSION} && make -j $${PLATFORM_NUMCORES})
  }

  !exists($${OUT_PWD}/x264-$${X264_VERSION}/libx264.a) {
    # Extract
    system(cp $${PWD}/x264_$${X264_VERSION}.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf x264_$${X264_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/x264-$${X264_VERSION} && sh configure --enable-static --disable-shared --disable-thread --extra-cflags=\"-w $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/x264-$${X264_VERSION} && make -j $${PLATFORM_NUMCORES})
  }

  !exists($${OUT_PWD}/libav-$${LIBAV_VERSION}/libavcodec/libavcodec.a) {
    # Extract
    system(cp $${PWD}/libav_$${LIBAV_VERSION}.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf libav_$${LIBAV_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/libav-$${LIBAV_VERSION} && sh configure --enable-gpl --enable-version3 --disable-ffmpeg --disable-avconv --disable-avplay --disable-avprobe --disable-avserver --disable-avdevice --enable-swscale --enable-network --disable-debug --disable-zlib --disable-bzlib --disable-pthreads --enable-libmp3lame --enable-libx264 --enable-runtime-cpudetect --extra-cflags=\"-I../lame-$${LAME_VERSION}/include/ -I../x264-$${X264_VERSION}/ -w\" --extra-ldflags=\"-L../lame-$${LAME_VERSION}/libmp3lame/.libs/ -L../x264-$${X264_VERSION}/\")
    system(cd $${OUT_PWD}/libav-$${LIBAV_VERSION} && make -j $${PLATFORM_NUMCORES})
  }
}
win32 {
  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)

  !exists($${OUT_PWD}/lame-$${LAME_VERSION}/libmp3lame/.libs/libmp3lame.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\lame_$${LAME_VERSION}.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf lame_$${LAME_VERSION}.orig.tar.bz2)

    system(mkdir $$replace(OUT_PWD,/,\\)\\lame-$${LAME_VERSION}\\include\\lame > NUL 2>&1)
    system(copy /Y $$replace(OUT_PWD,/,\\)\\lame-$${LAME_VERSION}\\include\\lame.h $$replace(OUT_PWD,/,\\)\\lame-$${LAME_VERSION}\\include\\lame > NUL)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\lame-$${LAME_VERSION} && sh configure --enable-static --disable-shared CFLAGS=\"-w $${PLATFORM_CFLAGS}\")
    system(cd $$replace(OUT_PWD,/,\\)\\lame-$${LAME_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES} MAKE=mingw32-make)
  }

  !exists($${OUT_PWD}/x264-$${X264_VERSION}/libx264.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\x264_$${X264_VERSION}.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf x264_$${X264_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\x264-$${X264_VERSION} && sh configure --enable-static --disable-shared --disable-thread --extra-cflags=\"-w $${PLATFORM_CFLAGS}\")
    system(cd $$replace(OUT_PWD,/,\\)\\x264-$${X264_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
  }

  !exists($${OUT_PWD}/libav-$${LIBAV_VERSION}/libavcodec/libavcodec.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\libav_$${LIBAV_VERSION}.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf libav_$${LIBAV_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\libav-$${LIBAV_VERSION} && sh configure --enable-gpl --enable-version3 --disable-ffmpeg --disable-avconv --disable-avplay --disable-avprobe --disable-avserver --disable-avdevice --enable-swscale --enable-network --disable-debug --disable-zlib --disable-bzlib --disable-pthreads --disable-w32threads --disable-encoder=h263 --enable-libmp3lame --enable-libx264 --enable-runtime-cpudetect --extra-cflags=\"-I../lame-$${LAME_VERSION}/include/ -I../x264-$${X264_VERSION}/ -w\" --extra-ldflags=\"-L../lame-$${LAME_VERSION}/libmp3lame/.libs/ -L../x264-$${X264_VERSION}/\")
    system(cd $$replace(OUT_PWD,/,\\)\\libav-$${LIBAV_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
  }
}
