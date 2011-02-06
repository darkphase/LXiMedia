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
    inline                      StreamEvent(QEvent::Type type, const QHttpRequestHeader &request, QAbstractSocket *socket, QSemaphore *sem)
        : QEvent(type), request(request), socket(socket), sem(sem) { }

    const QHttpRequestHeader    request;
    QAbstractSocket     * const socket;
    QSemaphore          * const sem;
  };

  static const QEvent::Type     startStreamEventType;
  static const QEvent::Type     buildPlaylistEventType;
  static const int              maxStreams = 64;

  QList<Stream *>               streams;
  QList<Stream *>               reusableStreams;

  HttpServerDir               * httpDir;
  DlnaServerDir               * dlnaDir;
};

const QEvent::Type MediaServer::Private::startStreamEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type MediaServer::Private::buildPlaylistEventType = QEvent::Type(QEvent::registerEventType());

const qint32  MediaServer::defaultDirSortOrder  = -65536;
const qint32  MediaServer::defaultFileSortOrder = 0;
const int     MediaServer::seekBySecs = 120;


MediaServer::MediaServer(const char *name, Plugin *plugin, BackendServer::MasterServer *server)
  : BackendServer(name, plugin, server),
    FileServer(),
    p(new Private())
{
  p->httpDir = NULL;
  p->dlnaDir = NULL;

  // Ensure the logo is loaded.
  GlobalSettings::productLogo().bits();
}

MediaServer::~MediaServer()
{
  delete p->httpDir;
  delete p->dlnaDir;
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void MediaServer::setRoot(MediaServerDir *root)
{
  FileServer::setRoot(root);

  HttpServer * const httpServer = masterServer()->httpServer();

  SDebug::WriteLocker hl(&httpServer->lock, __FILE__, __LINE__);
  httpServer->addDir(httpPath(), p->httpDir = new HttpDir(httpServer, this, "/"));
  hl.unlock();

  DlnaServer * const dlnaServer = masterServer()->dlnaServer();

  SDebug::WriteLocker dl(&dlnaServer->lock, __FILE__, __LINE__);
  dlnaServer->addDir(dlnaPath(), p->dlnaDir = new DlnaDir(dlnaServer, this, "/"));
  dl.unlock();
}

bool MediaServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (file.isEmpty())
  {
    QString path = url.path().mid(httpPath().length());
    path = path.startsWith('/') ? path : ('/' + path);

    return buildDir(url, path, socket);
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
      return true; // Socket will be closed by event handler
    }
    else if (mime.startsWith("audio/") || mime.startsWith("video/"))
    {
      QSemaphore sem(0);

      socket->moveToThread(QObject::thread());
      QCoreApplication::postEvent(this, new Private::StreamEvent(Private::startStreamEventType, request, socket, &sem));

      sem.acquire();
      return true; // Socket will be closed by event handler
    }
  }

  qWarning() << "MediaServer: Failed to find:" << request.path();
  socket->write(QHttpResponseHeader(404).toString().toUtf8());
  return false;
}

bool MediaServer::buildDir(const QUrl &url, const QString &path, QAbstractSocket *socket)
{
  MediaServerConstDirHandle dir = findDir(path);
  if (dir != NULL)
  {
    QHttpResponseHeader response(200);
    response.setContentType("text/html;charset=utf-8");
    response.setValue("Cache-Control", "no-cache");

    ThumbnailListItemMap items;

    foreach (const QString &subDirName, dir->listDirs())
    {
      MediaServerConstDirHandle subDir = dir->findDir(subDirName);
      if (subDir != NULL)
      {
        ThumbnailListItem item;
        item.title = subDirName;
        item.subtitle = QString::number(subDir->count()) + " " + tr("items");
        item.iconurl = subDir->getIcon();
        item.url = subDirName + '/';

        items.insert(("00000000" + QString::number(quint32(subDir->sortOrder + 0x80000000), 16)).right(8) + item.title.toUpper(), item);
      }
    }

    foreach (const QString &fileName, dir->listFiles())
    {
      const MediaServerDir::File file = dir->findFile(fileName);
      if (!file.isNull())
      {
        ThumbnailListItem item;
        item.title = fileName;
        item.iconurl = file.iconUrl;
        item.url = file.url.left(file.url.lastIndexOf('.')) + ".html";
        item.played = file.played;

        items.insert(("00000000" + QString::number(quint32(file.sortOrder + 0x80000000), 16)).right(8) + item.title.toUpper(), item);
      }
    }

    return sendHtmlContent(socket, url, response, buildThumbnailView("", items, url), headList);
  }

  qWarning() << "MediaServer: Failed to find:" << url.toString();
  socket->write(QHttpResponseHeader(404).toString().toUtf8());
  return false;
}

