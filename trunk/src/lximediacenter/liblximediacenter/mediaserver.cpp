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

#include "mediaserver.h"
#include "globalsettings.h"
#include "htmlparser.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {


struct MediaServer::Private
{
  class StreamEvent : public QEvent
  {
  public:
    inline                      StreamEvent(QEvent::Type type, const HttpServer::RequestHeader &request, QAbstractSocket *socket, QSemaphore *sem)
        : QEvent(type), request(request), socket(socket), sem(sem) { }

    const HttpServer::RequestHeader request;
    QAbstractSocket     * const socket;
    QSemaphore          * const sem;
  };

  inline                        Private(void) : mutex(QMutex::Recursive)        { }

  static const QEvent::Type     startStreamEventType;
  static const QEvent::Type     buildPlaylistEventType;
  static const int              maxStreams = 64;

  QMutex                        mutex;
  QList<Stream *>               streams;
  QList<Stream *>               reusableStreams;
};

const QEvent::Type MediaServer::Private::startStreamEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type MediaServer::Private::buildPlaylistEventType = QEvent::Type(QEvent::registerEventType());

const qint32  MediaServer::defaultDirSortOrder  = -65536;
const qint32  MediaServer::defaultFileSortOrder = 0;
const int     MediaServer::seekBySecs = 120;


MediaServer::MediaServer(const char *name, Plugin *plugin, BackendServer::MasterServer *server)
  : BackendServer(name, plugin, server),
    p(new Private())
{
  // Ensure the logo is loaded.
  GlobalSettings::productLogo().bits();

  masterServer()->httpServer()->registerCallback(httpPath(), this);
  masterServer()->dlnaServer()->registerCallback(dlnaPath(), this);
}

MediaServer::~MediaServer()
{
  masterServer()->httpServer()->unregisterCallback(this);
  masterServer()->dlnaServer()->unregisterCallback(this);

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void MediaServer::customEvent(QEvent *e)
{
  if (e->type() == p->startStreamEventType)
  {
    SDebug::MutexLocker l(&p->mutex, __FILE__, __LINE__);

    Private::StreamEvent * const event = static_cast<Private::StreamEvent *>(e);
    const QHostAddress peer = event->socket->peerAddress();
    const QString url = event->request.path();

    foreach (Stream *stream, p->streams)
    if ((stream->peer == peer) && (stream->url == url))
    if (stream->output.addSocket(event->socket))
    {
      event->sem->release();
      return;
    }

    if (streamVideo(event->request, event->socket) == HttpServer::SocketOp_Close)
    {
      event->socket->disconnectFromHost();
      if (event->socket->state() != QAbstractSocket::UnconnectedState)
      if (!event->socket->waitForDisconnected(BackendServer::maxRequestTime))
        event->socket->abort();

      delete event->socket;
    }

    event->sem->release();
  }
  else if (e->type() == p->buildPlaylistEventType)
  {
    Private::StreamEvent * const event = static_cast<Private::StreamEvent *>(e);
    if (buildPlaylist(event->request, event->socket) == HttpServer::SocketOp_Close)
    {
      event->socket->disconnectFromHost();
      if (event->socket->state() != QAbstractSocket::UnconnectedState)
      if (!event->socket->waitForDisconnected(BackendServer::maxRequestTime))
        event->socket->abort();

      delete event->socket;
    }

    event->sem->release();
  }
}

void MediaServer::cleanStreams(void)
{
  SDebug::MutexLocker l(&p->mutex, __FILE__, __LINE__);

  QList<Stream *> obsolete;
  foreach (Stream *stream, p->streams)
  if (!stream->isRunning() || !stream->output.isConnected())
    obsolete += stream;

  foreach (Stream *stream, obsolete)
  {
    stream->stop();
    delete stream;
  }
}

HttpServer::SocketOp MediaServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.isEmpty())
  {
    HttpServer::ResponseHeader response(HttpServer::Status_Ok);
    response.setContentType("text/html;charset=utf-8");
    response.setField("Cache-Control", "no-cache");

    QString path = url.path().mid(httpPath().length());
    path = path.startsWith('/') ? path : ('/' + path);

    const int total = countItems(path);
    const int start = url.queryItemValue("start").toInt();

    ThumbnailListItemList thumbItems;

    foreach (const DlnaServer::Item &item, listItems(path, start, itemsPerThumbnailPage))
    {
      if (item.isDir)
      {
        ThumbnailListItem thumbItem;
        thumbItem.title = item.title;
        thumbItem.iconurl = item.iconUrl;
        thumbItem.url = item.title + '/';

        thumbItems.append(thumbItem);
      }
      else
      {
        ThumbnailListItem thumbItem;
        thumbItem.title = item.title;
        thumbItem.iconurl = item.iconUrl;
        thumbItem.url = item.url;
        thumbItem.url.setPath(thumbItem.url.path() + ".html");
        thumbItem.played = item.played;

        thumbItems.append(thumbItem);
      }
    }

    return sendHtmlContent(socket, url, response, buildThumbnailView(path, thumbItems, start, total), headList);
  }
  else
  {
    const QString mime = HttpServer::toMimeType(url.path());
    if (mime.endsWith("/x-mpegurl"))
    {
      QSemaphore sem(0);

      socket->moveToThread(QObject::thread());
      QCoreApplication::postEvent(this, new Private::StreamEvent(Private::buildPlaylistEventType, request, socket, &sem));

      sem.acquire();
      return HttpServer::SocketOp_LeaveOpen; // Socket will be closed by event handler
    }
    else if (mime.startsWith("audio/") || mime.startsWith("video/"))
    {
      QSemaphore sem(0);

      socket->moveToThread(QObject::thread());
      QCoreApplication::postEvent(this, new Private::StreamEvent(Private::startStreamEventType, request, socket, &sem));

      sem.acquire();
      return HttpServer::SocketOp_LeaveOpen; // Socket will be closed by event handler
    }
  }

  qWarning() << "MediaServer: Failed to find:" << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

