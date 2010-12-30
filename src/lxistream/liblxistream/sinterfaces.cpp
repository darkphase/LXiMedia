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
#include "sfactory.hpp"
#include "sgraph.h"

namespace LXiStream {

// Instantiated SFactorizable templates for the interfaces:
template class SFactorizable<SInterfaces::FormatProber>;
template class SFactorizable<SInterfaces::BufferReader>;
template class SFactorizable<SInterfaces::BufferWriter>;
template class SFactorizable<SInterfaces::AudioDecoder>;
template class SFactorizable<SInterfaces::VideoDecoder>;
template class SFactorizable<SInterfaces::DataDecoder>;
template class SFactorizable<SInterfaces::AudioEncoder>;
template class SFactorizable<SInterfaces::VideoEncoder>;
template class SFactorizable<SInterfaces::AudioInput>;
template class SFactorizable<SInterfaces::AudioOutput>;
template class SFactorizable<SInterfaces::AudioResampler>;
template class SFactorizable<SInterfaces::VideoDeinterlacer>;
template class SFactorizable<SInterfaces::VideoInput>;
template class SFactorizable<SInterfaces::VideoResizer>;

namespace SInterfaces {


Module::~Module()
{
}

Node::Node(SGraph *graph)
  : graph(graph)
{
  if (graph)
    graph->addNode(this);
}

Node::~Node()
{
  *const_cast<SGraph **>(&graph) = NULL;
}

SinkNode::SinkNode(SGraph *graph)
  : graph(graph)
{
  if (graph)
    graph->addNode(this);
}

SinkNode::~SinkNode()
{
  *const_cast<SGraph **>(&graph) = NULL;
}

SourceNode::SourceNode(SGraph *graph)
  : graph(graph),
    mutex(QMutex::Recursive)
{
  if (graph)
    graph->addNode(this);
}

SourceNode::~SourceNode()
{
  *const_cast<SGraph **>(&graph) = NULL;
}

/*! Creates all registred format probers.
    \param parent   The parent object, or NULL if none.
 */
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
      SFactorizable<BufferReader>::create(parent, format, nonNull);

  if (bufferReader)
  if (!bufferReader->openFormat(format))
  {
    delete bufferReader;
    bufferReader = NULL;

    if (nonNull)
      qFatal("Failed to open input format \"%s\".", format.toAscii().data());
  }

  return bufferReader;
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
      SFactorizable<BufferWriter>::create(parent, format, nonNull);

  if (bufferWriter)
  if (!bufferWriter->openFormat(format))
  {
    delete bufferWriter;
    bufferWriter = NULL;

    if (nonNull)
      qFatal("Failed to open output format \"%s\".", format.toAscii().data());
  }

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
      SFactorizable<AudioDecoder>::create(parent, codec.codec(), nonNull);

  if (audioDecoder)
  if (!audioDecoder->openCodec(codec, flags))
  {
    delete audioDecoder;
    audioDecoder = NULL;

    if (nonNull)
      qFatal("Failed to open audio decoder for \"%s\".", codec.codec().toAscii().data());
  }

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
      SFactorizable<VideoDecoder>::create(parent, codec.codec(), nonNull);

  if (videoDecoder)
  if (!videoDecoder->openCodec(codec, flags))
  {
    delete videoDecoder;
    videoDecoder = NULL;

    if (nonNull)
      qFatal("Failed to open video decoder for \"%s\".", codec.codec().toAscii().data());
  }

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
      SFactorizable<DataDecoder>::create(parent, codec.codec(), nonNull);

  if (dataDecoder)
  if (!dataDecoder->openCodec(codec, flags))
  {
    delete dataDecoder;
    dataDecoder = NULL;

    if (nonNull)
      qFatal("Failed to open data decoder for \"%s\".", codec.codec().toAscii().data());
  }

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
      SFactorizable<AudioEncoder>::create(parent, codec.codec(), nonNull);

  if (audioEncoder)
  if (!audioEncoder->openCodec(codec, flags))
  {
    delete audioEncoder;
    audioEncoder = NULL;

    if (nonNull)
      qFatal("Failed to open audio encoder for \"%s\".", codec.codec().toAscii().data());
  }

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
      SFactorizable<VideoEncoder>::create(parent, codec.codec(), nonNull);

  if (videoEncoder)
  if (!videoEncoder->openCodec(codec, flags))
  {
    delete videoEncoder;
    videoEncoder = NULL;

    if (nonNull)
      qFatal("Failed to open video encoder for \"%s\".", codec.codec().toAscii().data());
  }

  return videoEncoder;
}


} } // End of namespaces
