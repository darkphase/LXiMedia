/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#ifdef __SSE__
  #include <xmmintrin.h>
#endif

void LXiStream_FFMpegBackend_AudioDecoder_postFilterDts
    (int16_t *dst, const int16_t *src, size_t numBytes, unsigned numChannels)
{
  if (numChannels == 6)
  {
    for (unsigned i=0, n=numBytes/sizeof(int16_t); i<n; i+=6)
    {
      dst[i+0] = src[i+0];
      dst[i+1] = src[i+2];
      dst[i+2] = src[i+1];
      dst[i+3] = src[i+4];
      dst[i+4] = src[i+5];
      dst[i+5] = src[i+3];
    }
  }
  else
    memcpy(dst, src, numBytes);
}

void LXiStream_FFMpegBackend_AudioDecoder_postFilterAac
    (int16_t *dst, const int16_t *src, size_t numBytes, unsigned numChannels)
{
  if (numChannels == 6)
  {
    for (unsigned i=0, n=numBytes/sizeof(int16_t); i<n; i+=6)
    {
      dst[i+0] = src[i+1];
      dst[i+1] = src[i+0];
      dst[i+2] = src[i+2];
      dst[i+3] = src[i+3];
      dst[i+4] = src[i+4];
      dst[i+5] = src[i+5];
    }
  }
  else
    memcpy(dst, src, numBytes);
}
