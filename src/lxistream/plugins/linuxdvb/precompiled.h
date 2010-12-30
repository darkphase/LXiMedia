// This header file is precompiled to speed up the build process. Only put 
// system headers in this file to prevent ugly issues with compile dependencies.
#include "../../liblxistream/precompiled.h"

#ifdef __cplusplus

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <linux/dvb/osd.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/net.h>

#endif
