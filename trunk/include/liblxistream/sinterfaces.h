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
      \returns true if data has been produced.
   */
  virtual bool                  process(void) = 0;
};

/*! The Filesystem interface class is used to expose media filesystems.
 */
class LXISTREAM_PUBLIC Filesystem : public QObject
{
Q_OBJECT
S_FACTORIZABLE(Filesystem)
public:
  struct Info
  {
    inline Info(void) : isDir(false), isReadable(false), size(0), lastModified() { }

    bool                        isDir;
    bool                        isReadable;
    qint64                      size;
    QDateTime                   lastModified;
  };

protected:
  inline explicit               Filesystem(QObject *parent) : QObject(parent) { }

public:
  virtual bool                  openDirectory(const QUrl &) = 0;

  virtual QStringList           entryList(QDir::Filters, QDir::SortFlags) const = 0;
  virtual QUrl                  filePath(const QString &fileName) const = 0;
  virtual Info                  readInfo(const QString &fileName) const = 0;
  virtual QIODevice           * openFile(const QString &fileName) const = 0;
};

/*! The FormatProber interface can be used to detect the format of a byte
    stream.
 */
class LXISTREAM_PUBLIC FormatProber : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(FormatProber)
public:
  struct Format
  {
    inline                      Format(void) : name(), confidence(0) { }
    inline                      Format(const QString &name, int confidence) : name(name), confidence(confidence) { }

    QString                     name;
    int                         confidence;
  };

  struct LXISTREAM_PUBLIC StreamId
  {
    enum Type
    {
      Type_None               = 0x0000,
      Type_Audio              = 0x0001,
      Type_Video              = 0x0002,
      Type_Subtitle           = 0x0003,
      Type_Flags              = 0xF000,
      Type_Flag_Native        = 0x1000
    };

    inline StreamId(void) : type(Type_None), id(0)                              { }
    inline StreamId(quint16 type, quint16 id) : type(type), id(id)              { }

    inline bool                 operator==(const StreamId &other) const         { return (type == other.type) && (id == other.id); }
    inline bool                 operator!=(const StreamId &other) const         { return !operator==(other); }
    inline bool                 operator<(const StreamId &other) const          { return id < other.id; }

    QString                     toString(void) const;
    static StreamId             fromString(const QString &);

    quint16                     type;
    quint16                     id;
  };

  struct LXISTREAM_PUBLIC StreamInfo : StreamId
  {
    inline StreamInfo(void) : StreamId() { }
    inline StreamInfo(StreamId id, const QString &language, const QString &title)
      : StreamId(id), language(language), title(title)
    {
    }

    QString                     fullName(void) const;

    QString                     language; //!< ISO 639-1 or ISO 639-2 language code (empty string if undefined).
    QString                     title;
  };

  struct AudioStreamInfo : StreamInfo
  {
    inline AudioStreamInfo(void) : codec() { }
    inline AudioStreamInfo(StreamId id, const QString &language, const QString &title, const SAudioCodec &codec)
      : StreamInfo(id, language, title), codec(codec)
    {
      type = (type & Type_Flags) | Type_Audio;
    }

    SAudioCodec                 codec;
  };

  struct VideoStreamInfo : StreamInfo
  {
    inline VideoStreamInfo(void) : codec() { }
    inline VideoStreamInfo(StreamId id, const QString &language, const QString &title, const SVideoCodec &codec)
      : StreamInfo(id, language, title), codec(codec)
    {
      type = (type & Type_Flags) | Type_Video;
    }

    SVideoCodec                 codec;
  };

  struct DataStreamInfo : StreamInfo
  {
    inline DataStreamInfo(void) : codec() { }
    inline DataStreamInfo(StreamId id, const QString &language, const QString &title, const SDataCodec &codec)
      : StreamInfo(id, language, title), codec(codec)
    {
    }

    SDataCodec                  codec;
    QUrl                        file;
  };

  struct Chapter
  {
    QString                     title;
    STime                       begin;
    STime                       end;
  };

  struct ProbeInfo : QSharedData
  {
    /*! The type of file.
     */
    enum FileType
    {
      FileType_None           = 0,                                              //!< Unknown file type.


      FileType_Audio          = 10,                                             //!< Audio file.
      FileType_Video,                                                           //!< Video file.
      FileType_Image,                                                           //!< Image file.


      FileType_Directory      = 20,                                             //!< Filesystem directry.
      FileType_Drive,                                                           //!< A drive (e.g. removable disk, network drive, etc.).
      FileType_Disc                                                             //!< A media disc (e.g. DVD).
    };

    /*! A title in the file, usually a file has only one title, but DVD images
        may have multiple titles.
     */
    struct Title
    {
      STime                     duration;                                       //!< The duration of the title.
      QList<Chapter>            chapters;                                       //!< A list of chapters in the title.

      QList<AudioStreamInfo>    audioStreams;                                   //!< The audio streams in the title.
      QList<VideoStreamInfo>    videoStreams;                                   //!< The video streams in the title.
      QList<DataStreamInfo>     dataStreams;                                    //!< The data streams in the title.
      SVideoCodec               imageCodec;                                     //!< If the title is a signle image, this holds the image codec..

      SVideoBuffer              thumbnail;                                      //!< A thumbnail of the title.
    };

    inline ProbeInfo(void)
      : isReadable(false), isFileInfoRead(false),
        isFormatProbed(false), isContentProbed(false)
    {
      fileInfo.isDir = false;
      fileInfo.size = 0;
      format.fileType = FileType_None;
      format.isComplexFile = false;
    }

    QUrl                        filePath;                                       //!< The full absolute path to the file.

    bool                        isReadable;                                     //!< True if the file can be read.
    bool                        isFileInfoRead;                                 //!< True if the fileInfo structure has been read.
    bool                        isFormatProbed;                                 //!< True if the format structure has been read.
    bool                        isContentProbed;                                //!< True if the content structure has been read.

    /*! Contains the filesystem information.
     */
    struct
    {
      bool                      isDir;
      qint64                    size;
      QDateTime                 lastModified;
    }                           fileInfo;

    /*! Contains the file format information.
     */
    struct
    {
      QString                   format;
      FileType                  fileType;
      QString                   fileTypeName;
      bool                      isComplexFile;                                  //!< True if this file has multiple streams or is >= 10 min.
      QByteArray                quickHash;
      QMap<QString, QVariant>   metadata;
    }                           format;

    /*! Contains the file content information.
     */
    struct
    {
      QList<Title>              titles;
    }                           content;
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
  virtual QList<Format>         probeFormat(const QByteArray &buffer, const QUrl &filePath) = 0;

  /*! Should probe the provided device and retrieve all information in
      probeInfo.format. Note that for probing a file, probeFormat() should be
      invoked on all probers returned by create() until probeInfo.isFormatProbed
      is set to true.

      \param probeInfo          The ProbeInfo structure that needs to be filled
                                with data.
      \param ioDevice           The QIODevice that is to be used to read data.
   */
  virtual void                  probeFormat(ProbeInfo &probeInfo, QIODevice *ioDevice) = 0;

  /*! Should probe the provided object and retrieve all information in
      probeInfo.content. Note that for probing a file, probeContent() should be
      invoked on all probers returned by create() until
      probeInfo.isContentProbed is set to true.

      \param probeInfo          The ProbeInfo structure that needs to be filled
                                with data.
      \param ioDevice           The QIODevice that is to be used to read data.
   */
  virtual void                  probeContent(ProbeInfo &probeInfo, QIODevice *ioDevice, const QSize &thumbSize) = 0;
};

