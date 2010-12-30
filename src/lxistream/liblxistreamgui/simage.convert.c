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
#ifdef __SSE__
  #include <xmmintrin.h>
#endif

__inline int bound(int a, int b, int c) { return b < a ? a : (b > c ? c : b); }

struct YUYVPixel { uint8_t y0, u, y1, v; } __attribute__((packed));
struct UYVYPixel { uint8_t u, y0, v, y1; } __attribute__((packed));
struct RGBAPixel { uint8_t b,  g, r,  a; } __attribute__((packed));


void LXiStreamGui_SImage_convertYUYVtoRGB
 (struct RGBAPixel * restrict rgb, const struct YUYVPixel * restrict yuv, unsigned numPixels)
{
  unsigned i;

#ifdef __SSE__
  // Check alignment
  if ((numPixels % (sizeof(__m128i) / sizeof(uint32_t))) != 0)
  {
#endif
    for (i=0; i<numPixels; i+=2)
    {
      int y0 = (int)(yuv->y0);
      int u =  (int)(yuv->u);
      int y1 = (int)(yuv->y1);
      int v =  (int)(yuv->v);
      yuv++;

      // Preprocess
      y0 = (75 * (y0 - 16)) >> 6;
      u = u - 128;
      y1 = (75 * (y1 - 16)) >> 6;
      v = v - 128;

      // Extract Pixel 1
      rgb->r = bound(0, y0 + ((51 * v) >> 5), 255);
      rgb->g = bound(0, y0 - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
      rgb->b = bound(0, y0 + ((65 * u) >> 5), 255);
      rgb->a = 0xFF;
      rgb++;

      // Extract Pixel 2
      rgb->r = bound(0, y1 + ((51 * v) >> 5), 255);
      rgb->g = bound(0, y1 - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
      rgb->b = bound(0, y1 + ((65 * u) >> 5), 255);
      rgb->a = 0xFF;
      rgb++;
    }
#ifdef __SSE__
  }
  else
  {
    const int aligned = (((size_t)rgb & (size_t)15) == 0);
    const __m128i zero = _mm_setzero_si128();
    const __m128i minv = _mm_setr_epi16(0, 0, 0, 255, 0, 0, 0, 255);            // "a" component should always be 255
    const __m128i maxv = _mm_setr_epi16(255, 255, 255, 255, 255, 255, 255, 255);
    const __m128i val1 = _mm_setr_epi16(16, 128, 16, 128, 16, 128, 16, 128);    // = 0.0625f, 0.5f, 0.0625f, 0.5f Multiplied by 256
    const __m128i val2 = _mm_setr_epi16(75, 64, 75, 64, 75, 64, 75, 64);        // = 1.1643f, 1.0f, 1.1643f, 1.0f Multiplied by 64
    const __m128i val3 = _mm_setr_epi16(65, -13, 51, -26, 65, -13, 51, -26);    // = 2.017f, -0.39173f, 1.5958f, -0.81290f Multiplied by 32

    for (i=0; i<numPixels; i+=4)
    {
      __m128i px = _mm_unpacklo_epi8(_mm_loadl_epi64(yuv), zero);
      yuv += 2;

      // y0 = 1.1643f * (y0 - 0.0625f);
      // u = u - 0.5f;
      // y1 = 1.1643f * (y1 - 0.0625f);
      // v = v - 0.5f;
      px = _mm_srai_epi16(_mm_mullo_epi16(_mm_sub_epi16(px, val1), val2), 6); // Shift to divide by 64.

      // Extract pixels 0 and 2
      // r = bound(0, (int)((y0 + 1.5958f  * v)                * 255.0f), 255);
      // g = bound(0, (int)((y0 - 0.39173f * u - 0.81290f * v) * 255.0f), 255);
      // b = bound(0, (int)((y0 + 2.017f   * u)                * 255.0f), 255);
      __m128i uv = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(3, 3, 1, 1)), _MM_SHUFFLE(3, 3, 1, 1));
      uv = _mm_srai_epi16(_mm_mullo_epi16(uv, val3), 5); // Shift to divide by 32.
      uv = _mm_add_epi16(uv, _mm_slli_epi64(_mm_srli_epi64(uv, 48), 16)); // add the "-0.81290f * v" component to the "-0.39173f * u" component

      __m128i c02 = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(0, 0, 0, 0)), _MM_SHUFFLE(0, 0, 0, 0));
      c02 = _mm_max_epi16(_mm_min_epi16(_mm_add_epi16(c02, uv), maxv), minv); // bound 0 >= n >= 255, except for "a" component.

      // Extract pixels 1 and 3
      // r = bound(0, (int)((y1 + 1.5958f  * v)                * 255.0f), 255);
      // g = bound(0, (int)((y1 - 0.39173f * u - 0.81290f * v) * 255.0f), 255);
      // b = bound(0, (int)((y1 + 2.017f   * u)                * 255.0f), 255);
      __m128i c13 = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(2, 2, 2, 2)), _MM_SHUFFLE(2, 2, 2, 2));
      c13 = _mm_max_epi16(_mm_min_epi16(_mm_add_epi16(c13, uv), maxv), minv); // bound 0 >= n >= 255, except for "a" component.

      // Store data
      if (aligned)
        _mm_store_si128(rgb, _mm_shuffle_epi32(_mm_packus_epi16(c02, c13), _MM_SHUFFLE(3, 1, 2, 0)));
      else
        _mm_storeu_si128(rgb, _mm_shuffle_epi32(_mm_packus_epi16(c02, c13), _MM_SHUFFLE(3, 1, 2, 0)));

      rgb += 4;
    }
  }
