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

#ifndef LXSTREAMCOMMON_AUDIORESAMPLER_H
#define LXSTREAMCOMMON_AUDIORESAMPLER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


/*! This audio filter can be used to resample an audio stream.
 */
class AudioResampler : public SInterfaces::AudioResampler
{
Q_OBJECT
public:
  explicit                      AudioResampler(const QString &, QObject *parent);

public: // From SInterfaces::AudioResampler
  virtual void                  setSampleRate(unsigned);
  virtual unsigned              sampleRate(void);

  virtual SAudioBuffer          processBuffer(const SAudioBuffer &);

private:
  unsigned                      outSampleRate;
  SAudioBuffer                  lastBuffer;
  unsigned                      nextPos;
  float                         weightOffset;
};


} } // End of namespaces

#endif
