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

#include "subtitles.h"
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

void Subtitles::drawLine(
    uint8_t *dsty, int yStride, uint8_t *dstu, int uStride, uint8_t *dstv, int vStride,
    int wf, int hf, int width, int height, float stretch,
    const char *line, int pos, const Char * const *characters)
{
  int lineWidth = 0, lineHeight = 0;
  for (int i=0; line[i]; i++)
  {
    lineWidth += characters[uint8_t(line[i])]->advance;
    lineHeight = max(lineHeight, characters[uint8_t(line[i])]->height);
  }

  lineWidth = int(float(lineWidth) / stretch);

  const int ypos = height - (lineHeight * pos) - lineHeight - (lineHeight / 2);

  if ((lineWidth + (lineHeight * 2)) < width)
  for (int y=0; y<lineHeight; y++)
  {
    uint8_t * const yline = dsty + ((ypos + y) * yStride) + ((width / 2) - (lineWidth / 2));
    uint8_t * const uline = dstu + (((ypos + y) / hf) * uStride) + (((width / wf) / 2) - ((lineWidth / wf) / 2));
    uint8_t * const vline = dstv + (((ypos + y) / hf) * vStride) + (((width / wf) / 2) - ((lineWidth / wf) / 2));

    if (abs(1.0f - stretch) < 0.0001f)
    {
      // First draw the black shadow.
      for (int i=0, p=0; line[i]; i++)
      {
        const Char * const c = characters[uint8_t(line[i])];
        const uint8_t * const pixels = c->pixels + (y * c->width);

        for (int x=0; x<c->width; x++)
        if (pixels[x] < 128)
          yline[p+x] = ((pixels[x] * 2) >= yline[p+x]) ? 0 : (yline[p+x] - (pixels[x] * 2));

        p += c->advance;
      }

      // Mext the text (to prevent the shadow overlapping the previous text).
      for (int i=0, p=0; line[i]; i++)
      {
        const Char * const c = characters[uint8_t(line[i])];
        const uint8_t * const pixels = c->pixels + (y * c->width);

        for (int x=0; x<c->width; x++)
        if (pixels[x] >= 128)
          yline[p+x] = (pixels[x] - 128) * 2;

        if ((y % hf) == 0)
        for (int x=0; x<c->width; x+=wf)
        if (pixels[x] >= 128)
          uline[(p+x)/wf] = vline[(p+x)/wf] = uint8_t(127);

        p += c->advance;
      }
    }
    else
    {
      // First draw the black shadow.
      float pf = 0.0f, pw = 0.0f; int p = 0;
      for (int i=0; line[i]; i++)
      {
        const Char * const c = characters[uint8_t(line[i])];
        const uint8_t * const pixels = c->pixels + (y * c->width);

        for (int x=0, n=c->width/stretch; x<n; x++)
        {
          const float xp = (float(x) * stretch) - pw;
          const int xx[2] = { bound(0, int(xp), c->width - 1), min(int(xp) + 1, c->width - 1) };

          uint8_t yv[2];
          for (int j=0; j<2; j++)
          if (pixels[xx[j]] < 128)
            yv[j] = ((pixels[xx[j]] * 2) >= yline[p+x]) ? 0 : (yline[p+x] - (pixels[xx[j]] * 2));
          else
            yv[j] = yline[p+x];

          const float w = xp - float(int(xp));
          yline[p+x] = uint8_t((float(yv[0]) * (1.0f - w)) + (float(yv[1]) * w));
        }

        p = int(pf += float(c->advance) / stretch);
        pw = pf - float(p);
      }

      // Mext the text (to prevent the shadow overlapping the previous text).
      pf = 0.0f; pw = 0.0f; p = 0;
      for (int i=0, p=0; line[i]; i++)
      {
        const Char * const c = characters[uint8_t(line[i])];
        const uint8_t * const pixels = c->pixels + (y * c->width);

        for (int x=0, n=c->width/stretch; x<n; x++)
        {
          const float xp = (float(x) * stretch) - pw;
          const int xx[2] = { bound(0, int(xp), c->width - 1), min(int(xp) + 1, c->width - 1) };

          uint8_t yv[2];
          for (int j=0; j<2; j++)
          if (pixels[xx[j]] >= 128)
            yv[j] = (pixels[xx[j]] - 128) * 2;
          else
            yv[j] = yline[p+x];

          const float w = xp - float(int(xp));
          yline[p+x] = uint8_t((float(yv[0]) * (1.0f - w)) + (float(yv[1]) * w));

          if ((pixels[xx[0]] >= 128) || (pixels[xx[1]] >= 128))
            uline[(p+x)/wf] = vline[(p+x)/wf] = uint8_t(127);
        }

        p = int(pf += float(c->advance) / stretch);
        pw = pf - float(p);
      }
    }
  }
}

} } // End of namespaces
