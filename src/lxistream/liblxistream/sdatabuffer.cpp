/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "sdatabuffer.h"

namespace LXiStream {

void SDataBuffer::assign(const SDataBuffer &from)
{
  d.type = from.d.type;
  switch(d.type)
  {
  case Type_None:
    d.buffer = NULL;
    return;

  case Type_SubtitleBuffer:
    d.buffer = new SSubtitleBuffer(from.subtitleBuffer());
    return;

  case Type_SubpictureBuffer:
    d.buffer = new SSubpictureBuffer(from.subpictureBuffer());
    return;
  }

  d.type = Type_None;
  d.buffer = NULL;
}

void SDataBuffer::destroy(void)
{
  switch(d.type)
  {
  case Type_None:
    break;

  case Type_SubtitleBuffer:
    delete static_cast<SSubtitleBuffer *>(d.buffer);
    break;

  case Type_SubpictureBuffer:
    delete static_cast<SSubpictureBuffer *>(d.buffer);
    break;
  }

  d.buffer = NULL;
  d.type = Type_None;
}


/*! Takes the received lines of the page in c and overwrites them in this page.
    This way, if lines are not received because of errors, the old lines are
    taken.
 */
/*SDataBuffer::TeletextPage & SDataBuffer::TeletextPage::operator|=(const TeletextPage &c)
{
  pgno = c.pgno;
  subno = c.subno;
  lang = c.lang;
  flags = c.flags;
  errors = c.errors;

  flof = c.flof;
  if (c.flof)
    memcpy(link, c.link, sizeof(link));

  lines |= c.lines;
  for (int i=0; i<25; i++)
  if ((c.lines & (1 << i)) != 0)
    memcpy(data[i], c.data[i], sizeof(data[i]));

  return *this;
}

QStringList SDataBuffer::TeletextPage::decodedLines(void) const
{
  QStringList result;

  for (unsigned y=1; y<24; y++) // Skip top and bottom lines
  {
    QString text;

    int graphics = -1;
    const char * const line = this->line(y);

    if (line != NULL)
    for (unsigned x=0; x<40; x++)
    {
      const char c = ((y > 0) || (x >= 8)) ? line[x] : ' ';
      const char l = c & 0x0F;
      const char h = c & 0x70;

      if (h == 0)
      {
        if (l < 8)
          graphics = -qAbs(graphics);
      }
      else if (h == 0x10)
      {
        if (l != 8)
          graphics = qAbs(graphics);

        if (l == 9)
          graphics = 1;
        else if (l == 10)
          graphics = 2;
        else if (l == 15)
          graphics = -qAbs(graphics);
      }

      if ((h >= 0x20) && (graphics < 0))
        text += QChar::fromLatin1(c);
      else
        text += ' ';
    }

    result += text.simplified();
  }

  return result;
}

QString SDataBuffer::TeletextPage::channelName(void) const
{
  const char *line0 = line(0);

  if (line0 != NULL)
  {
    char header[40];
    int channelNameStart = 0;
    int channelNameLen = 0;

    for (unsigned i=8; i<40; i++)
    {
      header[i] = line0[i] & 0x7F; // Remove parity bit
      if ((channelNameStart == 0) && (header[i] >= 'A'))
        channelNameStart = i;
      else if ((channelNameStart > 0) && (channelNameLen == 0) && (header[i] <= 32))
      {
        channelNameLen = i - channelNameStart;
        break;
      }
    }

    if (channelNameLen > 0)
      return QString::fromAscii(header + channelNameStart, channelNameLen);
  }

  return QString::null;
}

QTime SDataBuffer::TeletextPage::localTime(void) const
{
  const char *line0 = line(0);

  if (line0 != NULL)
  {
    char time[8];

    for (unsigned i=0; i<8; i++)
    {
      time[i] = line0[i + 32] & 0x7F; // Remove parity bit

      if (((time[i] < '0') || (time[i] > '9')) &&
          ((time[i] > ' ') && (time[i] < 'A')))
      {
        time[i] = ':';
      }
    }

    return QTime::fromString(QString::fromAscii(time, 8));
  }

  return QTime(-1, -1, -1);
}*/


} // End of namespace
