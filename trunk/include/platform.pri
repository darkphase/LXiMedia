PLATFORM_NUMCORES = 1
PLATFORM_CFLAGS =

win32:PLATFORM_NUMCORES = $$(NUMBER_OF_PROCESSORS)

unix|win32-g++ {
  contains(QMAKE_HOST.os, Linux) {
    PLATFORM_NUMCORES = system(cat /proc/cpuinfo | grep processor | wc -l)
  }

  # All floating point operations are to be performed on the SSE2 unit
  PLATFORM_CFLAGS += -mtune=generic -mmmx -msse -msse2 -mfpmath=sse

  !contains(QMAKE_HOST.arch, x86_64) {
    PLATFORM_CFLAGS += -march=i686 -D_FILE_OFFSET_BITS=64
    macx:PLATFORM_CFLAGS += -msse3
  } else {
    macx:PLATFORM_CFLAGS += -msse3 -mssse3
  }
}
