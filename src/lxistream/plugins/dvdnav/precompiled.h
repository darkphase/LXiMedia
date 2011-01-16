// This header file is precompiled to speed up the build process. Only put 
// system headers in this file to prevent ugly issues with compile dependencies.
#include "../../liblxistream/precompiled.h"

#ifdef __cplusplus

#include <QtNetwork>
#include <QtXml>

extern "C"
{
#define UINT64_C Q_UINT64_C
#define INT64_C Q_INT64_C
#ifndef USE_FFMPEG_OLD_PATH
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#else
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
#endif
}

#endif
