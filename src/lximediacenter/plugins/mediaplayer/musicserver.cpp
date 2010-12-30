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

#include "musicserver.h"

namespace LXiMediaCenter {

MusicServer::MusicServer(MediaDatabase *mediaDatabase, Plugin *plugin, MasterServer *server)
  : MediaServer(QT_TR_NOOP("Music"), mediaDatabase, plugin, server),
    musicVideosDir(NULL),
    playlistDir(GlobalSettings::applicationDataDir() + "/playlists"),
    nextStreamId(1)
{
  if (!playlistDir.exists())
    playlistDir.mkpath(playlistDir.absolutePath());

  enableDlna();
  connect(mediaDatabase, SIGNAL(updatedMusic()), SLOT(startDlnaUpdate()));
}

MusicServer::~MusicServer()
{
}

BackendServer::SearchResultList MusicServer::search(const QStringList &query) const
{
  SearchResultList results;

  foreach (const MediaDatabase::UniqueID &uid, mediaDatabase->queryMusic(query))
  {
    const MediaDatabase::Node node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QDir parentDir(node.path);
      parentDir.cdUp();

      const QString albumName = parentDir.dirName();
      const QString rawAlbumName = SStringParser::toRawName(albumName);
      const qreal match =
          qMin(SStringParser::computeMatch(SStringParser::toRawName(node.title()), query) +
               SStringParser::computeMatch(SStringParser::toRawName(node.mediaInfo.album()), query) +
               SStringParser::computeMatch(SStringParser::toRawName(node.mediaInfo.author()), query), 1.0);

      if (match >= minSearchRelevance)
      {
        SearchResult result;
        result.relevance = match;
        result.headline = node.title() + " [" + node.mediaInfo.author() + "] (" + tr("Song") + ")";
        result.location = MediaDatabase::toUidString(node.uid) + ".html";
        result.text = tr("Album") + ": " + node.mediaInfo.album() + ", " +
                      tr("Duration") + ": " + QTime().addSecs(node.mediaInfo.duration().toSec()).toString(audioTimeFormat) + ", " +
                      tr("Comment") + ": " + node.mediaInfo.comment();

        results += result;
      }
    }
  }

  return results;
}

void MusicServer::updateDlnaTask(void)
{
  DlnaServerDir * musicVideosDir = new DlnaServerDir(dlnaDir.server());
  foreach (const QString &clipAlbum, mediaDatabase->allMusicVideoAlbums())
  {
    QString albumName;

    QMultiMap<QString, MediaDatabase::Node> clips;
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->allMusicVideoFiles(clipAlbum))
    {
      const MediaDatabase::Node node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        if (albumName.isEmpty())
          albumName = node.mediaInfo.author();

        clips.insert(node.title(), node);
      }
    }

    if (!clips.isEmpty())
    {
      if (albumName.isEmpty())
        albumName = tr("Unknown Artist");

      DlnaServerDir * subDir = new DlnaServerDir(dlnaDir.server());
      for (QMultiMap<QString, MediaDatabase::Node>::Iterator i=clips.begin(); i!=clips.end(); i++)
      {
        DlnaServer::File file(subDir->server());
        file.date = i->lastModified;
        file.url = httpPath() + MediaDatabase::toUidString(i->uid) + ".mpeg";
        file.iconUrl = httpPath() + MediaDatabase::toUidString(i->uid) + "-thumb.jpeg";
        file.mimeType = "video/mpeg";
        file.music = true;

        subDir->addFile(i.key(), file);
      }

      musicVideosDir->addDir(albumName, subDir);
    }
  }

  DlnaServerDir * playlistsDir = new DlnaServerDir(dlnaDir.server());
  foreach (const QString &name, playlists())
  {
    DlnaServer::File file(dlnaDir.server());
    file.url = httpPath() + "stream.-1.mpeg?playlist=" + name;
    file.mimeType = "video/mpeg";
    file.music = true;
    playlistsDir->addFile(name, file);
  }

  DlnaServer::File file(dlnaDir.server());
  file.url = httpPath() + "stream.-1.mpeg";
  file.mimeType = "video/mpeg";
  file.music = true;
  file.sortOrder -= 1;
  playlistsDir->addFile("(" + tr("All files") + ")", file);

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  this->musicVideosDir = musicVideosDir;

  dlnaDir.clear();
  dlnaDir.addDir(tr("Music videos"), musicVideosDir);
  dlnaDir.addDir(tr("Playlists"), playlistsDir);
}

bool MusicServer::streamVideo(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  if ((file.count() >= 3) && (file.first() == "stream"))
  {
    SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

    int streamId = file[1].toInt();
    if (streamId == -1)
      streamId = createStream(url.queryItemValue("playlist"), socket->peerAddress(), request.path());

    QMap<int, PlaylistStream *>::Iterator i = streams.find(streamId);
    if (i != streams.end())
    {
      if ((*i)->setup(request, socket))
      if ((*i)->start())
        return true; // The graph owns the socket now.

      delete (*i);
      streams.erase(i);
    }

    socket->write(QHttpResponseHeader(404).toString().toUtf8());

    qWarning() << "Failed to start stream";
    return false;
  }
  else
    return MediaServer::streamVideo(request, socket);
}

