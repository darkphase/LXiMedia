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
#include <LXiCore>
#include "saudiobuffer.h"
#include "saudiocodec.h"
#include "sdatabuffer.h"
#include "sdatacodec.h"
#include "sencodedaudiobuffer.h"
#include "sencodeddatabuffer.h"
#include "sencodedvideobuffer.h"
#include "sgraph.h"
#include "ssubpicturebuffer.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"
#include "svideocodec.h"
#include "export.h"

namespace LXiStream {

class STimer;

namespace SInterfaces {

/*! The Node abstract class is used for processing nodes.
 */
class LXISTREAM_PUBLIC Node : public QObject
{
Q_OBJECT
public:
  explicit                      Node(SGraph *);

  /*! This method shall be invoked when the node is about to start processing
      data.
   */
  virtual bool                  start(void) = 0;

  /*! This method shall be invoked when the node is finished processing data.

      \note This method may be invoked multiple times.
   */
  virtual void                  stop(void) = 0;
};

/*! The SinkNode abstract class is used for sink nodes.
 */
class LXISTREAM_PUBLIC SinkNode : public QObject
{
Q_OBJECT
public:
  explicit                      SinkNode(SGraph *);

  /*! This method shall be invoked when the node is about to start processing
      data.
   */
  virtual bool                  start(STimer *) = 0;

  /*! This method shall be invoked when the node is finished processing data.

      \note This method may be invoked multiple times.
   */
  virtual void                  stop(void) = 0;
};

/*! The SourceNode abstract class is used for source nodes.
 */
class LXISTREAM_PUBLIC SourceNode : public QObject
{
Q_OBJECT
public:
  explicit                      SourceNode(SGraph *);

  /*! This method shall be invoked when the node is about to start processing
      data.
   */
  virtual bool                  start(void) = 0;

  /*! This method shall be invoked when the node is finished processing data.

      \note This method may be invoked multiple times.
   */
  virtual void                  stop(void) = 0;

  /*! This method shall be invoked to indicate the node has to produce data.
   */
  virtual void                  process(void) = 0;
};

/*! The FormatProber interface can be used to detect the format of a byte
    stream.
 */
class LXISTREAM_PUBLIC FormatProber : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(FormatProber)
public:
  struct ReadCallback
  {
    inline explicit             ReadCallback(const QString &path) : path(path)  { }

    const QString               path;
    virtual qint64              read(uchar *, qint64) = 0;
    virtual qint64              seek(qint64, int) = 0;
  };

  struct Format
  {
    inline                      Format(void) : name(), confidence(0) { }
    inline                      Format(const QString &name, int confidence) : name(name), confidence(confidence) { }

    QString                     name;
    int                         confidence;
  };

  struct StreamId
  {
    enum Type
    {
      Type_None               = 0x00000,
      Type_Audio              = 0x10000,
      Type_Video              = 0x20000,
      Type_Subtitle           = 0x30000
    };

    inline StreamId(void) : streamSpec(Type_None)                               { }
    inline StreamId(quint32 streamSpec) : streamSpec(streamSpec)                { }
    inline StreamId(Type type, quint16 id) : streamSpec(type | quint32(id))     { }

    inline                      operator quint32() const                        { return streamSpec; }
    inline bool                 operator<(StreamId other) const                 { return streamSpec < other.streamSpec; }

    inline Type                 streamType(void) const                          { return Type(streamSpec & 0xFFFF0000u); }
    inline quint16              streamId(void) const                            { return quint16(streamSpec & 0xFFFF); }

    quint32                     streamSpec;
  };

  struct StreamInfo : StreamId
  {
    inline StreamInfo(void) : StreamId() { memset(language, 0, sizeof(language)); }
    inline StreamInfo(Type type, quint16 id, const char *language, const QString &title)
      : StreamId(type, id),
        nativeId(0),
        title(title)
    {
      memset(this->language, 0, sizeof(this->language));
      if (language && (qstrlen(language) > 0))
        qstrncpy(this->language, language, sizeof(this->language));
    }

    char                        language[4]; //!< ISO 639-1 or ISO 639-2 language code (empty string if undefined).
    quint32                     nativeId; //!< The native stream ID, if available.
    QString                     title;
  };

  struct AudioStreamInfo : StreamInfo
  {
    inline AudioStreamInfo(void) : codec() { }
    inline AudioStreamInfo(quint16 id, const char *language, const QString &title, const SAudioCodec &codec) : StreamInfo(Type_Audio, id, language, title), codec(codec) { }

