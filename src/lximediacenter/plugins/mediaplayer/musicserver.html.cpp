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

const char * const MusicServer::htmlMusicStreams =
    " <table class=\"widgetsfull\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <p class=\"head\">{TR_PLAYLIST_STREAMS}</p>\n"
    "    <br />\n"
    "{PLAYLISTS}"
    "    <br />\n"
    "    <form name=\"create\" action=\"music.html\" method=\"get\">\n"
    "     <input type=\"hidden\" size=\"40\" name=\"stream\" value=\"-1\" />\n"
    "     <input type=\"text\" size=\"40\" name=\"playlist\" value=\"{TR_NEW_PLAYLIST}\" />\n"
    "     <input type=\"submit\" value=\"{TR_CREATE}\" />\n"
    "    </form>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <p class=\"head\">{TR_MANAGE_STREAM}</p>\n"
    "    <br />\n"
    "{STREAMS}"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <p class=\"head\">{TR_MUSIC_VIDEOS}</p>\n"
    "    <br />\n"
    "    <a href=\"{MUSICVIDEODIR}\">{TR_MUSIC_VIDEOS}</a><br />\n"
    "   </td>\n"
    "   <td class=\"nowidget\" width=\"50%\">\n"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const MusicServer::htmlMusicPlaylist =
    "    <a href=\"music.html?stream=-1&amp;playlist={PLAYLISTLINK}\">{PLAYLIST}</a><br />\n";

const char * const MusicServer::htmlMusicStream =
    "    <a href=\"music.html?stream={STREAM}\">{STREAM} ({PEER})</a><br />\n";

const char * const MusicServer::htmlMusic =
    " <table class=\"main\">\n"
    "  <tr class=\"main\">\n"
    "   <td class=\"center\" width=\"33%\">\n"
    "    <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "     <!--\n"
    "     document.write(\"<iframe class=\\\"noborder\\\" src=\\\"artistlist.html?stream={STREAM}\\\" name=\\\"artistlist\\\" width=\\\"100%\\\" height=\\\"\" + Math.max(window.innerHeight - 200, 300) + \"\\\" frameborder=\\\"0\\\"></iframe>\");\n"
    "     //-->\n"
    "    </script>\n"
    "    <noscript>\n"
    "     <iframe class=\"noborder\" src=\"artistlist.html?stream={STREAM}\" name=\"artistlist\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "    </noscript>\n"
    "   </td>\n"
    "   <td class=\"center\" width=\"33%\">\n"
    "    <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "     <!--\n"
    "     document.write(\"<iframe class=\\\"noborder\\\" src=\\\"songlist.html?stream={STREAM}\\\" name=\\\"songlist\\\" width=\\\"100%\\\" height=\\\"\" + Math.max(window.innerHeight - 200, 300) + \"\\\" frameborder=\\\"0\\\"></iframe>\");\n"
    "     //-->\n"
    "    </script>\n"
    "    <noscript>\n"
    "     <iframe class=\"noborder\" src=\"songlist.html?stream={STREAM}\" name=\"songlist\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "    </noscript>\n"
    "   </td>\n"
    "   <td class=\"center\" width=\"33%\">\n"
    "    <iframe class=\"noborder\" src=\"player.html?stream={STREAM}\" name=\"player\" width=\"100%\" height=\"98\" frameborder=\"0\"></iframe>\n"
    "    <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "     <!--\n"
    "     document.write(\"<iframe class=\\\"noborder\\\" src=\\\"playlist.html?stream={STREAM}\\\" name=\\\"playlist\\\" width=\\\"100%\\\" height=\\\"\" + Math.max(window.innerHeight - 300, 200) + \"\\\" frameborder=\\\"0\\\"></iframe>\");\n"
    "     //-->\n"
    "    </script>\n"
    "    <noscript>\n"
    "     <iframe class=\"noborder\" src=\"playlist.html?stream={STREAM}\" name=\"playlist\" width=\"100%\" height=\"200\" frameborder=\"0\"></iframe>\n"
    "    </noscript>\n"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const MusicServer::htmlList =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "{HEAD}"
    "</head>\n"
    "<body class=\"listiframe\">\n"
    " <table class=\"listiframe\">\n"
    "{ITEMS}\n"
    " </table>\n"
    "</body>\n"
    "</html>\n";