int MediaServer::countDlnaItems(const QString &path)
{
  QString subPath = path.mid(dlnaPath().length());
  subPath = subPath.startsWith('/') ? subPath : ('/' + subPath);

  return countItems(subPath);
}

QList<DlnaServer::Item> MediaServer::listDlnaItems(const QString &path, unsigned start, unsigned count)
{
  QString subPath = path.mid(dlnaPath().length());
  subPath = subPath.startsWith('/') ? subPath : ('/' + subPath);

  QList<DlnaServer::Item> result;
  foreach (Item item, listItems(subPath, start, count))
  {
    item.url.setPath(httpPath() + item.url.path());
    item.iconUrl.setPath(httpPath() + item.iconUrl.path());
    result += item;
  }

  return result;
}

void MediaServer::addStream(Stream *stream)
{
  SDebug::MutexLocker l(&p->mutex, __FILE__, __LINE__);

  connect(stream, SIGNAL(finished()), SLOT(cleanStreams()), Qt::QueuedConnection);
  connect(&(stream->output), SIGNAL(disconnected()), SLOT(cleanStreams()), Qt::QueuedConnection);

  p->streams += stream;
}

void MediaServer::removeStream(Stream *stream)
{
  SDebug::MutexLocker l(&p->mutex, __FILE__, __LINE__);

  qDebug() << "Closed stream" << stream->id;

  p->streams.removeAll(stream);
  p->reusableStreams.removeAll(stream);
}

QAtomicInt MediaServer::Stream::idCounter = 1;