void MediaServer::customEvent(QEvent *e)
{
  if (e->type() == p->startStreamEventType)
  {
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

    if (!streamVideo(event->request, event->socket))
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
    if (!buildPlaylist(event->request, event->socket))
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
  lock.lockForWrite();

  QList<Stream *> obsolete;
  foreach (Stream *stream, p->streams)
  if (!stream->isRunning() || !stream->output.isConnected())
    obsolete += stream;

  foreach (Stream *stream, obsolete)
  {
    stream->stop();
    delete stream;
  }

  lock.unlock();
}

void MediaServer::addStream(Stream *stream)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  connect(stream, SIGNAL(finished()), SLOT(cleanStreams()), Qt::QueuedConnection);
  connect(&(stream->output), SIGNAL(disconnected()), SLOT(cleanStreams()), Qt::QueuedConnection);

  p->streams += stream;
}

void MediaServer::removeStream(Stream *stream)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

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

void MediaServer::Stream::setup(bool addHeader, const QString &name, const QImage &thumb)
{
  const SAudioCodec audioCodec = audioEncoder.codec();
  const SVideoCodec videoCodec = videoEncoder.codec();

  if (!videoCodec.isNull())
    sync.setFrameRate(videoCodec.frameRate());

  // Build test card
  if (addHeader && !audioCodec.isNull() && !videoCodec.isNull())
  {
    SAudioBuffer audioBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                          audioCodec.channelSetup(),
                                          audioCodec.sampleRate()),
                             int(audioCodec.sampleRate() / videoCodec.frameRate().toFrequency()));
    memset(audioBuffer.data(), 0, audioBuffer.size());

    QImage darkThumb;
    if (!thumb.isNull())
    {
      darkThumb = thumb;
      QPainter p;
      p.begin(&darkThumb);
        p.fillRect(darkThumb.rect(), QColor(0, 0, 0, 192));
      p.end();
    }

    SImage img(videoCodec.size().size(), QImage::Format_RGB32);
    QPainter p;
    p.begin(&img);
      p.fillRect(img.rect(), Qt::black);

      const qreal ar = videoCodec.size().aspectRatio();

      if (!darkThumb.isNull())
      {
        QSize size = darkThumb.size();
        size.scale(int(img.width() * ar), img.height(), Qt::KeepAspectRatio);
        darkThumb = darkThumb.scaled(int(size.width() / ar), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        const QPoint pos((img.width() / 2) - (darkThumb.width() / 2), (img.height() / 2) - (darkThumb.height() / 2));
        p.drawImage(pos, darkThumb);
      }

      const int bsv = qMin(img.width() / 16, img.height() / 16), bsvi = bsv / 8;
      const int bsh = int(bsv / ar), bshi = int(bsvi / ar);

      const QColor bc(255, 255, 255, 64);
      p.fillRect(0, 0, bsh * 4, bsv * 2, bc);
      p.fillRect(0, 0, bsh * 2, bsv * 4, bc);
      p.fillRect(img.width(), img.height(), -bsh * 4, -bsv * 2, bc);
      p.fillRect(img.width(), img.height(), -bsh * 2, -bsv * 4, bc);

      const int dv = qMin(img.width(), img.height()) - bsv, dh = int(dv / ar);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setPen(QPen(Qt::white, 4.0));
      p.setBrush(QColor(0, 16, 32, 160));
      p.drawEllipse(QRect((img.width() / 2) - (dh / 2), (img.height() / 2) - (dv / 2), dh, dv));
      p.setRenderHint(QPainter::Antialiasing, false);

      QImage logo = GlobalSettings::productLogo();
      if ((img.height() < 1080) || !qFuzzyCompare(ar, 1.0))
      {
        logo = logo.scaled(int((logo.width() * img.height() / 1080) / ar),
                           logo.height() * img.height() / 1080,
                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      }

      const QPoint pos((img.width() / 2) - (logo.width() / 2), (img.height() / 2) - (logo.height() / 2));
      p.drawImage(pos, logo);

      for (int i=0; i<8; i++)
      {
        const int c = qBound(0, 256 - (i * 32), 255);
        const int xPos = ((img.width() / 2) - (bsh * 4)) + (bsh * i);
        const int yPos = bsv * 4;
        p.fillRect(xPos, yPos, bsh, bsv, QColor(c, c, c));
      }
    p.end();

    static const int duration = 4; // Seconds
    SVideoBufferList buffers;
    for (int i=0; i<duration; i++)
    {
      p.begin(&img);
        const int xPos = ((img.width() / 2) + (bsh * (duration / 2))) - (bsh * i) - bshi;
        const int yPos = img.height() - (bsv * 4) - bsvi;
        p.fillRect(xPos, yPos, -(bsh - (bshi * 2)), -(bsv - (bsvi * 2)), Qt::white);
      p.end();

      // The subtitle renderer is used here to render the text as on Unix fonts
      // are not available if X is not loaded.
      QStringList text;
      text += name;
      text += QString::null;

      buffers.prepend(SSubtitleRenderNode::renderSubtitles(img.toVideoBuffer(ar, videoCodec.frameRate()), text));
    }

    sync.setHeader(audioBuffer, buffers, STime::fromSec(buffers.count()));
  }

  //enableTrace("/tmp/test.svg");

  qDebug() << "Started stream" << id << ":"
      << videoCodec.size().width() << "x" << videoCodec.size().height()
      << "@" << videoCodec.frameRate().toFrequency() << videoEncoder.codec().codec()
      << SAudioFormat::channelSetupName(audioEncoder.codec().channelSetup())
      << audioEncoder.codec().sampleRate() << audioEncoder.codec().codec();
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

bool MediaServer::TranscodeStream::setup(const QHttpRequestHeader &request, QAbstractSocket *socket, SInterfaces::BufferReaderNode *input, STime duration, const QString &name, const QImage &thumb)
{
  QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  if (url.queryItemValue("priority") == "lowest")
    setPriority(Priority_Lowest);
  else if (url.queryItemValue("priority") == "low")
    setPriority(Priority_Low);
  else if (url.queryItemValue("priority") == "high")
    setPriority(Priority_High);
  else if (url.queryItemValue("priority") == "highest")
    setPriority(Priority_Highest);

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

  // Set stream properties
  if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;
    const SVideoCodec videoInCodec = videoStreams.first().codec;

    if (url.hasQueryItem("position"))
    {
      const STime pos = STime::fromSec(url.queryItemValue("position").toInt());
      input->setPosition(pos);

      if (duration > pos)
        duration -= pos;
      else
        duration = STime::null;
    }

    // Graph options.
    SInterval frameRate = videoInCodec.frameRate();
    if (frameRate.isValid())
    {
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
    }
    else
      frameRate = SInterval::fromFrequency(25);

    SSize size = videoInCodec.size();
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

    SAudioFormat::Channels channels = audioInCodec.channelSetup();
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

    QHttpResponseHeader header(200);
    header.setValue("Cache-Control", "no-cache");

    if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg"))
    {
      header.setContentType("video/MP2P");

      if (SAudioFormat::numChannels(channels) <= 2)
      {
        const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
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
    }
    else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "ogv"))
    {
      header.setContentType("video/ogg");

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
    }
    else if (file.last().toLower() == "flv")
    {
      header.setContentType("video/x-flv");

      const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
      audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;

      const SVideoCodec videoOutCodec("FLV1", size, frameRate);
      if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
        return false;

      output.enablePseudoStreaming(1.1f);
      output.openFormat("flv", audioEncoder.codec(), videoEncoder.codec(), duration);
    }
    else
      return false;

    videoResizer.setSize(size);
    videoResizer.setAspectRatioMode(aspectRatioMode);
    videoBox.setSize(size);

    Stream::setup(url.queryItemValue("header") == "true", name, thumb);

    output.setHeader(header.toString().toUtf8());
    output.addSocket(socket);

    return true;
  }
  else if (!audioStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;

    if (url.hasQueryItem("position"))
    {
      const STime pos = STime::fromSec(url.queryItemValue("position").toInt());
      input->setPosition(pos);

      if (duration > pos)
        duration -= pos;
      else
        duration = STime::null;
    }

    // Graph options.
    SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_Fast;
    if (url.hasQueryItem("encode") && (url.queryItemValue("encode") == "slow"))
      audioEncodeFlags = SInterfaces::AudioEncoder::Flag_None;

    SAudioFormat::Channels channels = audioInCodec.channelSetup();
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

    QHttpResponseHeader header(200);
    header.setValue("Cache-Control", "no-cache");

    if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg"))
    {
      header.setContentType("video/MP2P");

      if (SAudioFormat::numChannels(channels) <= 2)
      {
        const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
        audioResampler.setChannels(audioOutCodec.channelSetup());
        audioResampler.setSampleRate(audioOutCodec.sampleRate());
        audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);
      }
      else if (SAudioFormat::numChannels(channels) == 4)
      {
        const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channel_Quadraphonic, 48000);
        audioResampler.setChannels(audioOutCodec.channelSetup());
        audioResampler.setSampleRate(audioOutCodec.sampleRate());
        audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);
      }
      else
      {
        const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channel_Surround_5_1, 48000);
        audioResampler.setChannels(audioOutCodec.channelSetup());
        audioResampler.setSampleRate(audioOutCodec.sampleRate());
        audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);
      }

      output.openFormat("vob", audioEncoder.codec(), SVideoCodec(), duration);
    }
    else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "ogv"))
    {
      header.setContentType("video/ogg");

      const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channel_Stereo, 44100);
      audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

      output.enablePseudoStreaming(1.1f);
      output.openFormat("ogg", audioEncoder.codec(), SVideoCodec(), duration);
    }
    else if (file.last().toLower() == "flv")
    {
      header.setContentType("video/x-flv");

      const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
      audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

      output.enablePseudoStreaming(1.1f);
      output.openFormat("flv", audioEncoder.codec(), SVideoCodec(), duration);
    }
    else
      return false;

    Stream::setup(false, name);

    output.setHeader(header.toString().toUtf8());
    output.addSocket(socket);

    return true;
  }

  return false;
}