#endif
}

void LXiStreamGui_SImage_convertUYVYtoRGB
 (struct RGBAPixel * restrict rgb, const struct UYVYPixel * restrict yuv, unsigned numPixels)
{
  unsigned i;

#ifdef __SSE__
  // Check alignment
  if ((numPixels % (sizeof(__m128i) / sizeof(uint32_t))) != 0)
  {
#endif
    for (i=0; i<numPixels; i+=2)
    {
      int y0 = (int)(yuv->y0);
      int u =  (int)(yuv->u);
      int y1 = (int)(yuv->y1);
      int v =  (int)(yuv->v);
      yuv++;

      // Preprocess
      y0 = (75 * (y0 - 16)) >> 6;
      u = u - 128;
      y1 = (75 * (y1 - 16)) >> 6;
      v = v - 128;

      // Extract Pixel 1
      rgb->r = bound(0, y0 + ((51 * v) >> 5), 255);
      rgb->g = bound(0, y0 - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
      rgb->b = bound(0, y0 + ((65 * u) >> 5), 255);
      rgb->a = 0xFF;
      rgb++;

      // Extract Pixel 2
      rgb->r = bound(0, y1 + ((51 * v) >> 5), 255);
      rgb->g = bound(0, y1 - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
      rgb->b = bound(0, y1 + ((65 * u) >> 5), 255);
      rgb->a = 0xFF;
      rgb++;
    }
#ifdef __SSE__
  }
  else
  {
    const int aligned = (((size_t)rgb & (size_t)15) == 0);
    const __m128i zero = _mm_setzero_si128();
    const __m128i minv = _mm_setr_epi16(0, 0, 0, 255, 0, 0, 0, 255);            // "a" component should always be 255
    const __m128i maxv = _mm_setr_epi16(255, 255, 255, 255, 255, 255, 255, 255);
    const __m128i val1 = _mm_setr_epi16(128, 16, 128, 16, 128, 16, 128, 16);    // = 0.0625f, 0.5f, 0.0625f, 0.5f Multiplied by 256
    const __m128i val2 = _mm_setr_epi16(64, 75, 64, 75, 64, 75, 64, 75);        // = 1.1643f, 1.0f, 1.1643f, 1.0f Multiplied by 64
    const __m128i val3 = _mm_setr_epi16(65, -13, 51, -26, 65, -13, 51, -26);    // = 2.017f, -0.39173f, 1.5958f, -0.81290f Multiplied by 32

    for (i=0; i<numPixels; i+=4)
    {
      __m128i px = _mm_unpacklo_epi8(_mm_loadl_epi64(yuv), zero);
      yuv += 2;

      // y0 = 1.1643f * (y0 - 0.0625f);
      // u = u - 0.5f;
      // y1 = 1.1643f * (y1 - 0.0625f);
      // v = v - 0.5f;
      px = _mm_srai_epi16(_mm_mullo_epi16(_mm_sub_epi16(px, val1), val2), 6); // Shift to divide by 64.

      // Extract pixels 0 and 2
      // r = bound(0, (int)((y0 + 1.5958f  * v)                * 255.0f), 255);
      // g = bound(0, (int)((y0 - 0.39173f * u - 0.81290f * v) * 255.0f), 255);
      // b = bound(0, (int)((y0 + 2.017f   * u)                * 255.0f), 255);
      __m128i uv = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(2, 2, 0, 0)), _MM_SHUFFLE(2, 2, 0, 0));
      uv = _mm_srai_epi16(_mm_mullo_epi16(uv, val3), 5); // Shift to divide by 32.
      uv = _mm_add_epi16(uv, _mm_slli_epi64(_mm_srli_epi64(uv, 48), 16)); // add the "-0.81290f * v" component to the "-0.39173f * u" component

      __m128i c02 = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(1, 1, 1, 1)), _MM_SHUFFLE(1, 1, 1, 1));
      c02 = _mm_max_epi16(_mm_min_epi16(_mm_add_epi16(c02, uv), maxv), minv); // bound 0 >= n >= 255, except for "a" component.

      // Extract pixels 1 and 3
      // r = bound(0, (int)((y1 + 1.5958f  * v)                * 255.0f), 255);
      // g = bound(0, (int)((y1 - 0.39173f * u - 0.81290f * v) * 255.0f), 255);
      // b = bound(0, (int)((y1 + 2.017f   * u)                * 255.0f), 255);
      __m128i c13 = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(3, 3, 3, 3)), _MM_SHUFFLE(3, 3, 3, 3));
      c13 = _mm_max_epi16(_mm_min_epi16(_mm_add_epi16(c13, uv), maxv), minv); // bound 0 >= n >= 255, except for "a" component.

      // Store data
      if (aligned)
        _mm_store_si128(rgb, _mm_shuffle_epi32(_mm_packus_epi16(c02, c13), _MM_SHUFFLE(3, 1, 2, 0)));
      else
        _mm_storeu_si128(rgb, _mm_shuffle_epi32(_mm_packus_epi16(c02, c13), _MM_SHUFFLE(3, 1, 2, 0)));

      rgb += 4;
    }
  }