    SAudioCodec                 codec;
  };

  struct VideoStreamInfo : StreamInfo
  {
    inline VideoStreamInfo(void) : codec() { }
    inline VideoStreamInfo(quint16 id, const char *language, const QString &title, const SVideoCodec &codec) : StreamInfo(Type_Video, id, language, title), codec(codec) { }

    SVideoCodec                 codec;
  };

  struct DataStreamInfo : StreamInfo
  {
    inline DataStreamInfo(void) : codec() { }
    inline DataStreamInfo(Type type, quint16 id, const char *language, const QString &title, const SDataCodec &codec) : StreamInfo(type, id, language, title), codec(codec) { }

    SDataCodec                  codec;
    QString                     file;
  };

  struct Chapter
  {
    QString                     title;
    STime                       begin;
    STime                       end;
  };

  struct ProbeInfo : QSharedData
  {
    inline ProbeInfo(void) : size(0), isProbed(false), isReadable(false), year(0), track(0) { }

    QString                     filePath;
    QString                     path;                                           //!< Only use this if the path deviates from the filePath.
    qint64                      size;
    QDateTime                   lastModified;

    QString                     format;

    bool                        isProbed;
    bool                        isReadable;

    QString                     fileTypeName;

    struct Program
    {
      QString                   title;
      STime                     duration;
      QList<Chapter>            chapters;

      QList<AudioStreamInfo>    audioStreams;
      QList<VideoStreamInfo>    videoStreams;
      QList<DataStreamInfo>     dataStreams;
      SVideoCodec               imageCodec;

      QByteArray                thumbnail;
    };

    QList<Program>              programs;

    QString                     title;
    QString                     author;
    QString                     copyright;
    QString                     comment;
    QString                     album;
    QString                     genre;
    unsigned                    year;
    unsigned                    track;
  };

public:
  /*! Creates all registred format probers.
      \param parent   The parent object, or NULL if none.
   */
  static QList<FormatProber *>  create(QObject *parent);

protected:
  inline explicit               FormatProber(QObject *parent) : QObject(parent) { }

public:
  /*! Defines the recommended size of the buffer that is provided to
      probeFormat().
   */
  static const unsigned         defaultProbeSize;

  /*! Should probe the provided buffer for the container format (e.g. ogg,
      matroska, mpeg-ps, dvd, etc.) and return zero or more format names. The
      confidence value can be used to provide a priority. The provided file path
      can optionally be used to detect the format.

      \param buffer             The buffer to probe, can be empty if only the
                                filename should be used to determine the format.
      \param filePath           The filename to probe, the file should not be
                                opened as the provided buffer should be used to
                                determine the format. One exception is when a
                                device path is provided to probe the format of a
                                disc.
   */
  virtual QList<Format>         probeFormat(const QByteArray &buffer, const QString &filePath) = 0;

  /*! Should probe the provided object and retrieve as much information from it
      as possible. Note that for probing a file, probeMetadata() should be
      invoked on all probers returned by create() until probeInfo.isProbed is
      set to true.

      \param probeInfo          The ProbeInfo structure that needs to be filled
                                with data.
      \param callback           The callback interface that is to be used to
                                read data from the object.
      \param filePath           The filename of the object to probe, the file
                                should not be opened as the provided callback
                                should be used to determine the format. One
                                exception is when a device path is provided to
                                probe the metadata of a disc.
   */
  virtual void                  probeMetadata(ProbeInfo &probeInfo, ReadCallback *callback) = 0;
};

/*! The AbstractBufferReader interface is used for interfaces and nodes that
    provide access to buffer streams.
 */
class LXISTREAM_PUBLIC AbstractBufferReader
{
public:
  typedef FormatProber::StreamId        StreamId;
  typedef FormatProber::AudioStreamInfo AudioStreamInfo;
  typedef FormatProber::VideoStreamInfo VideoStreamInfo;
  typedef FormatProber::DataStreamInfo  DataStreamInfo;
  typedef FormatProber::Chapter         Chapter;

  struct ProduceCallback
  {
    virtual void                produce(const SEncodedAudioBuffer &) = 0;
    virtual void                produce(const SEncodedVideoBuffer &) = 0;
    virtual void                produce(const SEncodedDataBuffer &) = 0;
  };

public:
  virtual STime                 duration(void) const = 0;
  virtual bool                  setPosition(STime) = 0;
  virtual STime                 position(void) const = 0;
  virtual QList<Chapter>        chapters(void) const = 0;