MediaServer::HttpDir::HttpDir(HttpServer *parent, MediaServer *mediaServer, const QString &mediaPath)
  : HttpServerDir(parent),
    mediaServer(mediaServer),
    mediaPath(mediaPath.endsWith('/') ? mediaPath : (mediaPath + '/'))
{
}

QStringList MediaServer::HttpDir::listDirs(void)
{
  MediaServerDirHandle mediaDir = mediaServer->findDir(mediaPath);
  if (mediaDir != NULL)
  {
    QSet<QString> dirs = QSet<QString>::fromList(mediaDir->listDirs());
    QStringList addDirs = dirs.toList();
    QStringList delDirs;

    foreach (const QString &dirName, HttpServerDir::listDirs())
    {
      if (!dirs.contains(dirName))
        delDirs.append(dirName);
      else
        addDirs.removeAll(dirName);
    }

    foreach (const QString &dirName, delDirs)
      removeDir(dirName);

    foreach (const QString &dirName, addDirs)
    {
      addDir(dirName, new HttpDir(
          server(), mediaServer,
          (mediaPath.endsWith('/') ? mediaPath : (mediaPath + '/')) + dirName));
    }
  }

  return HttpServerDir::listDirs();
}

bool MediaServer::HttpDir::handleConnection(const QHttpRequestHeader &header, QAbstractSocket *socket)
{
  return mediaServer->handleConnection(header, socket);
}