#endif
}

void LXiStreamGui_SImage_convertBGRtoRGB
 (uint32_t * restrict dst, const uint32_t * restrict src, unsigned numPixels)
{
  unsigned i;

#ifdef __SSE__
  // Check alignment
  if ((((size_t)dst & (size_t)15) != 0) || (((size_t)src & (size_t)15) != 0) ||
      ((numPixels % (sizeof(__m128i) / sizeof(uint32_t))) != 0))
  {
#endif
    for (i=0; i<numPixels; i++)
    {
      *dst++ =  (*src & 0xFF00FF00) |
               ((*src & 0x000000FF) << 16) |
               ((*src & 0x00FF0000) >> 16);
      src++;
    }
#ifdef __SSE__
  }
  else
  {
    __m128i mask1 = _mm_set_epi32(0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00);
    __m128i mask2 = _mm_set_epi32(0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF);
    __m128i mask3 = _mm_set_epi32(0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000);

    for (i=0; i<numPixels; i+=sizeof(__m128i)/sizeof(uint32_t))
    {
      __m128i px = _mm_load_si128((__m128i *)src);
      src += sizeof(__m128i) / sizeof(*src);

      __m128i rs = _mm_and_si128(px, mask1);
      rs = _mm_or_si128(rs, _mm_slli_si128(_mm_and_si128(px, mask2), 16));
      rs = _mm_or_si128(rs, _mm_srli_si128(_mm_and_si128(px, mask3), 16));

      _mm_store_si128((__m128i *)dst, rs);
      dst += sizeof(__m128i) / sizeof(*dst);
    }
  }
#endif
}