  virtual QList<AudioStreamInfo> audioStreams(void) const = 0;
  virtual QList<VideoStreamInfo> videoStreams(void) const = 0;
  virtual QList<DataStreamInfo> dataStreams(void) const = 0;
  virtual void                  selectStreams(const QList<StreamId> &) = 0;
};


/*! The AbstractBufferedReader interface is used for interfaces and nodes that
    provide access to buffered streams (e.g. network streams).
 */
class LXISTREAM_PUBLIC AbstractBufferedReader : public AbstractBufferReader
{
public:
  /*! Shall buffer more data, use bufferDuration() to check the amount of data
      buffered.
      \returns false if an error occured.
      \note This method shall be thread-safe, such that buffer() and
            bufferDuration() or process() can be invoked simulateously.
   */
  virtual bool                  buffer(void) = 0;

  /*! Shall return the amount of time buffered.
   */
  virtual STime                 bufferDuration(void) const = 0;
};

/*! The BufferReader interface can be used to read serialized buffers from a
    byte stream.
 */
class LXISTREAM_PUBLIC BufferReader : public QObject,
                                      public virtual AbstractBufferedReader
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(BufferReader)
public:
  typedef FormatProber::ReadCallback ReadCallback;

public:
  static BufferReader         * create(QObject *parent, const QString &format, bool nonNull = true);

protected:
  inline explicit               BufferReader(QObject *parent) : QObject(parent) { }

  virtual bool                  openFormat(const QString &) = 0;

public:
  virtual bool                  start(ReadCallback *, ProduceCallback *, quint16 programId, bool streamed) = 0;
  virtual void                  stop(void) = 0;

  /*! Shall demux a packet from the stream.
      \returns false if an error occured.
   */
  virtual bool                  process(void) = 0;
};

/*! The NetworkBufferReader interface can be used to read serialized buffers
    from a network stream.
 */
class LXISTREAM_PUBLIC NetworkBufferReader : public QObject,
                                             public virtual AbstractBufferedReader
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(NetworkBufferReader)
public:
  typedef BufferReader::ProduceCallback ProduceCallback;

public:
  static NetworkBufferReader  * create(QObject *parent, const QString &protocol, bool nonNull = true);

protected:
  inline explicit               NetworkBufferReader(QObject *parent) : QObject(parent) { }

  virtual bool                  openProtocol(const QString &) = 0;

public:
  virtual bool                  start(const QUrl &url, ProduceCallback *, quint16 programId) = 0;
  virtual void                  stop(void) = 0;

  /*! Shall demux a packet from the buffer, or the stream if the buffer is
      empty.
      \returns false if an error occured.
   */
  virtual bool                  process(void) = 0;
};

/*! The BufferWriter interface can be used to write serialized buffers to a
    byte stream.
 */
class LXISTREAM_PUBLIC BufferWriter : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(BufferWriter)
public:
  struct WriteCallback
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

  virtual bool                  start(WriteCallback *) = 0;
  virtual void                  stop(void) = 0;

  virtual void                  process(const SEncodedAudioBuffer &) = 0;
  virtual void                  process(const SEncodedVideoBuffer &) = 0;
  virtual void                  process(const SEncodedDataBuffer &) = 0;
};

/*! The AudioDecoder interface can be used to decode audio buffers.
 */
class LXISTREAM_PUBLIC AudioDecoder : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(AudioDecoder)
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
class LXISTREAM_PUBLIC VideoDecoder : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(VideoDecoder)
public:
  enum Flag
  {
    Flag_None                 = 0x0000,

    /*! Indicates that only keyframes should be decoded.
     */
    Flag_KeyframesOnly        = 0x0001,

    /*! Indicates the video decoding should be as fast as possible at the
        expense of image quality.
     */
    Flag_Fast                 = 0x0080
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
class LXISTREAM_PUBLIC DataDecoder : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(DataDecoder)
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
  virtual SDataBufferList       decodeBuffer(const SEncodedDataBuffer &) = 0;
};

/*! The AudioEncoder interface can be used to encode audio buffers.
 */
