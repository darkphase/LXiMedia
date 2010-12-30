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

#ifndef V4LBACKEND_V4L1DEVICE_H
#define V4LBACKEND_V4L1DEVICE_H

#include <QtCore>
#include <LXiStream>

#include "v4lcommon.h"

typedef qint64    __s64;
typedef quint64   __u64;
typedef qint32    __s32;
typedef quint32   __u32;
typedef qint16    __s16;
typedef quint16   __u16;
typedef qint8     __s8;
typedef quint8    __u8;
#include "videodev.h"

namespace LXiStream {
namespace V4lBackend {

class V4l1Input;
class VBIInput;


class V4l1Device : public STerminals::AudioVideoDevice,
                   private V4lCommon
{
Q_OBJECT
public:
  static SSystem::DeviceEntryList listDevices(void);

public:
                                V4l1Device(QObject *parent);
  virtual                       ~V4l1Device();

  virtual inline bool           audioEnabled(void) const                        { return openAudioDevice; }
  virtual inline void           setAudioEnabled(bool e)                         { openAudioDevice = e; }
  virtual inline bool           videoEnabled(void) const                        { return openVideoDevice; }
  virtual inline void           setVideoEnabled(bool e)                         { openVideoDevice = e; }

protected:
  inline virtual QObject      * object(void)                                    { return this; }

public: // From STerminal
  virtual bool                  open(const QUrl &);

  virtual QString               friendlyName(void) const;
  virtual QString               longName(void) const;
  virtual Types                 terminalType(void) const;

  virtual QStringList           inputs(void) const;
  virtual bool                  selectInput(const QString &);
  virtual STuner              * tuner(void) const;

  virtual QList<Stream>         inputStreams(void) const;
  virtual QList<Stream>         outputStreams(void) const;
  virtual SNode               * openStream(const Stream &);

public:
  static quint16                toV4L(SVideoCodec::Format);

private:
  static QMap<QString, int>     videoDevices;

  V4l1Input                   * videoDev;
  VBIInput                    * vbiDev;
};


} } // End of namespaces

#endif