MediaServer::DlnaDir::DlnaDir(DlnaServer *parent, MediaServer *mediaServer, const QString &mediaPath)
  : DlnaServerDir(parent),
    mediaServer(mediaServer),
    mediaPath(mediaPath.endsWith('/') ? mediaPath : (mediaPath + '/')),
    plainFiles()
{
  MediaServerDirHandle mediaDir = mediaServer->findDir(mediaPath);
  if (mediaDir != NULL)
    sortOrder = mediaDir->sortOrder;
}

QStringList MediaServer::DlnaDir::listDirs(void)
{
  MediaServerDirHandle mediaDir = mediaServer->findDir(mediaPath);
  if (mediaDir != NULL)
  {
    plainFiles.clear();

    QSet<QString> dirs;
    foreach (const QString &dir, mediaDir->listDirs())
      dirs.insert('+' + dir);

    QSet<QString> files = QSet<QString>::fromList(mediaDir->listFiles());

    QStringList addDirs = dirs.toList();
    QStringList addFiles = files.toList();
    QStringList delDirs;

    foreach (const QString &dirName, DlnaServerDir::listDirs())
    {
      if (!dirs.contains(dirName) && !files.contains(dirName))
      {
        delDirs.append(dirName);
      }
      else
      {
        addDirs.removeAll(dirName);
        addFiles.removeAll(dirName);
      }
    }

    foreach (const QString &dirName, delDirs)
      removeDir(dirName);

    foreach (const QString &dirName, addDirs)
    {
      addDir(dirName, new DlnaDir(
          server(), mediaServer,
          (mediaPath.endsWith('/') ? mediaPath : (mediaPath + '/')) + dirName.mid(1)));
    }

    foreach (const QString &fileName, addFiles)
    {
      const MediaServerDir::File srcFile = mediaDir->findFile(fileName);
      if (!srcFile.isNull())
      {
        if ((srcFile.mimeType == "video/mpeg") && !srcFile.mediaInfo.isNull() &&
            (srcFile.mediaInfo.duration().toMin() >= 10))
        {
          DlnaServerDir * const fileRootDir = new DlnaServerDir(server());
          fileRootDir->played = srcFile.played;
          fileRootDir->sortOrder = srcFile.sortOrder;

          const QList<SMediaInfo::AudioStreamInfo> audioStreams = srcFile.mediaInfo.audioStreams();
          const QList<SMediaInfo::DataStreamInfo> dataStreams =
              srcFile.mediaInfo.dataStreams() <<
              SIOInputNode::DataStreamInfo(SIOInputNode::DataStreamInfo::Type_Subtitle, 0xFFFF, NULL, SDataCodec());

          for (int a=0, an=audioStreams.count(); a < an; a++)
          for (int d=0, dn=dataStreams.count(); d < dn; d++)
          {
            DlnaServerDir * const fileDir = ((an + dn) == 2) ? fileRootDir : new DlnaServerDir(fileRootDir->server());
            DlnaServerDir * const chapterDir = new DlnaServerDir(fileDir->server());
            DlnaServerDir * const seekDir = new DlnaServerDir(fileDir->server());

            QString url = srcFile.url;
            url += "?language=" + QString::number(audioStreams[a], 16) + "&subtitles=";
            if (dataStreams[d].streamId() != 0xFFFF)
              url += QString::number(dataStreams[d], 16);

            File file(server());
            file.sortOrder = 1;
            file.mimeType = srcFile.mimeType;
            file.url = url;
            file.iconUrl = srcFile.iconUrl;
            fileDir->addFile(tr("Play"), file);

            int chapterNum = 1;
            foreach (const SMediaInfo::Chapter &chapter, srcFile.mediaInfo.chapters())
            {
              File file(chapterDir->server());
              file.sortOrder = chapterNum++;
              file.mimeType = srcFile.mimeType;
              file.url = url + (url.contains('?') ? "&position=" : "?position=") +
                          QString::number(chapter.begin.toSec());

              QString name = tr("Chapter") + " " + QString::number(file.sortOrder);
              if (!chapter.title.isEmpty())
                name += ", " + chapter.title;

              chapterDir->addFile(name, file);
            }

            for (int i=0, n=srcFile.mediaInfo.duration().toSec(); i<n; i+=seekBySecs)
            {
              File file(seekDir->server());
              file.sortOrder = i;
              file.mimeType = srcFile.mimeType;
              file.url = url + (url.contains('?') ? "&position=" : "?position=") + QString::number(i);

              seekDir->addFile(tr("Play from") + " " + QString::number(i / 3600) +
                               ":" + ("0" + QString::number((i / 60) % 60)).right(2),
                               file);
            }

            if (chapterDir->count() > 0)
            {
              chapterDir->sortOrder = 2;
              fileDir->addDir(tr("Chapters"), chapterDir);
            }
            else
              delete chapterDir;

            if (seekDir->count() > 0)
            {
              seekDir->sortOrder = 3;
              fileDir->addDir(tr("Seek"), seekDir);
            }
            else
              delete seekDir;

            if (fileDir != fileRootDir)
            {
              fileDir->sortOrder = (a + 1) * 2;
              QString name;
              if (an == 1) // One audio stream
              {
                if (dataStreams[d].streamId() != 0xFFFF)
                {
                  fileDir->sortOrder -= 1;
                  name += tr("With subtitles");
                  if (dataStreams[d].language[0])
                  {
                    name += " (";
                    if (dn > 2)
                      name += QString::number(d + 1) + ". ";

                    name += SStringParser::iso639Language(dataStreams[d].language) + ")";
                  }
                  else if (dn > 2)
                    name += " (" + QString::number(d + 1) + ")";
                }
                else
                  name += tr("Without subtitles");
              }
              else
              {
                name += QString::number(a + 1) + ". ";
                if (audioStreams[a].language[0])
                  name += SStringParser::iso639Language(audioStreams[a].language);
                else
                  name += tr("Unknown");

                if (dataStreams[d].streamId() != 0xFFFF)
                {
                  fileDir->sortOrder -= 1;
                  name += " " + tr("with subtitles");
                  if (dataStreams[d].language[0])
                  {
                    name += " (";
                    if (dn > 2)
                      name += QString::number(d + 1) + ". ";

                    name += SStringParser::iso639Language(dataStreams[d].language) + ")";
                  }
                  else if (dn > 2)
                    name += " (" + QString::number(d + 1) + ")";
                }
              }

              fileRootDir->addDir(name, fileDir);
            }
          }

          addDir(fileName, fileRootDir);
        }
        else
          plainFiles.append(fileName);
      }
    }
  }

  return DlnaServerDir::listDirs();
}

