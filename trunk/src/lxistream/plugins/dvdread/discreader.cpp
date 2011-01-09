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

#include "discreader.h"

namespace LXiStream {
namespace DVDReadBackend {

const char * const DiscReader::formatName = "dvd";

DiscReader::DiscReader(const QString &, QObject *parent)
  : SInterfaces::DiscReader(parent),
    dvdHandle(NULL),
    vmgFile(NULL)
{
}

DiscReader::~DiscReader()
{
  if (vmgFile)
    ::ifoClose(vmgFile);

  if (dvdHandle)
    ::DVDClose(dvdHandle);
}

bool DiscReader::openPath(const QString &, const QString &path)
{
  dvdHandle = ::DVDOpen(path.toUtf8());
  if (dvdHandle)
    return (vmgFile = ::ifoOpen(dvdHandle, 0)) != NULL;

  return false;
}

unsigned DiscReader::numTitles(void) const
{
  if (vmgFile)
    return vmgFile->tt_srpt->nr_of_srpts;

  return 0;
}

SInterfaces::BufferReader::ReadCallback * DiscReader::openTitle(unsigned title)
{
  if (dvdHandle)
  {
    ::dvd_file_t * const file = ::DVDOpenFile(dvdHandle, title, ::DVD_READ_TITLE_VOBS);
    if (file)
    {
      dvd_stat_t stat;
      ::memset(&stat, 0, sizeof(stat));
      ::DVDFileStat(dvdHandle, title + 1, ::DVD_READ_TITLE_VOBS, &stat);

      return new Callback(file, stat);
    }
  }

  return NULL;
}

void DiscReader::closeTitle(SInterfaces::BufferReader::ReadCallback *callback)
{
  if (callback)
  {
    ::DVDCloseFile(static_cast<Callback *>(callback)->file);

    delete callback;
  }
}

qint64 DiscReader::Callback::read(uchar *buffer, qint64 size)
{
  const unsigned blockStart = pos / DVD_VIDEO_LB_LEN;
  const unsigned blockOffset = pos % DVD_VIDEO_LB_LEN;
  if ((blockOffset == 0) && (size >= DVD_VIDEO_LB_LEN))
  {
    const unsigned count = qMax(size_t(1), size_t(size / DVD_VIDEO_LB_LEN));
    const ssize_t result = ::DVDReadBlocks(file, blockStart, count, buffer);
    if (result > 0)
    {
      const qint64 bytes = qint64(result) * DVD_VIDEO_LB_LEN;
      pos += bytes;
      return bytes;
    }
  }
  else // Unaligned read
  {
    static const unsigned count = 2;
    uchar ubuf[count * DVD_VIDEO_LB_LEN];
    const ssize_t result = ::DVDReadBlocks(file, blockStart, count, ubuf);
    if (result > 0)
    {
      const qint64 bytes = qMin(size, qint64(result * DVD_VIDEO_LB_LEN) - blockOffset);
      ::memcpy(buffer, ubuf + blockOffset, bytes);
      pos += bytes;
      return bytes;
    }
  }

  return -1;
}

qint64 DiscReader::Callback::seek(qint64 offset, int whence)
{
  if ((whence == SEEK_SET) && (offset <= stat.size))
  {
    pos = offset;
    return 0;
  }
  else if ((whence == SEEK_CUR) && (pos + offset <= stat.size))
  {
    pos += offset;
    return 0;
  }
  else if ((whence == SEEK_END) && (stat.size + offset <= stat.size))
  {
    pos = stat.size + offset;
    return 0;
  }
  else if (whence == -1) // get size
    return stat.size;

  return -1;
}

} } // End of namespaces