MediaServer::Stream::Stream(MediaServer *parent, const QHostAddress &peer, const QString &url)
  : SGraph(),
    id(idCounter.fetchAndAddRelaxed(1)),
    parent(parent),
    peer(peer),
    url(url),
    timeStampResampler(this),
    audioResampler(this, "linear"),
    deinterlacer(this),
    subpictureRenderer(this),
    letterboxDetectNode(this),
    videoResizer(this),
    videoBox(this),
    subtitleRenderer(this),
    sync(this),
    audioEncoder(this),
    videoEncoder(this),
    output(this)
{
  parent->addStream(this);

  // Audio
  connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audioResampler, SLOT(input(SAudioBuffer)));
  connect(&audioResampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&sync, SIGNAL(output(SAudioBuffer)), &audioEncoder, SLOT(input(SAudioBuffer)));
  connect(&audioEncoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  // Video
  connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &deinterlacer, SLOT(input(SVideoBuffer)));
  connect(&deinterlacer, SIGNAL(output(SVideoBuffer)), &subpictureRenderer, SLOT(input(SVideoBuffer)));
  connect(&subpictureRenderer, SIGNAL(output(SVideoBuffer)), &letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &videoResizer, SLOT(input(SVideoBuffer)));
  connect(&videoResizer, SIGNAL(output(SVideoBuffer)), &videoBox, SLOT(input(SVideoBuffer)));
  connect(&videoBox, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
  connect(&subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
  connect(&sync, SIGNAL(output(SVideoBuffer)), &videoEncoder, SLOT(input(SVideoBuffer)));
  connect(&videoEncoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));

  // Data
  connect(&timeStampResampler, SIGNAL(output(SSubpictureBuffer)), &subpictureRenderer, SLOT(input(SSubpictureBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SSubtitleBuffer)), &subtitleRenderer, SLOT(input(SSubtitleBuffer)));
}

MediaServer::Stream::~Stream()
{
  parent->removeStream(this);
}