const char * const MusicServer::htmlListItem =
    "  <tr class=\"listitem\"><td class=\"listitem\">\n"
    "   <a class=\"listitem\" href=\"{ITEM_LINK}\" target=\"{ITEM_TARGET}\">{ITEM_NAME}</a>\n"
    "   <p class=\"listitemlight\">{ITEM_TEXT}</p>\n"
    "  </td></tr>\n";

const char * const MusicServer::htmlListAltItem =
    "  <tr class=\"listaltitem\"><td class=\"listaltitem\">\n"
    "   <a class=\"listitem\" href=\"{ITEM_LINK}\" target=\"{ITEM_TARGET}\">{ITEM_NAME}</a>\n"
    "   <p class=\"listitemlight\">{ITEM_TEXT}</p>\n"
    "  </td></tr>\n";

const char * const MusicServer::htmlMusicPlayer =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "{HEAD}"
    "</head>\n"
    "<body style=\"margin:0;padding:0;background-color:#000000\">\n"
    "{ITEMS}\n"
    "</body>\n"
    "</html>\n";

const char * const MusicServer::htmlPlayListItem =
    "  <tr class=\"listitem\">\n"
    "   <td class=\"listitem\" colspan=\"2\">\n"
    "    <p class=\"listitem\">{ITEM_NAME}</p>\n"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"listitem\">\n"
    "   <td class=\"listitem\">\n"
    "    <p class=\"listitemlight\">{ITEM_TEXT}</p>\n"
    "   </td>\n"
    "   <td class=\"listitemright\">\n"
    "    <a class=\"listitemlight\" href=\"{REMOVE_LINK}\">{REMOVE_TEXT}</a>\n"
    "   </td>\n"
    "  </tr>\n";

const char * const MusicServer::htmlPlayListAltItem =
    "  <tr class=\"listaltitem\">\n"
    "   <td class=\"listaltitem\" colspan=\"2\">\n"
    "    <p class=\"listitem\">{ITEM_NAME}</p>\n"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"listaltitem\">\n"
    "   <td class=\"listaltitem\">\n"
    "    <p class=\"listitemlight\">{ITEM_TEXT}</p>\n"
    "   </td>\n"
    "   <td class=\"listaltitemright\">\n"
    "    <a class=\"listitemlight\" href=\"{REMOVE_LINK}\">{REMOVE_TEXT}</a>\n"
    "   </td>\n"
    "  </tr>\n";

const char * const MusicServer::htmlPlayListSelItem =
    "  <tr class=\"listselitem\">\n"
    "   <td class=\"listselitem\" colspan=\"2\">\n"
    "    <a class=\"bookmark\" name=\"current\" />\n"
    "    <p class=\"listselitem\">{ITEM_NAME}</p>\n"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"listselitem\">\n"
    "   <td class=\"listselitem\">\n"
    "    <p class=\"listselitemlight\">{ITEM_TEXT}</p>\n"
    "   </td>\n"
    "   <td class=\"listselitemright\">\n"
    "    <a class=\"listselitemlight\" href=\"{REMOVE_LINK}\">{REMOVE_TEXT}</a>\n"
    "   </td>\n"
    "  </tr>\n";


