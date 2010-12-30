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

#ifndef LXSTREAM_SDATABUFFER_H
#define LXSTREAM_SDATABUFFER_H

#include <QtCore>
#include "sbuffer.h"
#include "stime.h"

namespace LXiStream {

/*! This class represents a buffer containing subtitle data.
 */
class SSubtitleBuffer : public SBuffer
{
public:
  inline                        SSubtitleBuffer(void) : SBuffer()               { }
  inline                        SSubtitleBuffer(const QStringList &s) : SBuffer() { setSubtitle(s); }

  inline STime                  timeStamp(void) const                           { return d.timeStamp; }
  inline void                   setTimeStamp(STime t)                           { d.timeStamp = t; }

  inline QStringList            subtitle(void) const                            { return QString::fromUtf8(data(), size()).split('\n'); }
  inline void                   setSubtitle(const QStringList &s)               { setData(s.join("\n").toUtf8()); }

  inline STime                  duration(void) const                            { return d.duration; }
  inline void                   setDuration(STime t)                            { d.duration = t; }

private:
  struct
  {
    STime                       timeStamp;
    STime                       duration;
  }                             d;
};

typedef QList<SSubtitleBuffer>  SSubtitleBufferList;

/*
  struct TeletextPage
  {
    inline                      TeletextPage(void)                              { memset(this, 0, sizeof(*this)); }
    TeletextPage              & operator|=(const TeletextPage &);

    // Helper functions
    inline bool                 isNull(void) const                              { return pgno == 0; }
    inline unsigned             page(void) const                                { return pgno; }
    inline unsigned             subPage(void) const                             { return subno; }
    inline const char         * line(unsigned line) const                       { return ((lines & (1 << line)) != 0) ? data[line] : NULL; }
    QStringList                 decodedLines(void) const;

    QString                     channelName(void) const;
    QTime                       localTime(void) const;

    // Data fields
    int                         pgno, subno;	// the wanted page number
    int                         lang;		// language code
    int                         flags;		// misc flags (see PG_xxx below)
    int                         errors;		// number of single bit errors in page
    quint32                     lines;		// 1 bit for each line received
    char                        data[25][40];	// page contents
    int                         flof;		// page has FastText links
    struct{int pgno, subno;}    link[6];	// FastText links (FLOF)
  } __attribute__((packed));

  struct TeletextXPacket
  {
    char                        header[20];
    char                        programmeName[20];
  } __attribute__((packed));
*/

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SSubtitleBuffer)

#endif