bool MediaServer::Stream::setup(const HttpServer::RequestHeader &request, QAbstractSocket *socket, STime duration, SInterval frameRate, SSize size, SAudioFormat::Channels channels)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  QStringList file = request.file().split('.');
  if (file.count() <= 1)
    file += "mpeg"; // Default to mpeg file

  // Stream priority
  if (url.queryItemValue("priority") == "lowest")
    setPriority(Priority_Lowest);
  else if (url.queryItemValue("priority") == "low")
    setPriority(Priority_Low);
  else if (url.queryItemValue("priority") == "high")
    setPriority(Priority_High);
  else if (url.queryItemValue("priority") == "highest")
    setPriority(Priority_Highest);

  // Set stream properties
  const double freq = frameRate.toFrequency();
  const double d15 = freq - 15.0;
  const double d24 = freq - 24.0;
  const double d25 = freq - 25.0;
  const double d30 = freq - 30.0;
  const double d50 = freq - 50.0;
  const double d60 = freq - 60.0;

  if ((d15 > -3.0) && (d15 < 3.0))
  {
    duration *= (15.0 / freq);
    frameRate = SInterval::fromFrequency(15);
  }
  else if ((d24 > -2.0) && (d24 < 0.6))
  {
    duration *= (24.0 / freq);
    frameRate = SInterval::fromFrequency(24);
  }
  else if ((d25 > -2.0) && (d25 < 2.1))
  {
    duration *= (25.0 / freq);
    frameRate = SInterval::fromFrequency(25);
  }
  else if ((d30 > -3.0) && (d30 < 4.0))
  {
    duration *= (30.0 / freq);
    frameRate = SInterval::fromFrequency(30);
  }
  else if ((d50 > -5.0) && (d50 < 5.0))
  {
    duration *= (50.0 / freq);
    frameRate = SInterval::fromFrequency(50);
  }
  else if ((d60 > -5.0) && (d60 < 5.0))
  {
    duration *= (60.0 / freq);
    frameRate = SInterval::fromFrequency(60);
  }
  else
    frameRate = SInterval::fromFrequency(25);

  timeStampResampler.setFrameRate(frameRate);
  sync.setFrameRate(frameRate);

  Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
  if (url.hasQueryItem("size"))
  {
    const QStringList formatTxt = url.queryItemValue("size").split('/');

    const QStringList sizeTxt = formatTxt.first().split('x');
    if (sizeTxt.count() >= 2)
    {
      size.setWidth(sizeTxt[0].toInt());
      size.setHeight(sizeTxt[1].toInt());
      if (sizeTxt.count() >= 3)
        size.setAspectRatio(sizeTxt[2].toFloat());
      else
        size.setAspectRatio(1.0f);
    }

    if (formatTxt.count() >= 2)
    {
      if (formatTxt[1] == "box")
        aspectRatioMode = Qt::KeepAspectRatio;
      else if (formatTxt[1] == "zoom")
        aspectRatioMode = Qt::KeepAspectRatioByExpanding;
    }
  }

  if (url.hasQueryItem("requestchannels"))
  {
    const SAudioFormat::Channels c =
        SAudioFormat::Channels(url.queryItemValue("requestchannels").toUInt(NULL, 16));

    if ((SAudioFormat::numChannels(c) > 0) &&
        (SAudioFormat::numChannels(channels) > SAudioFormat::numChannels(c)))
    {
      channels = c;
    }
  }
  else if (url.hasQueryItem("forcechannels"))
    channels = SAudioFormat::Channels(url.queryItemValue("forcechannels").toUInt(NULL, 16));

  SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_Fast;
  SInterfaces::VideoEncoder::Flags videoEncodeFlags = SInterfaces::VideoEncoder::Flag_Fast;
  if (url.queryItemValue("encode") == "slow")
  {
    audioEncodeFlags = SInterfaces::AudioEncoder::Flag_None;
    videoEncodeFlags = SInterfaces::VideoEncoder::Flag_None;
    videoResizer.setHighQuality(true);
  }
  else
    videoResizer.setHighQuality(false);

  HttpServer::ResponseHeader header(HttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");

  if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg"))
  {
    if (SAudioFormat::numChannels(channels) <= 2)
    {
      const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else if (SAudioFormat::numChannels(channels) == 4)
    {
      const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channel_Quadraphonic, 48000);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else
    {
      const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channel_Surround_5_1, 48000);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }


    if (!videoEncoder.openCodec(SVideoCodec("MPEG2", size, frameRate), videoEncodeFlags))
    if (!videoEncoder.openCodec(SVideoCodec("MPEG1", size, frameRate), videoEncodeFlags))
      return false;

    output.openFormat("vob", audioEncoder.codec(), videoEncoder.codec(), duration);
    header.setContentType("video/MP2P");
  }
  /*else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "ogv"))
  {
    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channel_Stereo, 44100);
    audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("THEORA", size, frameRate);
    if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audioEncoder.codec(), videoEncoder.codec(), duration);
    header.setContentType("video/ogg");
  }*/
  else if (file.last().toLower() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("FLV1", size, frameRate);
    if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audioEncoder.codec(), videoEncoder.codec(), duration);
    header.setContentType("video/x-flv");
  }
  else
    return false;

  videoResizer.setSize(size);
  videoResizer.setAspectRatioMode(aspectRatioMode);
  videoBox.setSize(size);

  output.setHeader(header);
  output.addSocket(socket);

  //enableTrace("/tmp/test.svg");

  qDebug() << "Started video stream" << id << ":"
      << size.width() << "x" << size.height()
      << "@" << frameRate.toFrequency() << videoEncoder.codec().codec()
      << SAudioFormat::channelSetupName(audioEncoder.codec().channelSetup())
      << audioEncoder.codec().sampleRate() << audioEncoder.codec().codec();

  return true;
}

