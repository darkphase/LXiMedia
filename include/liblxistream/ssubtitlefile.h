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

#ifndef LXSTREAM_SSUBTITLEFILE_H
#define LXSTREAM_SSUBTITLEFILE_H

#include "sdatacodec.h"
#include "sencodeddatabuffer.h"

namespace LXiStream {

/*! Represents a separate subtitle file (e.g. srt file). This class is used
    internally by SMediaInfo and SFileInputNode to transparently support
    separate subtitle files (they are added as separate data streams).
 */
class SSubtitleFile
{
public:
                                SSubtitleFile(const QString &name);
                                ~SSubtitleFile();

  bool                          exists(void) const;
  QString                       fileName(void) const;

  bool                          open(void);
  void                          close(void);
  void                          reset(void);

  const char                  * language(void) const;
  SDataCodec                    codec(void) const;

  SEncodedDataBuffer            readSubtitle(STime);

public:
  static QStringList            findSubtitleFiles(const QString &);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
