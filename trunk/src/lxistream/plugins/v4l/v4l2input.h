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

#ifndef V4LBACKEND_V4L2INPUT_H
#define V4LBACKEND_V4L2INPUT_H

#include <QtCore>
#include <LXiStream>
#include "videodev2.h"

namespace LXiStream {
namespace V4lBackend {

class V4l2Input : public SInterfaces::VideoInput
{
Q_OBJECT
public:
  class Memory : public SBuffer::Memory
  {
  public:
                                Memory(char *data, int size, int, V4l2Input *);
    virtual                     ~Memory();

  private:
    int                         bufferIndex;
    V4l2Input           * const parent;
  };

public:
  static QList<SFactory::Scheme> listDevices(void);

public:
                                V4l2Input(const QString &, QObject *);
  virtual                       ~V4l2Input();

  inline qreal                  contrast(void) const                            { return control(V4L2_CID_CONTRAST); }
  inline void                   setContrast(qreal v)                            { setControl(V4L2_CID_CONTRAST, v); }
  inline qreal                  brightness(void) const                          { return control(V4L2_CID_BRIGHTNESS); }
  inline void                   setBrightness(qreal v)                          { setControl(V4L2_CID_BRIGHTNESS, v); }
  inline qreal                  saturation(void) const                          { return control(V4L2_CID_SATURATION); }
  inline void                   setSaturation(qreal v)                          { setControl(V4L2_CID_SATURATION, v); }
  inline qreal                  hue(void) const                                 { return control(V4L2_CID_HUE); }
  inline void                   setHue(qreal v)                                 { setControl(V4L2_CID_HUE, v); }

public: // From SInterfaces::VideoInput
  virtual void                  setFormat(const SVideoFormat &);
  virtual SVideoFormat          format(void);
  virtual void                  setMaxBuffers(int);
  virtual int                   maxBuffers(void) const;

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

private:
  qreal                         control(quint32 id) const;
  bool                          setControl(quint32 id, qreal value);

  V4l2Input::Memory           * nextImage(void);
  void                          queueBuffer(int);
  void                          updateAgc(const SVideoBuffer &);

  static quint32                toV4L(SVideoFormat::Format);
  static SVideoFormat::Format   fromV4L(quint32);
  static quint32                toV4L(const SVideoCodec &codec);
  static v4l2_field             toV4L(SVideoFormat::FieldMode);

private:
  static QMap<QString, int>     deviceMap;

  int                           devDesc;
  v4l2_capability               capabilities;

  QList<SVideoFormat::Format>   pixelFormats;
  //QMap<QString, v4l2_input>     videoInputs;
  //QMap<SAnalogTuner::VideoStandard, v4l2_standard> videoStandards;
  //QMap<QString, v4l2_queryctrl> videoControls;

  mutable QMutex                mutex;
  STimer                        timer;
  SVideoFormat                  outFormat;
  static const int              defaultOutBuffers = 32;
  int                           numOutBuffers;

  bool                          agc;
  unsigned                      agcCounter;
  float                         agcMin, agcMax, agcMaxc;
  float                         agcWeight;

  v4l2_requestbuffers           bufferRequest;
  int                           mappedBuffers;
  v4l2_buffer                 * buffers;
  char                       ** maps;
  size_t                        bufferSize;

  bool                          streamOn;
};


} } // End of namespaces

#endif
