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

#ifndef LXSTREAM_SINTERFACES_H
#define LXSTREAM_SINTERFACES_H

#include <QtCore>
#include "saudiobuffer.h"
#include "saudiocodec.h"
#include "sdatacodec.h"
#include "sencodedaudiobuffer.h"
#include "sencodeddatabuffer.h"
#include "sencodedvideobuffer.h"
#include "sfactory.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"
#include "svideocodec.h"

namespace LXiStream {

class SGraph;
class STimer;

namespace SInterfaces {

/*! The Module interface is used to register modules.

    \sa SSystem::loadModule()
 */
class Module
{
public:
  virtual                       ~Module();

  virtual void                  registerClasses(void) = 0;
  virtual void                  unload(void) = 0;
  virtual QByteArray            about(void) = 0;
};

/*! The Node interface is used for processing nodes.
 */
class Node
{
public:
  explicit                      Node(SGraph *);
  virtual                       ~Node();

protected:
  SGraph                * const graph;
};

/*! The SinkNode interface is used for sink nodes.
 */
class SinkNode
{
public:
  explicit                      SinkNode(SGraph *);
  virtual                       ~SinkNode();

  virtual bool                  start(STimer *) = 0;
  virtual void                  stop(void) = 0;

protected:
  SGraph                * const graph;
};

/*! The SourceNode interface is used for source nodes.
 */
class SourceNode
{
public:
  explicit                      SourceNode(SGraph *);
  virtual                       ~SourceNode();

  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual void                  process(void) = 0;

protected:
  SGraph                * const graph;
  mutable QMutex                mutex;
};

/*! The BufferReader interface can be used to read serialized buffers from a
    byte stream.
 */
class FormatProber : public QObject,
                     public SFactorizable<FormatProber>
{
Q_OBJECT
public:
  struct Format
  {
    inline                      Format(void) : name(), confidence(0) { }
    inline                      Format(const QString &name, int confidence) : name(name), confidence(confidence) { }

    QString                     name;
    int                         confidence;
  };

  struct StreamInfo
  {
    inline StreamInfo(void) : streamId(0) { }
    inline StreamInfo(quint16 streamId, const char *language)
      : streamId(streamId)
    {
      if (language && (qstrlen(language) > 0))
        qstrncpy(this->language, language, sizeof(this->language));
      else
        this->language[0] = 0;
    }

    quint16                     streamId;
    char                        language[4]; //!< ISO 639 3-letter language code (empty string if undefined).
  };

  struct AudioStreamInfo : StreamInfo
  {
    inline AudioStreamInfo(void) : codec() { }
    inline AudioStreamInfo(quint16 streamId, const char *language, const SAudioCodec &codec) : StreamInfo(streamId, language), codec(codec) { }

    SAudioCodec                 codec;
  };

  struct VideoStreamInfo : StreamInfo
  {
    inline VideoStreamInfo(void) : codec() { }
    inline VideoStreamInfo(quint16 streamId, const char *language, const SVideoCodec &codec) : StreamInfo(streamId, language), codec(codec) { }

    SVideoCodec                 codec;
  };

  struct DataStreamInfo : StreamInfo
  {
    inline DataStreamInfo(void) : codec() { }
    inline DataStreamInfo(quint16 streamId, const char *language, const SDataCodec &codec) : StreamInfo(streamId, language), codec(codec) { }

    SDataCodec                  codec;
    QString                     file;
  };

  struct ProbeInfo
  {
    inline ProbeInfo(void) : isProbed(false), isReadable(false), year(0), track(0) { }

    QString                     format;

    bool                        isProbed;
    bool                        isReadable;

    QString                     fileTypeName;

    STime                       duration;
    QList<AudioStreamInfo>      audioStreams;
    QList<VideoStreamInfo>      videoStreams;
    QList<DataStreamInfo>       dataStreams;
    SVideoCodec                 imageCodec;

    QString                     title;
    QString                     author;
    QString                     copyright;
    QString                     comment;
    QString                     album;
    QString                     genre;
    unsigned                    year;
    unsigned                    track;

    QList<QByteArray>           thumbnails;
  };

public:
  static QList<FormatProber *>  create(QObject *parent);

protected:
  inline explicit               FormatProber(QObject *parent) : QObject(parent) { }

public:
  /*! Should probe the provided buffer for the container format used (e.g. ogg,
      matroska, mpeg-ps, etc.) and return zero or more format names. The
      confidence value can be used to provide a priority.
   */
  virtual QList<Format>         probeFormat(const QByteArray &) = 0;

  /*! Should probe the provided path name and retrieve as much information from
      it as possible; the file shall _not_ to be opened. The prober should draw
      its conclusions based on the file name and extention and, if possible, the
      path.
   */
  virtual void                  probeName(ProbeInfo &, const QString &) = 0;

  /*! Should probe the provided file and retrieve as much information from it
      as possible.
   */
  virtual void                  probeFile(ProbeInfo &, QIODevice *) = 0;
};

/*! The BufferReader interface can be used to read serialized buffers from a
    byte stream.
 */
class BufferReader : public QObject,
                     public SFactorizable<BufferReader>
{
Q_OBJECT
public:
  struct Callback
  {
    virtual qint64              read(uchar *, qint64) = 0;
    virtual qint64              seek(qint64, int) = 0;

    virtual void                produce(const SEncodedAudioBuffer &) = 0;
    virtual void                produce(const SEncodedVideoBuffer &) = 0;
    virtual void                produce(const SEncodedDataBuffer &) = 0;
  };

  typedef FormatProber::AudioStreamInfo AudioStreamInfo;
  typedef FormatProber::VideoStreamInfo VideoStreamInfo;
  typedef FormatProber::DataStreamInfo  DataStreamInfo;

public:
  static BufferReader         * create(QObject *parent, const QString &format, bool nonNull = true);

protected:
  inline explicit               BufferReader(QObject *parent) : QObject(parent) { }

  virtual bool                  openFormat(const QString &) = 0;

public:
  virtual bool                  start(Callback *, bool streamed) = 0;
  virtual void                  stop(void) = 0;
  virtual bool                  process(void) = 0;

  virtual STime                 duration(void) const = 0;
  virtual bool                  setPosition(STime) = 0;
  virtual STime                 position(void) const = 0;

  virtual QList<AudioStreamInfo> audioStreams(void) const = 0;
  virtual QList<VideoStreamInfo> videoStreams(void) const = 0;
  virtual QList<DataStreamInfo>  dataStreams(void) const = 0;
  virtual void                  selectStreams(const QList<quint16> &) = 0;
};

/*! The BufferWriter interface can be used to write serialized buffers to a
    byte stream.
 */
class BufferWriter : public QObject,
                     public SFactorizable<BufferWriter>
{
Q_OBJECT
public:
  struct Callback
  {
    virtual void                write(const uchar *, qint64) = 0;
  };

public:
  static BufferWriter         * create(QObject *parent, const QString &format, bool nonNull = true);

protected:
  inline explicit               BufferWriter(QObject *parent) : QObject(parent) { }

  virtual bool                  openFormat(const QString &) = 0;

public:
  virtual bool                  createStreams(const QList<SAudioCodec> &, const QList<SVideoCodec> &, STime) = 0;

  virtual bool                  start(Callback *) = 0;
  virtual void                  stop(void) = 0;

  virtual void                  process(const SEncodedAudioBuffer &) = 0;
  virtual void                  process(const SEncodedVideoBuffer &) = 0;
  virtual void                  process(const SEncodedDataBuffer &) = 0;
};

/*! The AudioDecoder interface can be used to decode audio buffers.
 */
class AudioDecoder : public QObject,
                     public SFactorizable<AudioDecoder>
{
Q_OBJECT
public:
  enum Flag
  {
    Flag_None                 = 0x0000,
    Flag_DownsampleToStereo   = 0x0001
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static AudioDecoder         * create(QObject *parent, const SAudioCodec &codec, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               AudioDecoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SAudioCodec &, Flags) = 0;

public:
  virtual SAudioBufferList      decodeBuffer(const SEncodedAudioBuffer &) = 0;
};

/*! The VideoDecoder interface can be used to decode audio buffers.
 */
class VideoDecoder : public QObject,
                     public SFactorizable<VideoDecoder>
{
Q_OBJECT
public:
  enum Flag
  {
    Flag_None                 = 0x0000,
    Flag_KeyframesOnly        = 0x0001
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static VideoDecoder         * create(QObject *parent, const SVideoCodec &codec, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               VideoDecoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SVideoCodec &, Flags) = 0;

public:
  virtual SVideoBufferList      decodeBuffer(const SEncodedVideoBuffer &) = 0;
};

/*! The SubtitleDecoder interface can be used to decode subtitle buffers.
 */
class DataDecoder : public QObject,
                    public SFactorizable<DataDecoder>
{
Q_OBJECT
public:
  enum Flag
  {
    Flag_None                 = 0x0000,
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static DataDecoder          * create(QObject *parent, const SDataCodec &codec, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               DataDecoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SDataCodec &, Flags) = 0;

public:
  virtual SSubtitleBufferList   decodeBuffer(const SEncodedDataBuffer &) = 0;
};

/*! The BufferEncoder interface can be used to encode buffers.
 */
class AudioEncoder : public QObject,
                     public SFactorizable<AudioEncoder>
{
Q_OBJECT
public:
  enum Flag
  {
    Flag_None                 = 0x0000,
    Flag_LowQuality           = 0x0001,
    Flag_HighQuality          = 0x0002,
    Flag_Fast                 = 0x0080
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static AudioEncoder         * create(QObject *parent, const SAudioCodec &codec, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               AudioEncoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SAudioCodec &, Flags) = 0;

public:
  virtual SAudioCodec           codec(void) const = 0;
  virtual SEncodedAudioBufferList encodeBuffer(const SAudioBuffer &) = 0;
};

/*! The BufferEncoder interface can be used to encode buffers.
 */
class VideoEncoder : public QObject,
                     public SFactorizable<VideoEncoder>
{
Q_OBJECT
public:
  enum Flag
  {
    Flag_None                 = 0x0000,
    Flag_LowQuality           = 0x0001,
    Flag_HighQuality          = 0x0002,
    Flag_Fast                 = 0x0080
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static VideoEncoder         * create(QObject *parent, const SVideoCodec &codec, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               VideoEncoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SVideoCodec &, Flags) = 0;

public:
  virtual SVideoCodec           codec(void) const = 0;
  virtual SEncodedVideoBufferList encodeBuffer(const SVideoBuffer &) = 0;
};

/*! The AudioInput interface can be used to provide audio input devices.
 */
class AudioInput : public QObject,
                   public SFactorizable<AudioInput>
{
Q_OBJECT
protected:
  inline explicit               AudioInput(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setFormat(const SAudioFormat &) = 0;
  virtual SAudioFormat          format(void) = 0;

  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual void                  process(void) = 0;

signals:
  void                          produce(const SAudioBuffer &);
};

/*! The AudioOutput interface can be used to provide audio output devices.
 */
class AudioOutput : public QObject,
                    public SFactorizable<AudioOutput>
{
Q_OBJECT
protected:
  inline explicit               AudioOutput(QObject *parent) : QObject(parent) { }

public:
  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual STime                 latency(void) const = 0;

public slots:
  virtual void                  consume(const SAudioBuffer &) = 0;
};

/*! The VideoInput interface can be used to provide video input devices.
 */
class VideoInput : public QObject,
                   public SFactorizable<VideoInput>
{
Q_OBJECT
protected:
  inline explicit               VideoInput(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setFormat(const SVideoFormat &) = 0;
  virtual SVideoFormat          format(void) = 0;

  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual void                  process(void) = 0;

signals:
  void                          produce(const SVideoBuffer &);
};

/*! The AudioResampler interface can be used to provide resampling algorithms.
 */
class AudioResampler : public QObject,
                       public SFactorizable<AudioResampler>
{
Q_OBJECT
protected:
  inline explicit               AudioResampler(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setFormat(const SAudioFormat &) = 0;
  virtual SAudioFormat          format(void) = 0;

  virtual SAudioBuffer          processBuffer(const SAudioBuffer &) = 0;
};

/*! The VideoDeinterlacer interface can be used to provide deinterlacing
    algorithms.
 */
class VideoDeinterlacer : public QObject,
                          public SFactorizable<VideoDeinterlacer>
{
Q_OBJECT
protected:
  inline explicit               VideoDeinterlacer(QObject *parent) : QObject(parent) { }

public:
  virtual SVideoBufferList      processBuffer(const SVideoBuffer &) = 0;
};

/*! The VideoResizer interface can be used to provide scaling algorithms.
 */
class VideoResizer : public QObject,
                     public SFactorizable<VideoResizer>
{
Q_OBJECT
protected:
  inline explicit               VideoResizer(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setSize(const SSize &) = 0;
  virtual SSize                 size(void) const = 0;
  virtual void                  setAspectRatioMode(Qt::AspectRatioMode) = 0;
  virtual Qt::AspectRatioMode   aspectRatioMode(void) const = 0;

  virtual SVideoBuffer          processBuffer(const SVideoBuffer &) = 0;
};


} } // End of namespaces

Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::AudioDecoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::VideoDecoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::AudioEncoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::VideoEncoder::Flags)

Q_DECLARE_INTERFACE(LXiStream::SInterfaces::Module, "nl.dds.admiraal.www.LXiStream.SInterfaces.Module/1.0")
Q_DECLARE_INTERFACE(LXiStream::SInterfaces::Node, "nl.dds.admiraal.www.LXiStream.SInterfaces.Node/1.0")
Q_DECLARE_INTERFACE(LXiStream::SInterfaces::SinkNode, "nl.dds.admiraal.www.LXiStream.SInterfaces.SinkNode/1.0")
Q_DECLARE_INTERFACE(LXiStream::SInterfaces::SourceNode, "nl.dds.admiraal.www.LXiStream.SInterfaces.SourceNode/1.0")

#endif
