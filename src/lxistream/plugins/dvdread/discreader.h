/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef __DISCREADER_H
#define __DISCREADER_H

#include <QtCore>
#include <LXiStream>
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>

namespace LXiStream {
namespace DVDReadBackend {


class DiscReader : public SInterfaces::DiscReader
{
Q_OBJECT
private:
  struct Callback : SInterfaces::BufferReader::ReadCallback
  {
    inline explicit Callback(::dvd_file_t *file, const ::dvd_stat_t &stat)
      : file(file), stat(stat), pos(0)
    {
    }

    virtual qint64              read(uchar *, qint64);
    virtual qint64              seek(qint64, int);

    ::dvd_file_t        * const file;
    const ::dvd_stat_t          stat;
    qint64                      pos;
  };

public:
  static const char     * const formatName;

  explicit                      DiscReader(const QString &, QObject *);
  virtual                       ~DiscReader();

public: // From SInterfaces::DiscReader
  virtual bool                  openPath(const QString &format, const QString &path);

  virtual unsigned              numTitles(void) const;
  virtual SInterfaces::BufferReader::ReadCallback * openTitle(unsigned);
  virtual void                  closeTitle(SInterfaces::BufferReader::ReadCallback *);

private:
  ::dvd_reader_t              * dvdHandle;
  ::ifo_handle_t              * vmgFile;
};


} } // End of namespaces

#endif
