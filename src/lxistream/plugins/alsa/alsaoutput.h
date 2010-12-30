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

#ifndef __ALSAOUTPUT_H
#define __ALSAOUTPUT_H

#include <alsa/asoundlib.h>
#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace AlsaBackend {


class AlsaOutput : public SInterfaces::AudioOutput
{
Q_OBJECT
public:
                                AlsaOutput(const QString &, QObject *);
  virtual                       ~AlsaOutput();

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual STime                 latency(void) const;

  virtual void                  consume(const SAudioBuffer &);

  static snd_pcm_format_t       toALSA(SAudioFormat::Format);

private:
  void                          openFormat(const SAudioFormat &);

private:
  static const int              maxDelay = 100;

  const QString                 dev;
  snd_pcm_t                   * pcm;
  SInterfaces::AudioResampler * resampler;

  STime                         outLatency;
  SAudioFormat                  lastFormat;
  SAudioFormat                  outFormat;
};


} } // End of namespaces

#endif