bool MusicServer::handleHtmlRequest(const QUrl &url, const QString &file, QAbstractSocket *socket)
{
  struct T
  {
    static void buildItemText(HtmlParser &htmlParser, const MediaDatabase::Node &node)
    {
      htmlParser.setField("ITEM_TEXT", QByteArray());

      if (node.mediaInfo.duration().isPositive())
        htmlParser.appendField("ITEM_TEXT", QTime().addSecs(node.mediaInfo.duration().toSec()).toString(audioTimeFormat));

      if (!node.mediaInfo.author().isEmpty())
        htmlParser.appendField("ITEM_TEXT", " " + node.mediaInfo.author());

      if (!node.mediaInfo.album().isEmpty())
        htmlParser.appendField("ITEM_TEXT", " - " + node.mediaInfo.album());

      if (node.mediaInfo.year() > 0)
        htmlParser.appendField("ITEM_TEXT", " " + QByteArray::number(node.mediaInfo.year()));
    }
  };

  QHttpResponseHeader response(200);
  response.setContentType("text/html;charset=utf-8");
  response.setValue("Cache-Control", "no-cache");

  if ((file == "music.html") || file.isEmpty())
  {
    HtmlParser htmlParser;

    const int streamId = url.queryItemValue("stream").toInt();
    if (streamId == 0)
    {
      SDebug::MutexLocker dl(&dlnaDir.server()->mutex, __FILE__, __LINE__);
      htmlParser.setField("MUSICVIDEODIR", QByteArray::number(musicVideosDir->id, 16) + "-dir.html");
      dl.unlock();

      SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

      htmlParser.setField("TR_PLAYLIST_STREAMS", tr("Playlist streams"));
      htmlParser.setField("TR_MANAGE_STREAM", tr("Manage running stream"));
      htmlParser.setField("TR_NEW_PLAYLIST", tr("New playlist"));
      htmlParser.setField("TR_CREATE", tr("Create"));
      htmlParser.setField("TR_MUSIC_VIDEOS", tr("Music videos"));

      htmlParser.setField("PLAYLIST", "(" + tr("All files") + ")");
      htmlParser.setField("PLAYLISTLINK", QByteArray());
      htmlParser.setField("PLAYLISTS", htmlParser.parse(htmlMusicPlaylist));

      foreach (const QString &name, playlists())
      {
        htmlParser.setField("PLAYLIST", name);
        htmlParser.setField("PLAYLISTLINK", name);
        htmlParser.appendField("PLAYLISTS", htmlParser.parse(htmlMusicPlaylist));
      }

      htmlParser.setField("STREAMS", QByteArray());
      for (QMap<int, PlaylistStream *>::ConstIterator i=streams.begin(); i!=streams.end(); i++)
      {
        htmlParser.setField("STREAM", (*i)->name);
        htmlParser.setField("PEER", (*i)->peer.toString());
        htmlParser.appendField("STREAMS", htmlParser.parse(htmlMusicStream));
      }

      return sendHtmlContent(socket, url, response, htmlParser.parse(htmlMusicStreams));
    }
    else if (streamId == -1)
    {
      const int id = createStream(url.queryItemValue("playlist"), socket->peerAddress(), QString::null);

      return sendReply(socket, QByteArray(), NULL, false, "music.html?stream=" + QString::number(id));
    }
    else
    {
      htmlParser.setField("STREAM", QByteArray::number(streamId));

      return sendHtmlContent(socket, url, response, htmlParser.parse(htmlMusic));
    }
  }
  else if (file == "artistlist.html")
  {
    HtmlParser htmlParser;
    htmlParser.appendField("HEAD", QByteArray(headList));
    htmlParser.setField("ITEM_TARGET", QByteArray("songlist"));

    const QString target = "songlist.html?stream=" + url.queryItemValue("stream");

    int count = 0;
    foreach (const QString &artist, mediaDatabase->allMusicArtists())
    {
      htmlParser.setField("ITEM_LINK", target + "&artist=" + artist.toLower());
      htmlParser.setField("ITEM_NAME", !artist.isEmpty() ? artist.toLower() : tr("Unknown Artist"));

      const QList<MediaDatabase::UniqueID> files = mediaDatabase->allMusicArtistFiles(artist);
      const MediaDatabase::Node node = mediaDatabase->readNode(files.first());
      if (!node.isNull() && !node.mediaInfo.author().isEmpty())
        htmlParser.setField("ITEM_NAME", node.mediaInfo.author());

      htmlParser.setField("ITEM_TEXT", QString::number(files.count()) + " " + tr("songs"));

      htmlParser.appendField("ITEMS", htmlParser.parse((count++ & 1) ? htmlListAltItem : htmlListItem));
    }

    socket->write(response.toString().toUtf8());
    socket->write(htmlParser.parse(htmlList));
    return false;
  }
  else if (file == "songlist.html")
  {
    HtmlParser htmlParser;
    htmlParser.appendField("HEAD", QByteArray(headList));
    htmlParser.setField("ITEM_TARGET", QByteArray("playlist"));

    const QString artist = SStringParser::toRawName(url.queryItemValue("artist"));
    const QString target = "playlist.html?stream=" + url.queryItemValue("stream");

    int count = 0;
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->allMusicArtistFiles(artist))
    {
      const MediaDatabase::Node node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        htmlParser.setField("ITEM_LINK", target + "&queue=" + MediaDatabase::toUidString(node.uid));
        htmlParser.setField("ITEM_NAME", node.title());

        T::buildItemText(htmlParser, node);

        htmlParser.appendField("ITEMS", htmlParser.parse((count++ & 1) ? htmlListAltItem : htmlListItem));
      }
    }

    socket->write(response.toString().toUtf8());
    socket->write(htmlParser.parse(htmlList));
    return false;
  }
  else if (file == "player.html")
  {
    HtmlParser htmlParser;
    htmlParser.appendField("HEAD", QByteArray(headList));
    htmlParser.appendField("HEAD", QByteArray(headPlayer));
    htmlParser.setField("ITEMS", QByteArray());

    const int streamId = url.queryItemValue("stream").toInt();
    if (streamId > 0)
    {
      SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

      QMap<int, PlaylistStream *>::Iterator stream = streams.find(streamId);
      if ((stream != streams.end()) && !(*stream)->isRunning())
      { // No player yet; create one.
        if ((*stream)->playlistNode.playlist()->count() > 0)
        {
          htmlParser.setField("HEIGHT", QByteArray::number(98));
          htmlParser.setField("PLAYER_ITEM", "stream." + QByteArray::number(streamId));
          htmlParser.appendField("ITEMS", htmlParser.parse(htmlPlayerAudioItem));
        }
        else // Retry later
          response.setValue("Refresh", "10;URL=");
      }
    }

    socket->write(response.toString().toUtf8());
    socket->write(htmlParser.parse(htmlMusicPlayer));
    return false;
  }
  else if (file == "playlist.html")
  {
    HtmlParser htmlParser;
    htmlParser.appendField("HEAD", QByteArray(headList));
    htmlParser.setField("ITEM_TARGET", QByteArray("playlist"));
    htmlParser.setField("ITEMS", QByteArray());

    const int streamId = url.queryItemValue("stream").toInt();
    if (streamId > 0)
    {
      const QString target = "playlist.html?stream=" + QString::number(streamId);

      SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

      QMap<int, PlaylistStream *>::Iterator stream = streams.find(streamId);
      if (stream != streams.end())
      {
        bool save = false;

        if (url.hasQueryItem("skip"))
          (*stream)->playlistNode.nextSong();

        if (url.hasQueryItem("queue"))
        {
          (*stream)->playlistNode.playlist()->append(MediaDatabase::fromUidString(url.queryItemValue("queue")));
          save = true;
        }

        if (url.hasQueryItem("remove"))
        {
          (*stream)->playlistNode.playlist()->remove(MediaDatabase::fromUidString(url.queryItemValue("remove")));
          save = true;
        }

        if (save)
          storePlaylist((*stream)->playlistNode.playlist(), (*stream)->name);

        int count = 0;
        const MediaDatabase::UniqueID currentSong = (*stream)->playlistNode.currentSong();
        foreach (MediaDatabase::UniqueID uid, (*stream)->playlistNode.playlist()->next())
        {
          const MediaDatabase::Node node = mediaDatabase->readNode(uid);
          if (!node.isNull())
          {
            if (uid == currentSong)
            {
              htmlParser.setField("ITEM_NAME", node.title());
              htmlParser.setField("REMOVE_LINK", target + "&skip=skip");
              htmlParser.setField("REMOVE_TEXT", tr("Skip"));

              T::buildItemText(htmlParser, node);

              htmlParser.appendField("ITEMS", htmlParser.parse(htmlPlayListSelItem));
              count++;
            }
            else
            {
              htmlParser.setField("ITEM_NAME", node.title());
              htmlParser.setField("REMOVE_LINK", target + "&remove=" + MediaDatabase::toUidString(node.uid));
              htmlParser.setField("REMOVE_TEXT", tr("Remove"));

              T::buildItemText(htmlParser, node);

              htmlParser.appendField("ITEMS", htmlParser.parse((count++ & 1) ? htmlPlayListAltItem : htmlPlayListItem));
            }
          }
        }
      }
      else
        htmlParser.appendField("ITEMS", "<center>" + tr("Loading playlist") + " ...</center>");

      response.setValue("Refresh", "10;URL=" + target + "#current");
    }

    socket->write(response.toString().toUtf8());
    socket->write(htmlParser.parse(htmlList));
    return false;
  }
  else
    return MediaServer::handleHtmlRequest(url, file, socket);
}

} // End of namespace