void LXiStreamGui_SImage_convertYUV1toRGB
 (struct RGBAPixel * restrict rgb, const uint8_t * restrict iy, const uint8_t * restrict iu, const uint8_t * restrict iv, unsigned numPixels)
{
  unsigned i;

  for (i=0; i<numPixels; i+=2)
  {
    int y = (int)(iy[i]);
    int u = (int)(iu[i]);
    int v = (int)(iv[i]);

    // Preprocess
    y = (75 * (y - 16)) >> 6;
    u = u - 128;
    v = v - 128;

    // Extract Pixel
    rgb->r = bound(0, y + ((51 * v) >> 5), 255);
    rgb->g = bound(0, y - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
    rgb->b = bound(0, y + ((65 * u) >> 5), 255);
    rgb->a = 0xFF;
    rgb++;
  }
}

void LXiStreamGui_SImage_convertYUV2toRGB
 (struct RGBAPixel * restrict rgb, const uint8_t * restrict iy, const uint8_t * restrict iu, const uint8_t * restrict iv, unsigned numPixels)
{
  unsigned i;

#ifdef __SSE__
  // Check alignment
  if ((numPixels % (sizeof(__m128i) / sizeof(uint32_t))) != 0)
  {
#endif
    for (i=0; i<numPixels; i+=2)
    {
      int y0 = (int)(iy[i]);
      int u =  (int)(iu[i >> 1]);
      int y1 = (int)(iy[i + 1]);
      int v =  (int)(iv[i >> 1]);

      // Preprocess
      y0 = (75 * (y0 - 16)) >> 6;
      u = u - 128;
      y1 = (75 * (y1 - 16)) >> 6;
      v = v - 128;

      // Extract Pixel 1
      rgb->r = bound(0, y0 + ((51 * v) >> 5), 255);
      rgb->g = bound(0, y0 - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
      rgb->b = bound(0, y0 + ((65 * u) >> 5), 255);
      rgb->a = 0xFF;
      rgb++;

      // Extract Pixel 2
      rgb->r = bound(0, y1 + ((51 * v) >> 5), 255);
      rgb->g = bound(0, y1 - ((13 * u) >> 5) - ((26 * v) >> 5), 255);
      rgb->b = bound(0, y1 + ((65 * u) >> 5), 255);
      rgb->a = 0xFF;
      rgb++;
    }
#ifdef __SSE__
  }
  else
  {
    const int aligned = (((size_t)rgb & (size_t)15) == 0);
    const __m128i minv = _mm_setr_epi16(0, 0, 0, 255, 0, 0, 0, 255);            // "a" component should always be 255
    const __m128i maxv = _mm_setr_epi16(255, 255, 255, 255, 255, 255, 255, 255);
    const __m128i val1 = _mm_setr_epi16(16, 128, 16, 128, 16, 128, 16, 128);    // = 0.0625f, 0.5f, 0.0625f, 0.5f Multiplied by 256
    const __m128i val2 = _mm_setr_epi16(75, 64, 75, 64, 75, 64, 75, 64);        // = 1.1643f, 1.0f, 1.1643f, 1.0f Multiplied by 64
    const __m128i val3 = _mm_setr_epi16(65, -13, 51, -26, 65, -13, 51, -26);    // = 2.017f, -0.39173f, 1.5958f, -0.81290f Multiplied by 32

    for (i=0; i<numPixels; i+=4)
    {
      const int in = i + 2;
      __m128i px = _mm_setr_epi16(iy[i],  iu[i  >> 1], iy[i  + 1], iv[i  >> 1],
                                  iy[in], iu[in >> 1], iy[in + 1], iv[in >> 1]);

      // y0 = 1.1643f * (y0 - 0.0625f);
      // u = u - 0.5f;
      // y1 = 1.1643f * (y1 - 0.0625f);
      // v = v - 0.5f;
      px = _mm_srai_epi16(_mm_mullo_epi16(_mm_sub_epi16(px, val1), val2), 6); // Shift to divide by 64.

      // Extract pixels 0 and 2
      // r = bound(0, (int)((y0 + 1.5958f  * v)                * 255.0f), 255);
      // g = bound(0, (int)((y0 - 0.39173f * u - 0.81290f * v) * 255.0f), 255);
      // b = bound(0, (int)((y0 + 2.017f   * u)                * 255.0f), 255);
      __m128i uv = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(3, 3, 1, 1)), _MM_SHUFFLE(3, 3, 1, 1));
      uv = _mm_srai_epi16(_mm_mullo_epi16(uv, val3), 5); // Shift to divide by 32.
      uv = _mm_add_epi16(uv, _mm_slli_epi64(_mm_srli_epi64(uv, 48), 16)); // add the "-0.81290f * v" component to the "-0.39173f * u" component

      __m128i c02 = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(0, 0, 0, 0)), _MM_SHUFFLE(0, 0, 0, 0));
      c02 = _mm_max_epi16(_mm_min_epi16(_mm_add_epi16(c02, uv), maxv), minv); // bound 0 >= n >= 255, except for "a" component.

      // Extract pixels 1 and 3
      // r = bound(0, (int)((y1 + 1.5958f  * v)                * 255.0f), 255);
      // g = bound(0, (int)((y1 - 0.39173f * u - 0.81290f * v) * 255.0f), 255);
      // b = bound(0, (int)((y1 + 2.017f   * u)                * 255.0f), 255);
      __m128i c13 = _mm_shufflelo_epi16(_mm_shufflehi_epi16(px, _MM_SHUFFLE(2, 2, 2, 2)), _MM_SHUFFLE(2, 2, 2, 2));
      c13 = _mm_max_epi16(_mm_min_epi16(_mm_add_epi16(c13, uv), maxv), minv); // bound 0 >= n >= 255, except for "a" component.

      // Store data
      if (aligned)
        _mm_store_si128(rgb, _mm_shuffle_epi32(_mm_packus_epi16(c02, c13), _MM_SHUFFLE(3, 1, 2, 0)));
      else
        _mm_storeu_si128(rgb, _mm_shuffle_epi32(_mm_packus_epi16(c02, c13), _MM_SHUFFLE(3, 1, 2, 0)));

      rgb += 4;
    }
  }
#endif
}