bool MediaServer::Stream::setup(const HttpServer::RequestHeader &request, QAbstractSocket *socket, STime duration, SAudioFormat::Channels channels)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  QStringList file = request.file().split('.');
  if (file.count() <= 1)
    file += "mpa"; // Default to mpeg file

  // Stream priority
  if (url.queryItemValue("priority") == "lowest")
    setPriority(Priority_Lowest);
  else if (url.queryItemValue("priority") == "low")
    setPriority(Priority_Low);
  else if (url.queryItemValue("priority") == "high")
    setPriority(Priority_High);
  else if (url.queryItemValue("priority") == "highest")
    setPriority(Priority_Highest);

  // Set stream properties
  if (url.hasQueryItem("requestchannels"))
  {
    const SAudioFormat::Channels c =
        SAudioFormat::Channels(url.queryItemValue("requestchannels").toUInt(NULL, 16));

    if ((SAudioFormat::numChannels(c) > 0) &&
        (SAudioFormat::numChannels(channels) > SAudioFormat::numChannels(c)))
    {
      channels = c;
    }
  }
  else if (url.hasQueryItem("forcechannels"))
    channels = SAudioFormat::Channels(url.queryItemValue("forcechannels").toUInt(NULL, 16));

  SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_Fast;
  if (url.queryItemValue("encode") == "slow")
    audioEncodeFlags = SInterfaces::AudioEncoder::Flag_None;

  HttpServer::ResponseHeader header(HttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");

  if (file.last().toLower() == "mpa")
  {
    header.setContentType("audio/mpeg");

    const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.openFormat("mp2", audioEncoder.codec(), SVideoCodec(), duration);
  }
  /*else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "oga"))
  {
    header.setContentType("audio/ogg");

    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channel_Stereo, 44100);
    audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audioEncoder.codec(), SVideoCodec(), duration);
  }*/
  else if (file.last().toLower() == "wav")
  {
    header.setContentType("audio/wave");

    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("wav", audioEncoder.codec(), SVideoCodec(), duration);
  }
  else if (file.last().toLower() == "flv")
  {
    header.setContentType("video/x-flv");

    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audioEncoder.codec(), SVideoCodec(), duration);
  }
  else
    return false;

  output.setHeader(header);
  output.addSocket(socket);

  //enableTrace("/tmp/test.svg");

  qDebug() << "Started audio stream" << id << ":"
      << SAudioFormat::channelSetupName(audioEncoder.codec().channelSetup())
      << audioEncoder.codec().sampleRate() << audioEncoder.codec().codec();

  return true;
}


MediaServer::TranscodeStream::TranscodeStream(MediaServer *parent, const QHostAddress &peer, const QString &url)
  : Stream(parent, peer, url),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this)
{
  // Audio
  connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));

  // Video
  connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));

  // Data
  connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), &timeStampResampler, SLOT(input(SSubpictureBuffer)));
  connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &timeStampResampler, SLOT(input(SSubtitleBuffer)));
}

bool MediaServer::TranscodeStream::setup(const HttpServer::RequestHeader &request, QAbstractSocket *socket, SInterfaces::BufferReaderNode *input, STime duration)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  // Select streams
  const QList<SIOInputNode::AudioStreamInfo> audioStreams = input->audioStreams();
  const QList<SIOInputNode::VideoStreamInfo> videoStreams = input->videoStreams();
  const QList<SIOInputNode::DataStreamInfo>  dataStreams  = input->dataStreams();

  QList<SIOInputNode::StreamId> selectedStreams;
  if (url.hasQueryItem("language"))
    selectedStreams += url.queryItemValue("language").toUInt(NULL, 16);
  else if (!audioStreams.isEmpty())
    selectedStreams += audioStreams.first();

  if (!videoStreams.isEmpty())
    selectedStreams += videoStreams.first();

  if (url.hasQueryItem("subtitles"))
  {
    if (!url.queryItemValue("subtitles").isEmpty())
      selectedStreams += url.queryItemValue("subtitles").toUInt(NULL, 16);
  }
  else if (!dataStreams.isEmpty())
    selectedStreams += dataStreams.first();

  input->selectStreams(selectedStreams);

  // Seek to start
  if (!duration.isValid())
    duration = input->duration();

  if (url.hasQueryItem("position"))
  {
    const STime pos = STime::fromSec(url.queryItemValue("position").toInt());
    input->setPosition(pos);

    if (duration > pos)
      duration -= pos;
    else
      duration = STime::null;
  }

  // Set stream properties
  if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;
    const SVideoCodec videoInCodec = videoStreams.first().codec;

    if (Stream::setup(request, socket,
                      duration,
                      videoInCodec.frameRate(), videoInCodec.size(),
                      audioInCodec.channelSetup()))
    {
      if (audioResampler.channels() == SAudioFormat::Channel_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      return true;
    }
  }
  else if (!audioStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;

    if (Stream::setup(request, socket,
                      duration,
                      audioInCodec.channelSetup()))
    {
      if (audioResampler.channels() == SAudioFormat::Channel_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      return true;
    }
  }

  return false;
}

} // End of namespace