class AbstractBufferReader;

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
  static AudioDecoder         * create(QObject *parent, const SAudioCodec &codec, AbstractBufferReader * = NULL, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               AudioDecoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SAudioCodec &, AbstractBufferReader *, Flags) = 0;

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
  static VideoDecoder         * create(QObject *parent, const SVideoCodec &codec, AbstractBufferReader * = NULL, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               VideoDecoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SVideoCodec &, AbstractBufferReader *, Flags) = 0;

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
  static DataDecoder          * create(QObject *parent, const SDataCodec &codec, AbstractBufferReader *, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               DataDecoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SDataCodec &, AbstractBufferReader *, Flags) = 0;

public:
  virtual SDataBufferList       decodeBuffer(const SEncodedDataBuffer &) = 0;
};

/*! The AbstractBufferReader interface is used for interfaces and nodes that
    provide access to buffer streams.
 */
class LXISTREAM_PUBLIC AbstractBufferReader : public QObject
{
Q_OBJECT
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
  inline explicit               AbstractBufferReader(QObject *parent) : QObject(parent) { }

  virtual STime                 duration(void) const = 0;
  virtual bool                  setPosition(STime) = 0;
  virtual STime                 position(void) const = 0;
  virtual QList<Chapter>        chapters(void) const = 0;

  virtual int                   numTitles(void) const = 0;
  virtual QList<AudioStreamInfo> audioStreams(int title) const = 0;
  virtual QList<VideoStreamInfo> videoStreams(int title) const = 0;
  virtual QList<DataStreamInfo> dataStreams(int title) const = 0;
  virtual void                  selectStreams(int title, const QVector<StreamId> &) = 0;

  /*! Shall demux a packet from the stream.
      \returns false if an error occured.
   */
  virtual bool                  process(void) = 0;
};

/*! The BufferReader interface can be used to read serialized buffers from a
    byte stream.
 */
class LXISTREAM_PUBLIC BufferReader : public AbstractBufferReader
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(BufferReader)
public:
  static BufferReader         * create(QObject *parent, const QString &format, bool nonNull = true);

protected:
  inline explicit               BufferReader(QObject *parent) : AbstractBufferReader(parent) { }

  virtual bool                  openFormat(const QString &) = 0;

public:
  virtual bool                  start(QIODevice *, ProduceCallback *, bool streamed) = 0;
  virtual void                  stop(void) = 0;
};

/*! The NetworkBufferReader interface can be used to read serialized buffers
    from a network stream.
 */
class LXISTREAM_PUBLIC NetworkBufferReader : public AbstractBufferReader
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(NetworkBufferReader)
public:
  typedef BufferReader::ProduceCallback ProduceCallback;

