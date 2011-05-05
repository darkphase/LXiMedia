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

void LXiStream_SAudioMatrixNode_mixMatrix
 (const int16_t * __restrict srcData, unsigned numSamples, unsigned srcNumChannels,
  int16_t * __restrict dstData, const float * __restrict appliedMatrix, unsigned dstNumChannels)
{
  unsigned i, dc, mp, sc;

  for (i=0; i<numSamples; i++)
  {
    for (dc=0; dc<dstNumChannels; dc++)
    {
      mp = dc * srcNumChannels;
      dstData[dc] = (int16_t)((float)(srcData[0]) * appliedMatrix[mp]);

      for (sc=1; sc<srcNumChannels; sc++)
        dstData[dc] += (int16_t)((float)(srcData[sc]) * appliedMatrix[mp + sc]);
    }

    srcData += srcNumChannels;
    dstData += dstNumChannels;
  }
}