class LXISTREAM_PUBLIC AudioEncoder : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(AudioEncoder)
public:
  enum Flag
  {
    Flag_None                 = 0x0000,

    /*! Indicates that the audio should be encoded at low quality.
        \note When neighter Flag_LowQuality nor Flag_HighQuality are set, the
              audio should be encoded at normal quality.
     */
    Flag_LowQuality           = 0x0001,

    /*! Indicates that the audio should be encoded at high quality.
        \note When neighter Flag_LowQuality nor Flag_HighQuality are set, the
              audio should be encoded at normal quality.
     */
    Flag_HighQuality          = 0x0002,

    /*! Indicates the video encoding should be as fast as possible at the
        expense of image quality. (For example by only using intra-frames)
     */
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

/*! The VideoEncoder interface can be used to encode video buffers.
 */
class LXISTREAM_PUBLIC VideoEncoder : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(VideoEncoder)
public:
  enum Flag
  {
    Flag_None                 = 0x0000,

    /*! Indicates that the video should be encoded at low quality.
        \note When neighter Flag_LowQuality nor Flag_HighQuality are set, the
              video should be encoded at normal quality.
     */
    Flag_LowQuality           = 0x0001,

    /*! Indicates that the video should be encoded at high quality.
        \note When neighter Flag_LowQuality nor Flag_HighQuality are set, the
              video should be encoded at normal quality.
     */
    Flag_HighQuality          = 0x0002,

    /*! Indicates the reference bitrate should not be exceeded, complex scenes
        shall be encoded at a lower quality.
     */
    Flag_HardBitrateLimit     = 0x0010,

    /*! Indicates the video stream is a slideshow with (mostly) stationary
        images. When possible, the encoder shall optimize its settings for
        this.
     */
    Flag_Slideshow            = 0x0040,

    /*! Indicates the video encoding should be as fast as possible at the
        expense of image quality. (For example by only using intra-frames)
     */
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

/*! The VideoFormatConverter interface can be used to convert video formats.
 */
class LXISTREAM_PUBLIC VideoFormatConverter : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(VideoFormatConverter)
public:
  static VideoFormatConverter * create(QObject *parent, const SVideoFormat &srcFormat, const SVideoFormat &dstFormat, bool nonNull = true);

  template <class _instance>
  static inline void registerClass(const SVideoFormat &srcFormat, const SVideoFormat &dstFormat, int priority = 0)
  {
    registerClass<_instance>(
        SFactory::Scheme(
            priority,
            QString(srcFormat.formatName()) + "->" + QString(dstFormat.formatName())));
  }

protected:
  inline explicit               VideoFormatConverter(QObject *parent) : QObject(parent) { }

  virtual bool                  openFormat(const SVideoFormat &srcFormat, const SVideoFormat &dstFormat) = 0;

public:
  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &) = 0;
};

/*! The AudioResampler interface can be used to provide resampling algorithms.
 */
class LXISTREAM_PUBLIC AudioResampler : public QObject
{
Q_OBJECT
S_FACTORIZABLE(AudioResampler)
protected:
  inline explicit               AudioResampler(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setSampleRate(unsigned) = 0;
  virtual unsigned              sampleRate(void) = 0;

  virtual SAudioBuffer          processBuffer(const SAudioBuffer &) = 0;
  virtual void                  compensate(float) = 0;
};

/*! The VideoDeinterlacer interface can be used to provide deinterlacing
    algorithms.
 */
class LXISTREAM_PUBLIC VideoDeinterlacer : public QObject
{
Q_OBJECT
S_FACTORIZABLE(VideoDeinterlacer)
protected:
  inline explicit               VideoDeinterlacer(QObject *parent) : QObject(parent) { }

public:
  virtual SVideoBufferList      processBuffer(const SVideoBuffer &) = 0;
};

/*! The VideoResizer interface can be used to provide scaling algorithms.
 */
class LXISTREAM_PUBLIC VideoResizer : public QObject
{
Q_OBJECT
S_FACTORIZABLE(VideoResizer)
protected:
  inline explicit               VideoResizer(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setSize(const SSize &) = 0;
  virtual SSize                 size(void) const = 0;
  virtual void                  setAspectRatioMode(Qt::AspectRatioMode) = 0;
  virtual Qt::AspectRatioMode   aspectRatioMode(void) const = 0;

  virtual bool                  needsResize(const SVideoFormat &) = 0;
  virtual SVideoBuffer          processBuffer(const SVideoBuffer &) = 0;
};


} } // End of namespaces

Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::AudioDecoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::VideoDecoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::AudioEncoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::VideoEncoder::Flags)

#endif
