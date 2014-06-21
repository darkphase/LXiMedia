/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef V4LBACKEND_V4LINPUT_H
#define V4LBACKEND_V4LINPUT_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtCore/QMap>
#include <QtCore/QQueue>
#include <QtCore/QSemaphore>
#include <QtCore/QStack>
#include <QtCore/QUrl>

#include <lxstream/frontend/smediatimer.h>
#include <lxstream/backend/smultiplexer.h>
#include <lxstream/backend/svideoproducer.h>

#include "vbiinput.h"

typedef signed long long    __s64;
typedef unsigned long long  __u64;
typedef signed int          __s32;
typedef unsigned int        __u32;
typedef signed short        __s16;
typedef unsigned short      __u16;
typedef signed char         __s8;
typedef unsigned char       __u8;
#include "videodev.h"
#include "videodev2.h"

namespace LXiStream {
namespace V4lBackend {

using namespace LXBase;


class V4lInput : public QObject,
                 public Backend::SMultiplexer
{
public:
  static const char     * const scheme;
  static QStringList            listDevices(void);

private:
  struct CaptureBuffer
  {
    v4l2_buffer                 vidbuf;
    const char                * data;
  };

  class Tuner : public MultiMediaInput::AnalogTuner
  {
  public:
                                Tuner(V4lInput *);

    void                        addStandard(VideoMode, const v4l2_standard &);

    virtual Frequency           getFrequency(void) const;
    virtual bool                setFrequency(const Frequency &);
    virtual Status              getSignalStatus(void) const;

    virtual bool                setVolume(double);
    virtual double              getVolume(void) const;
    virtual bool                setContrast(double);
    virtual double              getContrast(void) const;
    virtual bool                setBrightness(double);
    virtual double              getBrightness(void) const;
    virtual bool                setSaturation(double);
    virtual double              getSaturation(void) const;
    virtual bool                setHue(double);
    virtual double              getHue(void) const;

  public:
    bool                        hasTuner;
    int                         tunerID;
    bool                        tunerLow;
    QMap<VideoMode, v4l2_standard> videoStandardsMode;
    QMap<int, VideoMode>        videoStandardsID;

  private:
    V4lInput * const parent;

    quint64                     transponderFreq;
    AudioMode                   audioMode;
    VideoMode                   videoMode;
  };

  class Producer : protected QThread,
                   public virtual Object,
                   public Stream,
                   public VideoProducer
  {
  public:
    class BufferPtr : public VideoBufferPtr
    {
    public:
      inline explicit           BufferPtr(Object *owner, size_t = 0) : VideoBufferPtr(owner), index(-1) { }

    public:
      int                       index;
    };

  public:
    explicit                    Producer(V4lInput *);
    virtual                     ~Producer();

    virtual Object::Ptr<AudioProducer> getAudioProducer(void) const;
    virtual Object::Ptr<VideoProducer> getVideoProducer(void) const;
    virtual Object::Ptr<DataProducer>  getDataProducer(void) const;

    virtual void                setProgram(const Stream::Program &);

    virtual bool                startStream(void);
    virtual bool                stopStream(void);
    virtual void                generateBuffer(BufferType, bool async);

    virtual void                objectReleased(Object *);

    inline void                 setDataProducer(VBIInput *v)                    { vbiInput = v; }
    double                      getDataQuality(void) const;

  protected:
    virtual void                run(void);

  private:
    bool                        nextImage(const char *&, size_t &, int &);
    bool                        readImage(void *, size_t &);
    bool                        queueBuffer(int);
    bool                        queueAllBuffers();

  public:
    int                         numBuffers;
    CaptureBuffer             * captureBuffers;
    unsigned long               frameCounter;
    MediaTimer                  mediaTimer;

    QQueue<BufferPtr *>         availableBuffers;
    BufferConsumer<VideoBuffer> producedBuffers;

  private:
    V4lInput * const parent;
    volatile bool               running;

    quint64                     lastTime;
    quint64                     sourceTime;
    qint64                      avgFrameDeltaT;

    Object::Ptr<VBIInput>       vbiInput;
  };

public:
                                V4lInput(const QUrl &);
  virtual                       ~V4lInput();

  virtual QString               getDeviceName(void) const;
  virtual QStringList           getInputs(void) const;

  virtual inline const char   * getCardType(void) const                         { return (const char *)capabilities.card; }
  virtual inline const char   * getDriverName(void) const                       { return (const char *)capabilities.driver; }
  virtual inline const char   * getBusID(void) const                            { return (const char *)capabilities.bus_info; }

  virtual MultiMediaInput::Tuner * getTuner(void) const;
  virtual Object::Ptr<Stream>   openStream(const MultiMediaInput::Program &);

  virtual bool                  selectInput(const QString &);
  virtual bool                  requestImageSize(unsigned &, unsigned &, VideoBuffer::FieldMode &);
  virtual bool                  requestHardwareEncoding(bool &);
/*
  virtual inline unsigned       getWidth(void) const                            { return width; }
  virtual inline unsigned       getHeight(void) const                           { return height; }
*/
  inline double                 getWidthAspect(void) const                      { return double(height) / double(width) * 1.333; }
  inline double                 getHeightAspect(void) const                     { return 1.0; }

protected:
  virtual bool                  setControl(quint32, double);
  virtual double                getControl(quint32) const;

  static quint32                ToV4L2(VideoBuffer::CodecID);
  static quint32                ToV4L1(VideoBuffer::CodecID);
  static int                    PixelSize(VideoBuffer::CodecID);
  static v4l2_field             ToV4L2(VideoBuffer::FieldMode);
  static int                    ToV4L2(Tuner::AudioMode);
  static Tuner::AudioMode       FromV4L2(int);

private:
  bool                          selectInput(int);

private:
  static QMap<QString, int>     foundDevices;

  volatile bool                 running;

  Tuner                         tuner;
  int                           devDesc;
  v4l2_capability               capabilities;
  bool                          isV4L1;
  video_capability              capabilitiesV1;

  QMap<quint32, v4l2_fmtdesc>   pixelFormats;
  QMap<QString, v4l2_input>     videoInputs;
  QMap<QString, v4l2_standard>  videoStandards;
  QMap<QString, v4l2_queryctrl> videoControls;

  unsigned                      width;
  unsigned                      height;
  VideoBuffer::CodecID          codec;
  VideoBuffer::FieldMode        fieldMode;

  Object::Ptr<Producer>         producer;
};


} } // End of namespaces

#endif
