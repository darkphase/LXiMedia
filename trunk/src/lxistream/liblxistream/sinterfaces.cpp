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

#include "sinterfaces.h"

namespace LXiStream {
namespace SInterfaces {

S_FACTORIZABLE_INSTANCE_NO_CREATE(FormatProber);
S_FACTORIZABLE_INSTANCE_NO_CREATE(BufferReader);
S_FACTORIZABLE_INSTANCE_NO_CREATE(BufferWriter);
S_FACTORIZABLE_INSTANCE_NO_CREATE(AudioDecoder);
S_FACTORIZABLE_INSTANCE_NO_CREATE(VideoDecoder);
S_FACTORIZABLE_INSTANCE_NO_CREATE(DataDecoder);
S_FACTORIZABLE_INSTANCE_NO_CREATE(AudioEncoder);
S_FACTORIZABLE_INSTANCE_NO_CREATE(VideoEncoder);
S_FACTORIZABLE_INSTANCE_NO_CREATE(VideoFormatConverter);
S_FACTORIZABLE_INSTANCE(AudioResampler);
S_FACTORIZABLE_INSTANCE(VideoDeinterlacer);
S_FACTORIZABLE_INSTANCE(VideoResizer);


Node::Node(SGraph *graph)
  : QObject(graph)
{
  if (graph)
    graph->addNode(this);
}

Node::~Node()
{
}


SinkNode::SinkNode(SGraph *graph)
  : QObject(graph)
{
  if (graph)
    graph->addNode(this);
}

SinkNode::~SinkNode()
{
}


SourceNode::SourceNode(SGraph *graph)
  : QObject(graph)
{
  if (graph)
    graph->addNode(this);
}

SourceNode::~SourceNode()
{
}


const unsigned FormatProber::defaultProbeSize = 16384;

QList<FormatProber *> FormatProber::create(QObject *parent)
{
  return factory().createObjects<FormatProber>(parent);
}

/*! Creates a buffer reader for the specified format.
    \param parent   The parent object, or NULL if none.
    \param format   The data format of the serialized data (e.g. "mpeg" or "flv").
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
BufferReader * BufferReader::create(QObject *parent, const QString &format, bool nonNull)
{
  BufferReader * bufferReader =
      static_cast<BufferReader *>(factory().createObject(staticMetaObject.className(), parent, format, nonNull));

  if (bufferReader)
  if (!bufferReader->openFormat(format))
  {
    delete bufferReader;
    bufferReader = NULL;
  }

  if (nonNull && (bufferReader == NULL))
    qFatal("Failed to open input format \"%s\".", format.toAscii().data());

  return bufferReader;
}

BufferReaderNode::~BufferReaderNode()
{
}

/*! Creates a buffer writer for the specified format.
    \param parent   The parent object, or NULL if none.
    \param format   The data format of the serialized data (e.g. "mpeg" or "flv").
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
BufferWriter * BufferWriter::create(QObject *parent, const QString &format, bool nonNull)
{
  BufferWriter * bufferWriter =
      static_cast<BufferWriter *>(factory().createObject(staticMetaObject.className(), parent, format, nonNull));

  if (bufferWriter)
  if (!bufferWriter->openFormat(format))
  {
    delete bufferWriter;
    bufferWriter = NULL;
  }

  if (nonNull && (bufferWriter == NULL))
    qFatal("Failed to open output format \"%s\".", format.toAscii().data());

  return bufferWriter;
}

/*! Creates an audio decoder for the specified codec.
    \param parent   The parent object, or NULL if none.
    \param codec    The codec to use.
    \param flags    The flags to use, see SAudioDecoderNode::Flags.
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
AudioDecoder * AudioDecoder::create(QObject *parent, const SAudioCodec &codec, Flags flags, bool nonNull)
{
  AudioDecoder * audioDecoder =
      static_cast<AudioDecoder *>(factory().createObject(staticMetaObject.className(), parent, codec.codec(), nonNull));

  if (audioDecoder)
  if (!audioDecoder->openCodec(codec, flags))
  {
    delete audioDecoder;
    audioDecoder = NULL;
  }

  if (nonNull && (audioDecoder == NULL))
    qFatal("Failed to open audio decoder for \"%s\".", codec.codec().toAscii().data());

  return audioDecoder;
}

/*! Creates a video decoder for the specified codec.
    \param parent   The parent object, or NULL if none.
    \param codec    The codec to use.
    \param flags    The flags to use, see SVideoDecoderNode::Flags.
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
VideoDecoder * VideoDecoder::create(QObject *parent, const SVideoCodec &codec, Flags flags, bool nonNull)
{
  VideoDecoder * videoDecoder =
      static_cast<VideoDecoder *>(factory().createObject(staticMetaObject.className(), parent, codec.codec(), nonNull));

  if (videoDecoder)
  if (!videoDecoder->openCodec(codec, flags))
  {
    delete videoDecoder;
    videoDecoder = NULL;
  }

  if (nonNull && (videoDecoder == NULL))
    qFatal("Failed to open video decoder for \"%s\".", codec.codec().toAscii().data());

  return videoDecoder;
}

/*! Creates a data decoder for the specified codec.
    \param parent   The parent object, or NULL if none.
    \param codec    The codec to use.
    \param flags    The flags to use, see SDataDecoderNode::Flags.
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
DataDecoder * DataDecoder::create(QObject *parent, const SDataCodec &codec, Flags flags, bool nonNull)
{
  DataDecoder * dataDecoder =
      static_cast<DataDecoder *>(factory().createObject(staticMetaObject.className(), parent, codec.codec(), nonNull));

  if (dataDecoder)
  if (!dataDecoder->openCodec(codec, flags))
  {
    delete dataDecoder;
    dataDecoder = NULL;
  }

  if (nonNull && (dataDecoder == NULL))
    qFatal("Failed to open data decoder for \"%s\".", codec.codec().toAscii().data());

  return dataDecoder;
}

/*! Creates an audio encoder for the specified codec.
    \param parent   The parent object, or NULL if none.
    \param codec    The codec to use.
    \param flags    The flags to use, see SAudioDecoderNode::Flags.
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
AudioEncoder * AudioEncoder::create(QObject *parent, const SAudioCodec &codec, Flags flags, bool nonNull)
{
  AudioEncoder * audioEncoder =
      static_cast<AudioEncoder *>(factory().createObject(staticMetaObject.className(), parent, codec.codec(), nonNull));

  if (audioEncoder)
  if (!audioEncoder->openCodec(codec, flags))
  {
    delete audioEncoder;
    audioEncoder = NULL;
  }

  if (nonNull && (audioEncoder == NULL))
    qFatal("Failed to open audio encoder for \"%s\".", codec.codec().toAscii().data());

  return audioEncoder;
}

/*! Creates a video encoder for the specified codec.
    \param parent   The parent object, or NULL if none.
    \param codec    The codec to use.
    \param flags    The flags to use, see SAudioDecoderNode::Flags.
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
VideoEncoder * VideoEncoder::create(QObject *parent, const SVideoCodec &codec, Flags flags, bool nonNull)
{
  VideoEncoder * videoEncoder =
      static_cast<VideoEncoder *>(factory().createObject(staticMetaObject.className(), parent, codec.codec(), nonNull));

  if (videoEncoder)
  if (!videoEncoder->openCodec(codec, flags))
  {
    delete videoEncoder;
    videoEncoder = NULL;
  }

  if (nonNull && (videoEncoder == NULL))
    qFatal("Failed to open video encoder for \"%s\".", codec.codec().toAscii().data());

  return videoEncoder;
}

/*! Creates a video format converter for the specified source and destination
    formats.
    \param parent   The parent object, or NULL if none.
    \param srcFormat The source format.
    \param dstFormat The destination format.
    \param nonNull  When true, the default, the method will throw a qFatal() if
                    the object can not be created, the method is guaranteed not
                    to return a null pointer in this case. When false, the
                    method will return a null pointer if the object can not be
                    created.
 */
VideoFormatConverter * VideoFormatConverter::create(QObject *parent, const SVideoFormat &srcFormat, const SVideoFormat &dstFormat, bool nonNull)
{
  const QString scheme = QString(srcFormat.formatName()) + "->" + QString(dstFormat.formatName());
  VideoFormatConverter * videoFormatConverter =
      static_cast<VideoFormatConverter *>(factory().createObject(staticMetaObject.className(), parent, scheme, nonNull));

  if (videoFormatConverter)
  if (!videoFormatConverter->openFormat(srcFormat, dstFormat))
  {
    delete videoFormatConverter;
    videoFormatConverter = NULL;
  }

  if (nonNull && (videoFormatConverter == NULL))
    qFatal("Failed to open video format converter for \"%s\".", scheme.toAscii().data());

  return videoFormatConverter;
}

} } // End of namespaces
