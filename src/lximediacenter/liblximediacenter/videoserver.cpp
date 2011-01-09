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

#include <liblximediacenter/videoserver.h>
#include <liblximediacenter/globalsettings.h>
#include <liblximediacenter/htmlparser.h>
#include <LXiStreamGui>

namespace LXiMediaCenter {


struct VideoServer::Private
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
};

const QEvent::Type VideoServer::Private::startStreamEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type VideoServer::Private::buildPlaylistEventType = QEvent::Type(QEvent::registerEventType());


VideoServer::VideoServer(const char *name, Plugin *plugin, BackendServer::MasterServer *server)
            :BackendServer(name, plugin, server),
             lock(QReadWriteLock::Recursive),
             p(new Private())
{
  // Ensure the logo is loaded.
  GlobalSettings::productLogo().bits();
}

VideoServer::~VideoServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

bool VideoServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());

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

  return BackendServer::handleConnection(request, socket);
}

void VideoServer::customEvent(QEvent *e)
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

void VideoServer::cleanStreams(void)
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

void VideoServer::addStream(Stream *stream)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  connect(stream, SIGNAL(finished()), SLOT(cleanStreams()), Qt::QueuedConnection);
  connect(&(stream->output), SIGNAL(disconnected()), SLOT(cleanStreams()), Qt::QueuedConnection);

  p->streams += stream;
}

void VideoServer::removeStream(Stream *stream)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  qDebug() << "Closed stream" << stream->id;

  p->streams.removeAll(stream);
  p->reusableStreams.removeAll(stream);
}

QAtomicInt VideoServer::Stream::idCounter = 1;

VideoServer::Stream::Stream(VideoServer *parent, const QHostAddress &peer, const QString &url)
  : SGraph(),
    id(idCounter.fetchAndAddRelaxed(1)),
    parent(parent),
    peer(peer),
    url(url),
    subtitleRenderer(this),
    sync(this),
    audioEncoder(this),
    videoEncoder(this),
    output(this)
{
  parent->addStream(this);

  // Audio
  connect(&sync, SIGNAL(output(SAudioBuffer)), &audioEncoder, SLOT(input(SAudioBuffer)));
  connect(&audioEncoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  // Video
  connect(&subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
  connect(&sync, SIGNAL(output(SVideoBuffer)), &videoEncoder, SLOT(input(SVideoBuffer)));
  connect(&videoEncoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));
}

VideoServer::Stream::~Stream()
{
  parent->removeStream(this);
}

void VideoServer::Stream::setup(bool addHeader, const QString &name, const QImage &thumb)
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


VideoServer::TranscodeStream::TranscodeStream(VideoServer *parent, const QHostAddress &peer, const QString &url)
  : Stream(parent, peer, url),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this),
    timeStampResampler(this),
    audioResampler(this, "linear"),
    letterboxDetectNode(this),
    deinterlacer(this),
    videoResizer(this),
    videoBox(this)
{
  // Audio
  connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audioResampler, SLOT(input(SAudioBuffer)));
  connect(&audioResampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));

  // Video
  connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &deinterlacer, SLOT(input(SVideoBuffer)));
  connect(&deinterlacer, SIGNAL(output(SVideoBuffer)), &videoResizer, SLOT(input(SVideoBuffer)));
  connect(&videoResizer, SIGNAL(output(SVideoBuffer)), &videoBox, SLOT(input(SVideoBuffer)));
  connect(&videoBox, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));

  // Data
  connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &timeStampResampler, SLOT(input(SSubtitleBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SSubtitleBuffer)), &subtitleRenderer, SLOT(input(SSubtitleBuffer)));
}

bool VideoServer::TranscodeStream::setup(const QHttpRequestHeader &request, QAbstractSocket *socket, SIOInputNode *input, STime duration, const QString &name, const QImage &thumb)
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

  QList<quint16> selectedStreams;
  if (url.hasQueryItem("language"))
    selectedStreams += url.queryItemValue("language").toUShort(NULL, 16);
  else if (!audioStreams.isEmpty())
    selectedStreams += audioStreams.first().streamId;

  if (!videoStreams.isEmpty())
    selectedStreams += videoStreams.first().streamId;

  if (url.hasQueryItem("subtitles"))
  {
    if (!url.queryItemValue("subtitles").isEmpty())
      selectedStreams += url.queryItemValue("subtitles").toUShort(NULL, 16);
  }
  else if (!dataStreams.isEmpty())
    selectedStreams += dataStreams.first().streamId;

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

} // End of namespace