public:
  static NetworkBufferReader  * create(QObject *parent, const QString &protocol, bool nonNull = true);

protected:
  inline explicit               NetworkBufferReader(QObject *parent) : AbstractBufferReader(parent) { }

  virtual bool                  openProtocol(const QString &) = 0;

public:
  virtual bool                  start(const QUrl &url, ProduceCallback *) = 0;
  virtual void                  stop(void) = 0;

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

class BufferWriter;

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

    /*! Indicates the audio encoding may not be delayed (encodig may be delayed
        due to parallelization).
     */
    Flag_NoDelay              = 0x0040,

    /*! Indicates the audio encoding should be as fast as possible at the
        expense of sound quality.
     */
    Flag_Fast                 = 0x0080
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static AudioEncoder         * create(QObject *parent, const SAudioCodec &codec, BufferWriter * = NULL, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               AudioEncoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SAudioCodec &, BufferWriter *, Flags) = 0;

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
    Flag_Slideshow            = 0x0020,

    /*! Indicates the video encoding may not be delayed (encodig may be delayed
        due to the encoding of B-frames and/or parallelization).
     */
    Flag_NoDelay              = 0x0040,

    /*! Indicates the video encoding should be as fast as possible at the
        expense of image quality. (For example by only using intra-frames)
     */
    Flag_Fast                 = 0x0080
  };
  Q_DECLARE_FLAGS(Flags, Flag)

public:
  static VideoEncoder         * create(QObject *parent, const SVideoCodec &codec, BufferWriter * = NULL, Flags = Flag_None, bool nonNull = true);

protected:
  inline explicit               VideoEncoder(QObject *parent) : QObject(parent) { }

  virtual bool                  openCodec(const SVideoCodec &, BufferWriter *, Flags) = 0;

public:
  virtual SVideoCodec           codec(void) const = 0;
  virtual SEncodedVideoBufferList encodeBuffer(const SVideoBuffer &) = 0;
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
    virtual qint64              seek(qint64, int) = 0;
  };

public:
  static BufferWriter         * create(QObject *parent, const QString &format, bool nonNull = true);

protected:
  inline explicit               BufferWriter(QObject *parent) : QObject(parent) { }

  virtual bool                  openFormat(const QString &) = 0;

public:
  virtual bool                  addStream(const AudioEncoder *, STime) = 0;
  virtual bool                  addStream(const VideoEncoder *, STime) = 0;

  virtual bool                  start(WriteCallback *, bool sequential) = 0;
  virtual void                  stop(void) = 0;

  virtual void                  process(const SEncodedAudioBuffer &) = 0;
  virtual void                  process(const SEncodedVideoBuffer &) = 0;
  virtual void                  process(const SEncodedDataBuffer &) = 0;
};

/*! The AudioFormatConverter interface can be used to convert audio formats.
 */
class LXISTREAM_PUBLIC AudioFormatConverter : public QObject
{
Q_OBJECT
S_FACTORIZABLE_NO_CREATE(AudioFormatConverter)
public:
  static AudioFormatConverter * create(QObject *parent, const SAudioFormat &srcFormat, const SAudioFormat &dstFormat, bool nonNull = true);

  template <class _instance>
  static inline void registerClass(const SAudioFormat &srcFormat, const SAudioFormat &dstFormat, int priority = 0)
  {
    registerClass<_instance>(
        SFactory::Scheme(
            priority,
            QString(srcFormat.formatName()) + "->" + QString(dstFormat.formatName())));
  }

protected:
  inline explicit               AudioFormatConverter(QObject *parent) : QObject(parent) { }

  virtual bool                  openFormat(const SAudioFormat &srcFormat, const SAudioFormat &dstFormat) = 0;

public:
  virtual SAudioBuffer          convertBuffer(const SAudioBuffer &) = 0;
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

/*! The SubtitleRenderer interface can be used to provide subtitle rendering.
 */
class LXISTREAM_PUBLIC SubtitleRenderer : public QObject
{
Q_OBJECT
S_FACTORIZABLE(SubtitleRenderer)
protected:
  inline explicit               SubtitleRenderer(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setFontRatio(float r) = 0;                      //!< Sets the relative font size (1.0 = the font will be as big as the image).
  virtual float                 fontRatio(void) const = 0;                      //!< Returns the relative font size.

  /*! Shall prepare a subtitle for rendering so that the next  invoke of
      processBuffer() with this subtitle can be faster. This method shall return
      immediately, the subtitle will be prepared on a thread of the global
      QThreadPool.
   */
  virtual void                  prepareSubtitle(const SSubtitleBuffer &, const SSize &) = 0;

  /*! Shall render the specified subtitle on the specified video buffer. If the
      specified subtitle has not been prepared yet it will be prepared first.
   */
  virtual SVideoBuffer          processBuffer(const SVideoBuffer &, const SSubtitleBuffer &) = 0;
};

} } // End of namespaces

Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::AudioDecoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::VideoDecoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::AudioEncoder::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SInterfaces::VideoEncoder::Flags)

#endif