QStringList MusicServer::playlists(void) const
{
  QStringList result;

  foreach (const QString &name, playlistDir.entryList(QStringList() << "*.m3u"))
    result += name.left(name.length() - 4);

  return result;
}

Playlist * MusicServer::createPlaylist(const QString &name)
{
  QFile file(playlistDir.absoluteFilePath(name + ".m3u"));
  if (file.open(QFile::ReadOnly))
  {
    Playlist * const playlist = new Playlist(mediaDatabase);
    if (playlist->deserialize(file.readAll()))
      return playlist;

    delete playlist;
  }

  return NULL;
}

int MusicServer::createStream(const QString &name, const QHostAddress &peer, const QString &url)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  const int id = nextStreamId.fetchAndAddRelaxed(1);

  Playlist *playlist = NULL;
  if (!name.isEmpty())
  {
    playlist = createPlaylist(name);
    if (playlist == NULL)
      playlist = new Playlist(mediaDatabase);
  }
  else
    playlist = Playlist::createAllFiles(mediaDatabase);

  PlaylistStream *stream =
      new PlaylistStream(this, id, peer, url, mediaDatabase, playlist,
                         name.isEmpty() ? ("(" + tr("All files") + ")") : name);

  stream->moveToThread(thread());
  streams.insert(id, stream);

  return id;
}

void MusicServer::storePlaylist(Playlist *playlist, const QString &name)
{
  const bool needUpdate = !playlistDir.exists(name + ".m3u");

  QFile file(playlistDir.absoluteFilePath(name + ".m3u"));
  if (file.open(QFile::WriteOnly))
    file.write(playlist->serialize());

  if (needUpdate)
    startDlnaUpdate();
}

MusicServer::PlaylistStream::PlaylistStream(MusicServer *parent, int id, const QHostAddress &peer, const QString &url, MediaDatabase *mediaDatabase, Playlist *playlist, const QString &name)
  : Stream(parent, peer, url),
    parent(parent),
    id(id),
    name(name),
    playlistNode(this, mediaDatabase, playlist)
{
  connect(&playlistNode, SIGNAL(finished()), SLOT(stop()));
  connect(&playlistNode, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&playlistNode, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));

  SDebug::WriteLocker l(&(parent->lock), __FILE__, __LINE__);
  parent->streams.insert(id, this);
}

MusicServer::PlaylistStream::~PlaylistStream()
{
  SDebug::WriteLocker l(&(parent->lock), __FILE__, __LINE__);
  parent->streams.remove(id);
}

bool MusicServer::PlaylistStream::setup(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  const SInterval frameRate = SInterval::fromFrequency(this->frameRate);
  SSize size(720, 576, 1.42222);
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
  }

  SAudioFormat::Channels channels = SAudioFormat::Channel_Stereo;
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

  const SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_None;
  const SInterfaces::VideoEncoder::Flags videoEncodeFlags = SInterfaces::VideoEncoder::Flag_Fast;

  QHttpResponseHeader header(200);
  header.setValue("Cache-Control", "no-cache");

  output.enablePseudoStreaming(1.0f);

  if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg"))
  {
    header.setContentType("video/MP2P");

    if (SAudioFormat::numChannels(channels) <= 2)
    {
      const SAudioCodec audioOutCodec("MP2", channels, 48000);
      playlistNode.setChannels(audioOutCodec.channelSetup());
      playlistNode.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else
    {
      const SAudioCodec audioOutCodec("AC3", channels, 48000);
      playlistNode.setChannels(audioOutCodec.channelSetup());
      playlistNode.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }

    playlistNode.enableVideo(size, frameRate);

    if (!videoEncoder.openCodec(SVideoCodec("MPEG2", size, frameRate), videoEncodeFlags))
    if (!videoEncoder.openCodec(SVideoCodec("MPEG1", size, frameRate), videoEncodeFlags))
      return false;

    output.openFormat("vob", audioEncoder.codec(), videoEncoder.codec(), STime::null);
  }
  else if (file.last().toLower() == "mpa")
  {
    header.setContentType("audio/mpeg");

    const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
    playlistNode.setChannels(audioOutCodec.channelSetup());
    playlistNode.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.openFormat("mp2", audioEncoder.codec(), STime::null);
  }
  else if (file.last().toLower() == "wav")
  {
    header.setContentType("audio/x-wav");

    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    playlistNode.setChannels(audioOutCodec.channelSetup());
    playlistNode.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.openFormat("wav", audioEncoder.codec(), STime::null);
  }
  else if (file.last().toLower() == "flv")
  {
    header.setContentType("video/x-flv");

    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    playlistNode.setChannels(audioOutCodec.channelSetup());
    playlistNode.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.openFormat("flv", audioEncoder.codec(), STime::null);
  }
  else
    return false;

  Stream::setup(url.queryItemValue("header") == "true", name);

  output.setHeader(header.toString().toUtf8());
  output.addSocket(socket);
  return true;
}

} // End of namespace