QStringList MediaServer::DlnaDir::listFiles(void)
{
  MediaServerDirHandle mediaDir = mediaServer->findDir(mediaPath);
  if (mediaDir != NULL)
  {
    QSet<QString> files = QSet<QString>::fromList(plainFiles);
    QStringList addFiles = files.toList();
    QStringList delFiles;

    foreach (const QString &fileName, DlnaServerDir::listFiles())
    {
      if (!files.contains(fileName))
        delFiles.append(fileName);
      else
        addFiles.removeAll(fileName);
    }

    foreach (const QString &fileName, delFiles)
      removeFile(fileName);

    foreach (const QString &fileName, addFiles)
    {
      const MediaServerDir::File srcFile = mediaDir->findFile(fileName);
      if (!srcFile.isNull())
      {
        File file(server());
        file.sortOrder = srcFile.sortOrder;
        file.mimeType = srcFile.mimeType;
        file.url = srcFile.url;
        file.iconUrl = srcFile.iconUrl;

        addFile(fileName, file);
      }
    }
  }

  return DlnaServerDir::listFiles();
}


MediaServerDir::MediaServerDir(MediaServer *parent)
  : FileServerDir(parent),
    sortOrder(MediaServer::defaultDirSortOrder)
{
}

void MediaServerDir::addFile(const QString &name, const File &file)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  files[name] = file;
}

void MediaServerDir::removeFile(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  files.remove(name);
}

void MediaServerDir::clear(void)
{
  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  files.clear();
  FileServerDir::clear();
}

int MediaServerDir::count(void) const
{
  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  return listFiles().count() + FileServerDir::count();
}

QStringList MediaServerDir::listFiles(void)
{
  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  return files.keys();
}

MediaServerDir::File MediaServerDir::findFile(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  QMap<QString, File>::ConstIterator i = files.find(name);
  if (i == files.end())
  {
    // Try to update the dir
    listFiles();
    i = files.find(name);
  }

  if (i != files.end())
    return *i;

  return File();
}

QString MediaServerDir::getIcon(void) const
{
  foreach (const QString &fileName, listFiles())
  {
    const File file = findFile(fileName);
    if (!file.iconUrl.isEmpty())
      return file.iconUrl;
  }

  foreach (const QString &dirName, listDirs())
  {
    MediaServerConstDirHandle dir = findDir(dirName);
    if (dir != NULL)
    {
      const QString iconUrl = dir->getIcon();
      if (!iconUrl.isEmpty())
        return iconUrl;
    }
  }

  return QString::null;
}

} // End of namespace
